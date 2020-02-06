#include "include/mesh.h"
#include "driver/gpio.h"
#define CONFIG_IPV4
static const char *MESH_TAG = "mesh_main";
static uint8_t tx_buf[TX_SIZE] = { 0, };
static uint8_t rx_buf[RX_SIZE] = { 0, };
static bool is_mesh_connected = false;
static mesh_addr_t mesh_parent_addr;
static int mesh_layer = -1;
static const char *TCP_TAG = "TCP_SERVER";

uint8_t SLAVE_ID;
uint8_t fconv=3200;
uint16_t port;
uint64_t energy_ini;
tipo_de_medidor tipo;

/****************************************/
/**** Configuracion e inicializacion ****/
/****************************************/

tipo_de_medidor str2enum (const char *str)
{    int j;
     for (j = 0;  j < sizeof (conversion) / sizeof (conversion[0]);  ++j)
         if (!strcmp (str, conversion[j].str))
             return conversion[j].val;
     ESP_LOGI(MESH_TAG,"Tipo no valido");
     return enlace;
}

void config_gpio(){
	/*Tipo de interrupcion*/
	gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);// instala el servicio ISR con la configuración por defecto.
	//Habilitacion de pines e interrupciones
	gpio_pad_select_gpio(SALVAR);   //configuro el BOTON_SALVAR como un pin GPIO
	gpio_set_direction(SALVAR, GPIO_MODE_DEF_INPUT);   // seleciono el BOTON_SALVAR como pin de entrada
	gpio_isr_handler_add(SALVAR, guadado_en_flash, NULL); // añado el manejador para el servicio ISR
	gpio_set_intr_type(SALVAR,GPIO_INTR_POSEDGE);  // habilito interrupción por flanco descendente (1->0)

	gpio_pad_select_gpio(PULSOS);   //configuro el BOTON_SALVAR como un pin GPIO
	gpio_set_direction(PULSOS, GPIO_MODE_DEF_INPUT);    // seleciono el PULSOS como pin de entrada
	gpio_isr_handler_add(PULSOS, interrupcion_pulsos, NULL); // añado el manejador para el servicio ISR
	gpio_set_intr_type(PULSOS,GPIO_INTR_POSEDGE);  // habilito interrupción por flanco descendente (1->0)
	gpio_pad_select_gpio(LED_PAPA);
	gpio_pad_select_gpio(RS485);
	gpio_set_direction(LED_PAPA,GPIO_MODE_DEF_OUTPUT);
	gpio_set_direction(RS485,GPIO_MODE_DEF_OUTPUT);
}

/********* Tareas del Root ************/

void esp_mesh_tx_to_ext(void *arg){

	mesh_addr_t from;
	mesh_data_t data;
	data.data = rx_buf;
	data.size = sizeof(rx_buf);
	data.proto = MESH_PROTO_BIN;
	int flag = 0;
	esp_err_t error;
	char trama[tamBUFFER];

	INT_VAL len;
	while(esp_mesh_is_root()){
		error = esp_mesh_recv(&from,&data,portMAX_DELAY,&flag,NULL,0);
		if(error==ESP_OK ){
			len.Val = rx_buf[4] + rx_buf[5] + 6;
			for(int i = 0 ;i < len.Val ;i++){
				trama[i] = (char)rx_buf[i];
			}
			for(int i = 0; i<len.Val;i++){
				printf("trama[%d] = %02x\r\n",i,trama[i]);
			}
			send(men,trama,len.Val,0);
		}
}
	ESP_LOGE(MESH_TAG,"Se elimino tarea TX EXT");
	vTaskDelete(NULL);
}

static void tcp_server_task(void *pvParameters)
{
    char rx_buffer[128];
    char addr_str[128];
    int addr_family;
    int ip_protocol;
    int sock=0;
    struct timeval timeout,timeout_listen;
    timeout.tv_sec = 20;
    timeout.tv_usec = 0;
    timeout_listen.tv_sec = 60*5;
    timeout_listen.tv_usec = 0;



#ifdef CONFIG_IPV4
        struct sockaddr_in dest_addr;
        dest_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(port);
        addr_family = AF_INET;
        ip_protocol = IPPROTO_IP;
        inet_ntoa_r(dest_addr.sin_addr, addr_str, sizeof(addr_str) - 1);

#else // IPV6
        struct sockaddr_in6 dest_addr;
        bzero(&dest_addr.sin6_addr.un, sizeof(dest_addr.sin6_addr.un));
        dest_addr.sin6_family = AF_INET6;
        dest_addr.sin6_port = htons(port);
        addr_family = AF_INET6;
        ip_protocol = IPPROTO_IPV6;
        inet6_ntoa_r(dest_addr.sin6_addr, addr_str, sizeof(addr_str) - 1);
#endif


    	int listen_sock = socket(addr_family, SOCK_STREAM, ip_protocol);   //Crea el socket
    	if (listen_sock < 0) {
        ESP_LOGE(TCP_TAG, "Unable to create socket: errno %d", errno);
        }

    	 if(setsockopt(listen_sock,SOL_SOCKET,SO_RCVTIMEO,(char *)&timeout_listen,sizeof(timeout_listen))==0){
    	             		printf("Tiempo Listening %d s\r\n",(uint32_t)timeout_listen.tv_sec);
    	             	}



    	ESP_LOGI(TCP_TAG, "Socket created");

    int err = bind(listen_sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));   //Asigna ip al socket
    if (err != 0) {
        ESP_LOGE(TCP_TAG, "Socket unable to bind: errno %d", errno);
    	}
    	ESP_LOGI(TCP_TAG, "Socket bound, port %d", port);


    while (esp_mesh_is_root()) {

        err = listen(listen_sock, 1);													//A la espera de las posibles conexiones
        if (err != 0) {
            ESP_LOGE(TCP_TAG, "Error occurred during listen: errno %d", errno);
            break;
        }
        ESP_LOGI(TCP_TAG, "Socket listening");

        struct sockaddr_in6 source_addr; // Large enough for both IPv4 or IPv6
        uint addr_len = sizeof(source_addr);
         sock = accept(listen_sock, (struct sockaddr *)&source_addr, &addr_len);		//Se acepta la conexión en caso de poder realizarse

         if(setsockopt(sock,SOL_SOCKET,SO_RCVTIMEO,(char *)&timeout,sizeof(timeout))==0){
             		printf("Configurado Timeout %d s\r\n",(uint32_t)timeout.tv_sec);
             	}
         men = sock;

         if (sock < 0) {
            ESP_LOGE(TCP_TAG, "Unable to accept connection: errno %d", errno);
           continue;
        }
        ESP_LOGI(TCP_TAG, "Socket accepted");


        while (esp_mesh_is_root()) {

        	printf("Recibiendo TCP...\r\n");
            int len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);					//Se queda esperando en el estado de recepción

            // Error occurred during receiving
            if (len < 0) {
                ESP_LOGE(TCP_TAG, "recv failed: errno %d", errno);
                timer_pause(TIMER_GROUP_0,TIMER_0);

                break;
            }

            // Connection closed
            else if (len == 0) {
                ESP_LOGI(TCP_TAG, "Connection closed");
                break;
            }
            // Data received
            else {

                // Get the sender's ip address as string
                if (source_addr.sin6_family == PF_INET) {
                    inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr.s_addr, addr_str, sizeof(addr_str) - 1);
                } else if (source_addr.sin6_family == PF_INET6) {
                    inet6_ntoa_r(source_addr.sin6_addr, addr_str, sizeof(addr_str) - 1);
                }


                rx_buffer[len] = 0; // Null-terminate whatever we received and treat like a string
                ESP_LOGI(TCP_TAG, "Received %d bytes from %s:", len, addr_str);
                ESP_LOGI(TCP_TAG, "%s",rx_buffer);

                xQueueSendToFront(RxSocket,rx_buffer,pdMS_TO_TICKS(1000));

            }
        }

        if (sock != -1) {
            ESP_LOGE(TCP_TAG, "Shutting down socket and restarting...");
            shutdown(sock, 0);
            close(sock);
           // socket(addr_family, SOCK_STREAM, ip_protocol);
       }
    }
    ESP_LOGE(MESH_TAG,"Se Elimino la tarea TCP\r\n");
    close(sock);
    vTaskDelete(NULL);
}

void esp_mesh_p2p_tx_main(void *Pa){
	esp_err_t err;
    mesh_data_t data;
    char mensaje2[tamBUFFER]="";
    data.proto=MESH_PROTO_BIN;
    data.data=tx_buf;
    data.size=sizeof(tx_buf);
    data.tos = MESH_TOS_P2P;

	int tamano, len=0;
	mesh_addr_t rt[CONFIG_MESH_ROUTE_TABLE_SIZE];

    while(esp_mesh_is_root()){

    	xQueueReceive(RxSocket,(char*)mensaje2,portMAX_DELAY);

    	tamano = 6 + (uint8_t)mensaje2[4] + (uint8_t)mensaje2[5];

    	for(int i =0;i<tamano;i++){
    		tx_buf[i]=(uint8_t)mensaje2[i];
    	}
    	esp_mesh_get_routing_table((mesh_addr_t*)&rt,CONFIG_MESH_ROUTE_TABLE_SIZE*6,&len);

    	printf("Mandando a Nodo\r\n");

    	for(int i = 1; i < len;i++){
    	err = esp_mesh_send(&rt[i],&data,MESH_DATA_P2P,NULL,0);
    	if(err != ESP_OK){
    		ESP_LOGW(MESH_TAG,"\r\nMensaje no enviado\r\n");
    	}
    	}

    }
    ESP_LOGE(MESH_TAG,"Se elimino tarea Tx main\r\n");
    vTaskDelete(NULL);
}


/********* Tareas de un Nodo ************/

/****************************/
/**** Bus RS485 Standard ****/
/****************************/

void esp_mesh_p2p_rx_main(void *arg)
	{
		INT_VAL len;
		INT_VAL CRC;
	    esp_err_t err;
	    mesh_addr_t from;
	    mesh_data_t data;
	    int flag = 0;
	    data.data = rx_buf;
	    data.size = sizeof(rx_buf);
	    mesh_rx_pending_t pendientes,auxi={.toSelf=0};
	    uint8_t trama[tamBUFFER];
	    const unsigned char *aux = &trama[4];

	    while (!esp_mesh_is_root()) {
	    	printf("Esperando...\r\n");
	    	err = esp_mesh_recv(&from,&data,portMAX_DELAY,&flag,NULL,0);
	    	esp_mesh_get_rx_pending(&pendientes);
			if(pendientes.toSelf!=auxi.toSelf){
				printf("Pendientes = %d\r\n",pendientes.toSelf);
				auxi.toSelf = pendientes.toSelf;
			}
	    	switch (err){
				case ESP_OK: {
					printf("Recibido por Mesh\r\n");

					if(!esp_mesh_is_root()){

					len.Val = rx_buf[4] + rx_buf[5];

					for(uint16_t i=0; i<len.Val ;i++){
						trama[i+4] = rx_buf[6+i];

						}
						CRC.Val = CRC16(aux,len.Val);
						len.Val = len.Val + 2;
						trama[len.Val+2]=CRC.byte.LB;
						trama[len.Val+3]=CRC.byte.HB;
						trama[0] = rx_buf[0];
						trama[1] = rx_buf[1];
						trama[2] = len.byte.HB;
						trama[3] = len.byte.LB;
						xQueueSendToFront(TxRS485,&trama,portMAX_DELAY);
					}
				}
				break;
				case ESP_ERR_MESH_NOT_START: {

					ESP_LOGE(MESH_TAG,"Aun no esta Iniciada la Mesh\r\n");

				}
				break;
				case ESP_ERR_MESH_TIMEOUT:{

					ESP_LOGW(MESH_TAG,"Timeout Error\r\n");
				}
				break;
				default:
					ESP_ERROR_CHECK_WITHOUT_ABORT(err);
				break;
	    	}

	    }

	    ESP_LOGW(MESH_TAG,"Tarea Rx main eliminada");
	    vTaskDelete(NULL);
}

void bus_rs485(void *arg){


		INT_VAL longitud;
		mesh_data_t dataMesh;
		BaseType_t ctrl_cola;

	    dataMesh.proto=MESH_PROTO_BIN;
		dataMesh.size=tamBUFFER;
	    dataMesh.data = tx_buf;
	    dataMesh.tos = MESH_TOS_P2P;

		uint16_t txlen,txctrl;

		uint8_t dataTx [tamBUFFER];
		uint8_t *aux = &dataTx[4];

		while(!esp_mesh_is_root()) {
				//Espera a recibir la trama RTU

				xQueueReceive(TxRS485,(uint8_t*)dataTx,portMAX_DELAY);
				printf("Recibido en cola...\r\n");
				txlen = dataTx[2]+dataTx[3];
				txctrl = uart_write_bytes(uart1,(char *)aux,txlen);

				if(txctrl>0){
					ctrl_cola = xQueueReceive(RxRS485,(uint8_t*)tx_buf+4,pdMS_TO_TICKS(500));
					if(ctrl_cola == pdTRUE){
						tx_buf[0]=dataTx[0];
						tx_buf[1]=dataTx[1];
						tx_buf[2]=0;
						tx_buf[3]=0;
						longitud.Val =tx_buf[4]+tx_buf[5]-2;
						tx_buf[4] = longitud.byte.HB;
						tx_buf[5] = longitud.byte.LB;
						tx_buf[4+longitud.Val] = NULL;
						tx_buf[4+longitud.Val +1] = NULL;
						for(int i =0; i < (longitud.Val +4);i++){
							printf("tx_buf[%d] = %02x\r\n",i,tx_buf[i]);
						}
						esp_err_t err = esp_mesh_send(NULL,&dataMesh,MESH_DATA_P2P,NULL,0);
						if(err == ESP_OK){
							printf("Mandado\r\n");
						}
					}else{
						uart_flush(UART_NUM_1);
					}
				}

		    }
		vTaskDelete(NULL);

}

/************************************/
/**** Medidor de salida a pulsos ****/
/************************************/

/*Interrupcion de entrada de pulso*/
void IRAM_ATTR interrupcion_pulsos (void* arg)
{
	xSemaphoreGiveFromISR(smfPulso,NULL);
}
/*Interrupcion asociada al guardado en flash*/
void IRAM_ATTR guadado_en_flash(void* arg)
{
	xSemaphoreGiveFromISR(smfNVS,NULL);
}
/*Tarea de comunicacion P2P con el sistema*/
void modbus_tcpip_pulsos(void *arg)
{
    esp_err_t err;
    int flag = 0;
    mesh_addr_t from;
    mesh_data_t data_rx,data_tx;
    data_rx.data = rx_buf;
    data_rx.size = RX_SIZE;
    data_tx.data = tx_buf;

	#define MODBUS_ENERGY_REG_INIT_POS 0x18
	#define MODBUS_ENERGY_REG_LEN 0x02
    energytype_t energia;

    while (1) {
        err = esp_mesh_recv(&from, &data_rx, portMAX_DELAY, &flag, NULL, 0);
        if (err == ESP_OK&&rx_buf[6]==SLAVE_ID&&rx_buf[7]==0x03&&rx_buf[8]==0x00&&rx_buf[9]==MODBUS_ENERGY_REG_INIT_POS&&rx_buf[11]==MODBUS_ENERGY_REG_LEN){

        	xQueuePeek(Cuenta_de_pulsos,&(energia.tot),portMAX_DELAY);
        	tx_buf[0] = rx_buf[0];
        	tx_buf[1] = rx_buf[1];
        	tx_buf[2] = 0x00;
        	tx_buf[3] = 0x00;
        	tx_buf[4] = 0x00; // len hb
        	tx_buf[5] = 0x07; // len lb
        	tx_buf[6] = SLAVE_ID; //SlaveID
        	tx_buf[7] = 0x03;
        	tx_buf[8] = 0x08; //byte count
        	tx_buf[9] = energia.u8.llll8;
        	tx_buf[10] = energia.u8.lll8;
        	tx_buf[11] = energia.u8.ll8;
        	tx_buf[12] = energia.u8.l8;
        	tx_buf[13] = energia.u8.h8;
        	tx_buf[14] = energia.u8.hh8;
        	tx_buf[15] = energia.u8.hhh8;
        	tx_buf[16] = energia.u8.hhhh8;
        	err = esp_mesh_send(&from,&data_tx, &flag, NULL, 0);
        }
    }
    vTaskDelete(NULL);
}
/*Tarea de guardado en flash de pulsos*/
void nvs_pulsos(void *arg){

		/*Variables para conteo de reinicios*/
		nvs_handle_t ctrl_pulsos;
		uint64_t pulsos;
		/* NVS open*/
		esp_err_t err = nvs_open("storage", NVS_READWRITE, &ctrl_pulsos);
	while(1){
			err = nvs_open("storage", NVS_READWRITE, &ctrl_pulsos);
			if (err != ESP_OK) {
				printf("Error (%s) abriendo el NVS!\n", esp_err_to_name(err));
			}else{
			xSemaphoreTake(smfNVS,portMAX_DELAY);
			xQueuePeek(Cuenta_de_pulsos,&pulsos,portMAX_DELAY);
			energy_ini = round(pulsos/fconv);
			err = nvs_set_u64(ctrl_pulsos, "energy", energy_ini);
			printf((err != ESP_OK) ? "Error pulse counter set!\n" : "Done\n");
			err = nvs_commit(ctrl_pulsos);
			printf((err != ESP_OK) ? "Error in pulse counter commit!\n" : "Done\n");
			close(ctrl_pulsos);
			}
			}
			ESP_LOGE(MESH_TAG,"La tarea de guardado en flash de pulsos fue eliminada");
			vTaskDelete(NULL);
}
/*Tarea del contador de pulsos*/
void conteo_pulsos (void *arg){

	uint64_t contador;
	contador = round(energy_ini/fconv);

	while(1){
		xSemaphoreTake(smfPulso,portMAX_DELAY);
		contador++;
		printf("Contador: %llu\r\n",contador);
		xQueueOverwrite(Cuenta_de_pulsos,&contador);
	}
	ESP_LOGE(MESH_TAG,"La tarea de conteo de pulsos fue eliminada");
	vTaskDelete(NULL);
}

/********************************/
/**** Manejadores de eventos ****/
/********************************/

void mesh_event_handler(void *arg, esp_event_base_t event_base,
                        int32_t event_id, void *event_data)
{
    mesh_addr_t id = {0,};
    static uint8_t last_layer = 0;

    switch (event_id) {
    case MESH_EVENT_STARTED: {
        esp_mesh_get_id(&id);
        ESP_LOGI(MESH_TAG, "<MESH_EVENT_MESH_STARTED>ID:"MACSTR"", MAC2STR(id.addr));
        is_mesh_connected = false;
        mesh_layer = esp_mesh_get_layer();
    }
    break;
    case MESH_EVENT_STOPPED: {
        ESP_LOGI(MESH_TAG, "<MESH_EVENT_STOPPED>");
        is_mesh_connected = false;
        mesh_layer = esp_mesh_get_layer();
    }
    break;
    case MESH_EVENT_CHILD_CONNECTED: {
        mesh_event_child_connected_t *child_connected = (mesh_event_child_connected_t *)event_data;
        ESP_LOGI(MESH_TAG, "<MESH_EVENT_CHILD_CONNECTED>aid:%d, "MACSTR"",
                 child_connected->aid,
                 MAC2STR(child_connected->mac));
    }
    break;
    case MESH_EVENT_CHILD_DISCONNECTED: {
        mesh_event_child_disconnected_t *child_disconnected = (mesh_event_child_disconnected_t *)event_data;
        ESP_LOGI(MESH_TAG, "<MESH_EVENT_CHILD_DISCONNECTED>aid:%d, "MACSTR"",
                 child_disconnected->aid,
                 MAC2STR(child_disconnected->mac));
    }
    break;
    case MESH_EVENT_ROUTING_TABLE_ADD: {
        mesh_event_routing_table_change_t *routing_table = (mesh_event_routing_table_change_t *)event_data;
        ESP_LOGW(MESH_TAG, "<MESH_EVENT_ROUTING_TABLE_ADD>add %d, new:%d",
                 routing_table->rt_size_change,
                 routing_table->rt_size_new);
    }
    break;
    case MESH_EVENT_ROUTING_TABLE_REMOVE: {
        mesh_event_routing_table_change_t *routing_table = (mesh_event_routing_table_change_t *)event_data;
        ESP_LOGW(MESH_TAG, "<MESH_EVENT_ROUTING_TABLE_REMOVE>remove %d, new:%d",
                 routing_table->rt_size_change,
                 routing_table->rt_size_new);
    }
    break;
    case MESH_EVENT_NO_PARENT_FOUND: {
        mesh_event_no_parent_found_t *no_parent = (mesh_event_no_parent_found_t *)event_data;
        ESP_LOGI(MESH_TAG, "<MESH_EVENT_NO_PARENT_FOUND>scan times:%d",
                 no_parent->scan_times);
    }
    /* TODO handler for the failure */
    break;
    case MESH_EVENT_PARENT_CONNECTED: {
    	gpio_set_level(LED_PAPA,1);
        mesh_event_connected_t *connected = (mesh_event_connected_t *)event_data;
        esp_mesh_get_id(&id);
        mesh_layer = connected->self_layer;
        memcpy(&mesh_parent_addr.addr, connected->connected.bssid, 6);
        ESP_LOGI(MESH_TAG,
                 "<MESH_EVENT_PARENT_CONNECTED>layer:%d-->%d, parent:"MACSTR"%s, ID:"MACSTR"",
                 last_layer, mesh_layer, MAC2STR(mesh_parent_addr.addr),
                 esp_mesh_is_root() ? "<ROOT>" :
                 (mesh_layer == 2) ? "<layer2>" : "", MAC2STR(id.addr));
        last_layer = mesh_layer;
        is_mesh_connected = true;
        if (esp_mesh_is_root()) {
            tcpip_adapter_dhcpc_start(TCPIP_ADAPTER_IF_STA);
        }
        else{
        	switch(tipo){
        		case(rs485):
                	xTaskCreatePinnedToCore(esp_mesh_p2p_rx_main, "Rx_child", 3072*2, NULL, 5, NULL,0);
                	xTaskCreatePinnedToCore(bus_rs485, "Rx_RS485", 3072*3, NULL, 5, NULL, 1);
        		break;
        		case(pulsos):
					xTaskCreatePinnedToCore(modbus_tcpip_pulsos,"Commun. P2P",3072*2,NULL,5,NULL,0);
        			xTaskCreatePinnedToCore(nvs_pulsos,"Guardar pulsos en flash",3072*2,NULL,5,NULL,1);
        			xTaskCreatePinnedToCore(conteo_pulsos,"Contador de pulsos",3072*2,NULL,5,NULL,1);
        		break;
        		case(chino):
					xTaskCreatePinnedToCore(esp_mesh_p2p_rx_main, "Rx_child", 3072*2, NULL, 5, NULL,0);
					xTaskCreatePinnedToCore(bus_rs485, "Rx_RS485", 3072*3, NULL, 5, NULL, 1);
        		break;
        		case(enlace):
        		break;
        	}
        }
    }
    break;
    case MESH_EVENT_PARENT_DISCONNECTED: {
    	gpio_set_level(LED_PAPA,0);
        mesh_event_disconnected_t *disconnected = (mesh_event_disconnected_t *)event_data;
        ESP_LOGI(MESH_TAG,
                 "<MESH_EVENT_PARENT_DISCONNECTED>reason:%d",
                 disconnected->reason);
        is_mesh_connected = false;
        mesh_layer = esp_mesh_get_layer();
    }
    break;
    case MESH_EVENT_LAYER_CHANGE: {
        mesh_event_layer_change_t *layer_change = (mesh_event_layer_change_t *)event_data;
        mesh_layer = layer_change->new_layer;
        ESP_LOGI(MESH_TAG, "<MESH_EVENT_LAYER_CHANGE>layer:%d-->%d%s",
                 last_layer, mesh_layer,
                 esp_mesh_is_root() ? "<ROOT>" :
                 (mesh_layer == 2) ? "<layer2>" : "");
        last_layer = mesh_layer;
    }
    break;
    case MESH_EVENT_ROOT_ADDRESS: {
        mesh_event_root_address_t *root_addr = (mesh_event_root_address_t *)event_data;
        ESP_LOGI(MESH_TAG, "<MESH_EVENT_ROOT_ADDRESS>root address:"MACSTR"",
                 MAC2STR(root_addr->addr));
        for(int i = 0;i<6;i++){
        	root_address.addr[i] = root_addr->addr[i];
        }

    }
    break;
    case MESH_EVENT_VOTE_STARTED: {
        mesh_event_vote_started_t *vote_started = (mesh_event_vote_started_t *)event_data;
        ESP_LOGI(MESH_TAG,
                 "<MESH_EVENT_VOTE_STARTED>attempts:%d, reason:%d, rc_addr:"MACSTR"",
                 vote_started->attempts,
                 vote_started->reason,
                 MAC2STR(vote_started->rc_addr.addr));
    }
    break;
    case MESH_EVENT_VOTE_STOPPED: {
        ESP_LOGI(MESH_TAG, "<MESH_EVENT_VOTE_STOPPED>");
        break;
    }
    case MESH_EVENT_ROOT_SWITCH_REQ: {
        mesh_event_root_switch_req_t *switch_req = (mesh_event_root_switch_req_t *)event_data;
        ESP_LOGI(MESH_TAG,
                 "<MESH_EVENT_ROOT_SWITCH_REQ>reason:%d, rc_addr:"MACSTR"",
                 switch_req->reason,
                 MAC2STR( switch_req->rc_addr.addr));
    }
    break;
    case MESH_EVENT_ROOT_SWITCH_ACK: {
        /* new root */
        mesh_layer = esp_mesh_get_layer();
        esp_mesh_get_parent_bssid(&mesh_parent_addr);
        ESP_LOGI(MESH_TAG, "<MESH_EVENT_ROOT_SWITCH_ACK>layer:%d, parent:"MACSTR"", mesh_layer, MAC2STR(mesh_parent_addr.addr));
    }
    break;
    case MESH_EVENT_TODS_STATE: {
        mesh_event_toDS_state_t *toDs_state = (mesh_event_toDS_state_t *)event_data;
        ESP_LOGI(MESH_TAG, "<MESH_EVENT_TODS_REACHABLE>state:%d", *toDs_state);
    }
    break;
    case MESH_EVENT_ROOT_FIXED: {
        mesh_event_root_fixed_t *root_fixed = (mesh_event_root_fixed_t *)event_data;
        ESP_LOGI(MESH_TAG, "<MESH_EVENT_ROOT_FIXED>%s",
                 root_fixed->is_fixed ? "fixed" : "not fixed");
    }
    break;
    case MESH_EVENT_ROOT_ASKED_YIELD: {
        mesh_event_root_conflict_t *root_conflict = (mesh_event_root_conflict_t *)event_data;
        ESP_LOGI(MESH_TAG,
                 "<MESH_EVENT_ROOT_ASKED_YIELD>"MACSTR", rssi:%d, capacity:%d",
                 MAC2STR(root_conflict->addr),
                 root_conflict->rssi,
                 root_conflict->capacity);
    }
    break;
    case MESH_EVENT_CHANNEL_SWITCH: {
        mesh_event_channel_switch_t *channel_switch = (mesh_event_channel_switch_t *)event_data;
        ESP_LOGI(MESH_TAG, "<MESH_EVENT_CHANNEL_SWITCH>new channel:%d", channel_switch->channel);
    }
    break;
    case MESH_EVENT_SCAN_DONE: {
        mesh_event_scan_done_t *scan_done = (mesh_event_scan_done_t *)event_data;
        ESP_LOGI(MESH_TAG, "<MESH_EVENT_SCAN_DONE>number:%d",
                 scan_done->number);
    }
    break;
    case MESH_EVENT_NETWORK_STATE: {
        mesh_event_network_state_t *network_state = (mesh_event_network_state_t *)event_data;
        ESP_LOGI(MESH_TAG, "<MESH_EVENT_NETWORK_STATE>is_rootless:%d",
                 network_state->is_rootless);
    }
    break;
    case MESH_EVENT_STOP_RECONNECTION: {
        ESP_LOGI(MESH_TAG, "<MESH_EVENT_STOP_RECONNECTION>");
    }
    break;
    case MESH_EVENT_FIND_NETWORK: {
        mesh_event_find_network_t *find_network = (mesh_event_find_network_t *)event_data;
        ESP_LOGI(MESH_TAG, "<MESH_EVENT_FIND_NETWORK>new channel:%d, router BSSID:"MACSTR"",
                 find_network->channel, MAC2STR(find_network->router_bssid));
    }
    break;
    case MESH_EVENT_ROUTER_SWITCH: {
        mesh_event_router_switch_t *router_switch = (mesh_event_router_switch_t *)event_data;
        ESP_LOGI(MESH_TAG, "<MESH_EVENT_ROUTER_SWITCH>new router:%s, channel:%d, "MACSTR"",
                 router_switch->ssid, router_switch->channel, MAC2STR(router_switch->bssid));
    }
    break;
    default:
        ESP_LOGI(MESH_TAG, "unknown id:%d", event_id);
        break;
    }
}

void ip_event_handler(void *arg, esp_event_base_t event_base,
                      int32_t event_id, void *event_data)
{
    ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
    ESP_LOGI(MESH_TAG, "<IP_EVENT_STA_GOT_IP>IP:%s", ip4addr_ntoa(&event->ip_info.ip));
    xTaskCreatePinnedToCore(tcp_server_task, "tcp_server", 1024*4, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(esp_mesh_p2p_tx_main, "TX", 1024*3, NULL, 5, NULL, 0);
    xTaskCreatePinnedToCore(esp_mesh_tx_to_ext, "TX_EXT", 3072*2, NULL, 5, NULL, 0);
}

/*Inicio Mesh*/

void mesh_init(struct form_home form){
	esp_err_t err;
	port = form.port;
	tipo = str2enum(form.tipo);
	SLAVE_ID = form.slaveid;
	energy_ini = form.energia;

	RxSocket = xQueueCreate(5,128);
	TxRS485 = xQueueCreate(5,128);
	RxlenRS485 = xQueueCreate(5,2);
	TCPsend = xQueueCreate(5,128);

	smfPulso = xSemaphoreCreateBinary();
	smfNVS = xSemaphoreCreateBinary();
	Cuenta_de_pulsos = xQueueCreate(1,8);

	config_gpio();
    /*  tcpip initialization */
    tcpip_adapter_init();
    /* for mesh
     * stop DHCP server on softAP interface by default
     * stop DHCP client on station interface by default
     * */
    tcpip_adapter_dhcps_stop(TCPIP_ADAPTER_IF_AP);
    tcpip_adapter_dhcpc_stop(TCPIP_ADAPTER_IF_STA);
	/*  event initialization */
    err = esp_event_loop_create_default();
    if(err!=ESP_OK){
	   printf("La cagadota");
    }
	/*  wifi initialization */
	wifi_init_config_t config = WIFI_INIT_CONFIG_DEFAULT();
	esp_wifi_init(&config);
	err = esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &ip_event_handler, NULL);
	if(err!=ESP_OK){
		printf("La cagada 1\r\n");
	}
	esp_wifi_set_mode(WIFI_MODE_STA);
	esp_wifi_set_storage(WIFI_STORAGE_FLASH);
	esp_wifi_set_ps(WIFI_PS_NONE);
	esp_mesh_set_6m_rate(false);
	esp_wifi_start();
	/*  mesh initialization */
	esp_mesh_init();
	err = esp_event_handler_register(MESH_EVENT, ESP_EVENT_ANY_ID, &mesh_event_handler, NULL);
	if(err!=ESP_OK){
			printf("La cagada 2\r\n");
	}
	esp_mesh_set_max_layer(form.max_layer);
	esp_mesh_set_vote_percentage(1);
	esp_mesh_set_ap_assoc_expire(10);
	mesh_cfg_t cfg = MESH_INIT_CONFIG_DEFAULT();
	/* mesh ID */
	memcpy((uint8_t *) &cfg.mesh_id, form.mesh_id, 6);
	/* router */
	cfg.channel = CONFIG_MESH_CHANNEL;
	cfg.router.ssid_len = strlen(form.ssid);
	memcpy((uint8_t *) &cfg.router.ssid, form.ssid, cfg.router.ssid_len);
	memcpy((uint8_t *) &cfg.router.password, form.password,strlen(form.password));
	/* mesh softAP */
	esp_mesh_set_ap_authmode(CONFIG_MESH_AP_AUTHMODE);
	cfg.mesh_ap.max_connection = form.max_sta;
	memcpy((uint8_t *) &cfg.mesh_ap.password, form.meshappass,strlen(form.meshappass));
	esp_mesh_set_config(&cfg);
	/* mesh start */
	esp_mesh_start();
}
