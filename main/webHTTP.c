/*********************************************************************************/
#include "include/webHTTP.h"
#include "esp_log.h"
#include "math.h"
const char *nvs_tag = "NVS";
const char *http_server = "HTTP SERVER";
static const char *REST_TAG = "esp-rest";
static const char *TAG = "REQUEST";
const char valid_user[]="usuario";
const char valid_pass[]="contrasena";

#define REST_CHECK(a, str, goto_tag, ...)                                              \
    do                                                                                 \
    {                                                                                  \
        if (!(a))                                                                      \
        {                                                                              \
            ESP_LOGE(REST_TAG, "%s(%d): " str, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
            goto goto_tag;                                                             \
        }                                                                              \
    } while (0)

#define FILE_PATH_MAX (ESP_VFS_PATH_MAX + 128)
#define SCRATCH_BUFSIZE (10240)

bool ahora = true;
conversion_t conversion[4] = {{rs485,"rs485"},{pulsos,"pulsos"},{chino,"chino"},{enlace,"enlace"}};

struct form_home fweb_mesh_config;

typedef struct rest_server_context {
    char base_path[ESP_VFS_PATH_MAX + 1];
    char scratch[SCRATCH_BUFSIZE];
} rest_server_context_t;

tipo_de_medidor str2enum (const char *str)
{    int j;
     for (j = 0;  j < sizeof (conversion) / sizeof (conversion[0]);  ++j)
         if (!strcmp (str, conversion[j].str))
             return conversion[j].val;
     ESP_LOGI(http_server,"Tipo no valido");
     return enlace;
}

#define CHECK_FILE_EXTENSION(filename, ext) (strcasecmp(&filename[strlen(filename) - strlen(ext)], ext) == 0)

void set_form_flash_mesh(struct form_home form){

	esp_err_t err;
	nvs_handle_t ctrl_flash;
	err = nvs_open("storage",NVS_READWRITE,&ctrl_flash);
	if (err != ESP_OK) {
		printf("Error (%s) opening NVS handle!\n\r", esp_err_to_name(err));
	}else{

		ESP_LOGI("FROM_NVS","%s",form.ssid);
		err=nvs_set_str(ctrl_flash,"ssid",form.ssid);
		if (err != ESP_OK) {printf("Error (%s) setting in flash ssid!\n\r", esp_err_to_name(err));}

		ESP_LOGI("FROM_NVS","%s",form.password);
		err=nvs_set_str(ctrl_flash,"password",form.password);
		if (err != ESP_OK) {printf("Error (%s) setting in flash password!\n\r", esp_err_to_name(err));}

		ESP_LOGI("FROM_NVS",MACSTR,MAC2STR(form.mesh_id));
		err=nvs_set_blob(ctrl_flash,"meshid",form.mesh_id,sizeof(form.mesh_id));
		if (err != ESP_OK) {printf("Error (%s) setting in flash mesh id!\n\r", esp_err_to_name(err));}

		ESP_LOGI("FROM_NVS","%s",form.meshappass);
		err=nvs_set_str(ctrl_flash,"meshpass",form.meshappass);
		if (err != ESP_OK) {printf("Error (%s) setting in flash mesh password!\n\r", esp_err_to_name(err));}

		ESP_LOGI("FROM_NVS","%d",form.max_layer);
		err=nvs_set_u8(ctrl_flash,"max_layer",form.max_layer);
		if (err != ESP_OK) {printf("Error (%s) setting in flash max layer!\n\r", esp_err_to_name(err));}

		ESP_LOGI("FROM_NVS","%d",form.max_sta);
		err=nvs_set_u8(ctrl_flash,"max_sta",form.max_sta);
		if (err != ESP_OK) {printf("Error (%s) setting in flash max STA!\n\r", esp_err_to_name(err));}

		ESP_LOGI("FROM_NVS","%d",form.port);
		err=nvs_set_u16(ctrl_flash,"port",form.port);
		if (err != ESP_OK) {printf("Error (%s) setting in flash port!\n\r", esp_err_to_name(err));}

		err = nvs_commit(ctrl_flash);
		if (err != ESP_OK) {printf("Error (%s) while the commit stage!\n\r", esp_err_to_name(err));}
	}
	nvs_close(ctrl_flash);
}

void set_form_flash_modbus(struct form_home form){
	esp_err_t err;
	nvs_handle_t ctrl_flash;
	tipo_de_medidor tipo;
	err = nvs_open("storage",NVS_READWRITE,&ctrl_flash);
	if (err != ESP_OK) {
		printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
	}else{
		nvs_set_str(ctrl_flash,"tipo",form.tipo);

		tipo = str2enum(form.tipo);
		switch(tipo){

			case(rs485):
					nvs_set_u32(ctrl_flash,"baud",form.baud_rate);
			break;
			case(pulsos):
					nvs_set_u64(ctrl_flash,"energy",form.energia);

					nvs_set_u8(ctrl_flash,"slaveid",form.slaveid);

					nvs_set_u16(ctrl_flash,"conver",form.conversion);
			break;
			case(chino):
					nvs_set_u32(ctrl_flash,"baud",form.baud_rate);
			break;
			case(enlace):
			break;

			}
		err = nvs_commit(ctrl_flash);
	}
	nvs_close(ctrl_flash);
}

void get_form_flash_mesh(struct form_home *form){
	size_t len;
	char mac[18];
	esp_err_t err;
	nvs_handle_t ctrl_flash, ctrl_prueba;

	err = nvs_open("storage",NVS_READWRITE,&ctrl_flash);
	if (err != ESP_OK) {
		printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
	}else{
		err = nvs_get_str(ctrl_flash,"ssid",NULL,&len);
		if(err==ESP_OK) {
			err = nvs_get_str(ctrl_flash,"ssid",form->ssid,&len);
			switch(err){
				case ESP_OK:
					ESP_LOGI(nvs_tag,"SSID en flash: %s",form->ssid);
				break;
				case ESP_ERR_NVS_NOT_FOUND:
					ESP_LOGI(nvs_tag,"SSID en flash: none");
				break;
				default:
					printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
				break;
			}
		}

		err = nvs_get_str(ctrl_flash,"password",NULL,&len);
		if(err==ESP_OK){
			err= nvs_get_str(ctrl_flash,"password",form->password,&len);
		switch(err){
			case ESP_OK:
				ESP_LOGI(nvs_tag,"Password en flash: %s",form->password);
			break;
			case ESP_ERR_NVS_NOT_FOUND:
				ESP_LOGI(nvs_tag,"Password en flash: none");
			break;
			default:
				printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
			break;
		}
		}
		err = nvs_get_blob(ctrl_flash,"meshid",NULL,&len);
		if(err==ESP_OK){
		err = nvs_get_blob(ctrl_flash,"meshid",form->mesh_id,&len);
		switch(err){
			case ESP_OK:
				sprintf(mac,MACSTR,MAC2STR(form->mesh_id));
				ESP_LOGI(nvs_tag,"Mesh id en flash: %s",mac);
			break;
			case ESP_ERR_NVS_NOT_FOUND:
				ESP_LOGI(nvs_tag,"Mesh id en flash: none");
			break;
			default:
				printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
			break;
		}
		}
	}nvs_close(ctrl_flash);

	err = nvs_open("storage",NVS_READWRITE,&ctrl_prueba);

		if (err != ESP_OK) {
			printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
		}else{
			err = nvs_get_str(ctrl_prueba,"meshpass",NULL,&len);
			if(err==ESP_OK){
				err= nvs_get_str(ctrl_prueba,"meshpass",form->meshappass,&len);
			switch(err){
				case ESP_OK:
					ESP_LOGI(nvs_tag,"Mesh password en flash: %s",form->meshappass);
				break;
				case ESP_ERR_NVS_NOT_FOUND:
					ESP_LOGI(nvs_tag,"Mesh password en flash: none");
				break;
				default:
					printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
				break;
			}
			}
			err = nvs_get_u8(ctrl_prueba,"max_layer",&(form->max_layer));
			switch(err){
				case ESP_OK:
					ESP_LOGI(nvs_tag,"Max. Layer en flash: %d",form->max_layer);
				break;
				case ESP_ERR_NVS_NOT_FOUND:
					ESP_LOGI(nvs_tag,"Max. Layer en flash: none");
				break;
				default:
					printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
				break;
			}
			err = nvs_get_u8(ctrl_prueba,"max_sta",&(form->max_sta));
			switch(err){
				case ESP_OK:
					ESP_LOGI(nvs_tag,"Max. Sta en flash: %d",form->max_sta);
				break;
				case ESP_ERR_NVS_NOT_FOUND:
					ESP_LOGI(nvs_tag,"Max. Sta en flash: none");
				break;
				default:
					printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
				break;
			}
			err = nvs_get_u16(ctrl_prueba,"port",&(form->port));
			switch(err){
				case ESP_OK:
					ESP_LOGI(nvs_tag,"Port en flash: %d",form->port);
				break;
				case ESP_ERR_NVS_NOT_FOUND:
					ESP_LOGI(nvs_tag,"Port en flash: none");
				break;
				default:
					printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
				break;
			}
			err = nvs_get_str(ctrl_prueba,"tipo",NULL,&len);
			if(err==ESP_OK){
				err= nvs_get_str(ctrl_prueba,"tipo",form->tipo,&len);
			switch(err){
				case ESP_OK:
					ESP_LOGI(nvs_tag,"Tipo en flash: %s",form->tipo);
				break;
				case ESP_ERR_NVS_NOT_FOUND:
					ESP_LOGI(nvs_tag,"Tipo en flash: none");
				break;
				default:
					printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
				break;
			}
			}
			switch(str2enum(form->tipo)){

			case(rs485):
					err = nvs_get_u32(ctrl_prueba,"baud",&(form->baud_rate));
					switch(err){
						case ESP_OK:
							ESP_LOGI(nvs_tag,"Baud Rate en flash: %"PRIu32,form->baud_rate);
						break;
						case ESP_ERR_NVS_NOT_FOUND:
							ESP_LOGI(nvs_tag,"Baud Rate en flash: none");
						break;
						default:
							printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
						break;
					}
					break;
			case(pulsos):
					err = nvs_get_u64(ctrl_prueba,"energy",&(form->energia));
					switch(err){
						case ESP_OK:
							ESP_LOGI(nvs_tag,"Pulsos en flash: %"PRIu64,form->energia);
						break;
						case ESP_ERR_NVS_NOT_FOUND:
							ESP_LOGI(nvs_tag,"Pulsos en flash: none");
						break;
						default:
							printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
						break;
					}
					err = nvs_get_u8(ctrl_prueba,"slaveid",&(form->slaveid));
					switch(err){
						case ESP_OK:
							ESP_LOGI(nvs_tag,"Slave ID en medidor a pulsos en flash: %d",form->slaveid);
						break;
						case ESP_ERR_NVS_NOT_FOUND:
							ESP_LOGI(nvs_tag,"Slave ID en flash: none");
						break;
						default:
							printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
						break;
					}

					err = nvs_get_u16(ctrl_prueba,"conver",&(form->conversion));
					switch(err){
						case ESP_OK:
							ESP_LOGI(nvs_tag,"Factor de conversion en flash: %d",form->conversion);
						break;
						case ESP_ERR_NVS_NOT_FOUND:
							ESP_LOGI(nvs_tag,"Factor de conversion en flash: none");
						break;
						default:
							printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
						break;
					}
					break;
				case(chino):
					err = nvs_get_u32(ctrl_prueba,"baud",&(form->baud_rate));
					switch(err){
						case ESP_OK:
							ESP_LOGI(nvs_tag,"Baud Rate en flash: %"PRIu32,form->baud_rate);
						break;
						case ESP_ERR_NVS_NOT_FOUND:
							ESP_LOGI(nvs_tag,"Baud Rate en flash: none");
						break;
						default:
							printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
						break;
					}
					break;
					case(enlace):
							break;
					default:
						break;
			}
	}
	nvs_close(ctrl_prueba);
}

bool fill_form_mesh(char * p, struct form_home *form){

    cJSON *root = cJSON_Parse(p);
    char *eptr;
    char *sys_info = cJSON_Print(root);
    ESP_LOGI("FILL","%s",sys_info);

    /*SSID*/
    char *aux_ssid = cJSON_GetObjectItem(root, "ssid")->valuestring;
    if(strlen(aux_ssid)>20){
    	printf("SSID No válido");
    	return false;}else{for (int i = 0; i<=strlen(aux_ssid);i++){form->ssid[i] = aux_ssid[i];}}
	ESP_LOGW("FILL","SSID: %s",form->ssid);

    /*WiFi Pass*/
    char *aux_password = cJSON_GetObjectItem(root, "password")->valuestring;
    if(strlen(aux_password)>20){
    	printf("Clave WiFi No válida");
    	return false;}else{for (int i = 0; i<=strlen(aux_password);i++){form->password[i] = aux_password[i];}}
    ESP_LOGW("FILL","WiFi Pass: %s",form->password);

    /*Mesh ID*/
    char *aux_meshID = cJSON_GetObjectItem(root, "meshID")->valuestring;
    if(strlen(aux_meshID)!=17){
    	printf("Mesh ID No válida : %s %d", aux_meshID,strlen(aux_meshID));
    	return false;}
    for(int i = 0; i<17;i++){
    	if((i==2||i==5||i==8||i==11||i==14) && aux_meshID[i]!=':'){
    		printf("Mesh ID No válida aux_meshID[%d]=%c",i,aux_meshID[i]);
    		return false;
    	}else if (!((aux_meshID[i]>=0x30 && aux_meshID[i]<=0x39)||(aux_meshID[i]>=0x61||aux_meshID[i]<=0x66))){
    			printf("Mesh ID No válida caractér no válido [%d]", i);
    			return false;
    	}
    }
    for(int i = 0; i<18;i++){
    	if(aux_meshID[i]>0x60){aux_meshID[i]-=0x57;}
    	if(aux_meshID[i+1]>0x60){aux_meshID[i+1]-=0x57;}
    	form->mesh_id[i/3]=((0x0f&aux_meshID[i])<<4)|(0x0f&aux_meshID[i+1]);
    	i+=2;
    }
    ESP_LOGW("FILL","meshID: "MACSTR ,MAC2STR(form->mesh_id));

    /*Mesh access password*/
    char* aux_mesh_password = cJSON_GetObjectItem(root, "mesh_password")->valuestring;
    if(strlen(aux_mesh_password)<6||strlen(aux_mesh_password)>15){
    	printf("Contraseña de la red mesh no válida");
    	return false;
    }else{ for(int i = 0; i<=strlen(aux_mesh_password);i++){form->meshappass[i]=aux_mesh_password[i];}}
    ESP_LOGW("FILL","Mesh access password: %s",form->meshappass);

    /*Max Layer*/
    uint8_t aux_max_layer = (uint8_t) strtoul(cJSON_GetObjectItem(root, "max_layer")->valuestring,&eptr,10);
    if(aux_max_layer>=10 && aux_max_layer<=25){form->max_layer = aux_max_layer;}else{return false;}
    ESP_LOGW("FILL","Layers: %d", form->max_layer);

    /*Max sta*/
    uint8_t aux_max_sta = (uint8_t) strtoul(cJSON_GetObjectItem(root, "max_sta")->valuestring,&eptr,10);
    if(aux_max_sta>=1 && aux_max_sta <=9){form->max_sta = aux_max_sta;}else{return false;}
    ESP_LOGW("FILL","STA: %d", form->max_sta);

    /*Port*/
    uint32_t aux_port = (uint32_t) strtoul(cJSON_GetObjectItem(root, "port")->valuestring,&eptr,10);
    if(aux_port<=65535){form->port = (uint16_t)aux_port;}else{return false;}
    ESP_LOGW("FILL","Port: %d", form->port);

	cJSON_Delete(root);
	return true;
}

bool fill_form_modbus(char *p, struct form_home form){
	char *ini;
	int count;
	tipo_de_medidor tipo;

	/*Extrayendo Modo Output*/
	ini = strstr(p,"modoOutput=");
	if(ini!=NULL){
		ini +=sizeof("modoOutput=")-1;
		for(int i =0;ini[i]!='&';i++){
			form.tipo[i]=ini[i];
		}
		printf("Modo de Nodo: %s\r\n",form.tipo);
	}
	else{
		return false;
	}

	tipo = str2enum(form.tipo);

	switch(tipo){
		case(rs485):
		ini=strstr(p,"baud_rate=");
		if(ini!=NULL){
			ini+=sizeof("baud_rate=")-1;
			char baud[10];
			count = 0;
			int auxRate;
			for(int i = 0; ini[i]!='&';i++){
				baud[i]=ini[i];
				count++;
			}
			baud[count]=0;
			auxRate = atoi(baud);
			if(auxRate>=0){
				form.baud_rate =(uint32_t)auxRate;
				printf("Baud Rate:%d\r\n",form.baud_rate);
			}else{
				return false;
			}
		}

		break;

		case(pulsos):
			/*Extrayendo Factor de Conversion*/
				ini=strstr(p,"conversion=");
				if(ini!=NULL){
					ini+=sizeof("conversion=")-1;
					char conversion[5];
					count = 0;
					int cont;
					for(int i = 0; ini[i]!='&';i++){
						conversion[i]=ini[i];
						count++;
					}
					conversion[count]=0;
					cont = atoi(conversion);
					if(cont>=0){
						form.conversion =(uint16_t)cont;
						printf("Factor de conversion: %d imp/kWh\r\n",form.conversion);
					}else{
						return false;
					}
				}
				/*Extrayendo Medida Inicial*/
				ini=strstr(p,"energia=");
				if(ini!=NULL){
					ini+=sizeof("energia=")-1;
					char energy[]="";
					float backup_f;
					uint64_t backup=0;
					for(int i = 0; ini[i]!='&';i++){
						energy[i]=ini[i];
					}
					backup_f = atof(energy);
					printf("Medicion Energia:%.4fkWh\r\n",backup_f);
					backup = round(backup_f*form.conversion);
					if((backup>0)&(backup!=0)){
						form.energia = backup;
						printf("Medicion Pulsos:%"PRIu64"\r\n",form.energia);
					}
				}
				/*Extrayendo Slave IDl*/
				ini=strstr(p,"slaveid=");
				if(ini!=NULL){
					ini+=sizeof("slaveid=")-1;
					char slaveid[3];
					count = 0;
					int auxSlave;
					for(int i = 0; ini[i]!='&';i++){
						slaveid[i]=ini[i];
						count++;
					}
					slaveid[count]=0;
					auxSlave = atoi(slaveid);
					if(auxSlave>=0){
						form.slaveid =(uint8_t)auxSlave;
						printf("Slave ID:%d\r\n",form.slaveid);
					}else{
						return false;
					}
				}
				break;

		case(chino):
		ini=strstr(p,"baud_rate=");
				if(ini!=NULL){
					ini+=sizeof("baud_rate=")-1;
					char baud[10];
					count = 0;
					int auxRate;
					for(int i = 0; ini[i]!='&';i++){
						baud[i]=ini[i];
						count++;
					}
					baud[count]=0;
					auxRate = atoi(baud);
					if(auxRate>=0){
						form.baud_rate =(uint32_t)auxRate;
						printf("Baud Rate:%d\r\n",form.baud_rate);
					}else{
						return false;
					}
				}
				break;

		case(enlace):
		break;
	}
	set_form_flash_modbus(form);
	return true;
}

/* Set HTTP response content type according to file extension */
static esp_err_t set_content_type_from_file(httpd_req_t *req, const char *filepath)
{
    const char *type = "text/plain";
    if (CHECK_FILE_EXTENSION(filepath, ".html")) {
        type = "text/html";
    }else if (CHECK_FILE_EXTENSION(filepath, ".js")) {
        type = "application/javascript";
    }else if (CHECK_FILE_EXTENSION(filepath, ".css")) {
        type = "text/css";
    } else if (CHECK_FILE_EXTENSION(filepath, ".png")) {
        type = "image/png";
    } else if (CHECK_FILE_EXTENSION(filepath, ".ico")) {
        type = "image/x-icon";
    } else if (CHECK_FILE_EXTENSION(filepath, ".svg")) {
        type = "text/xml";
    }
    return httpd_resp_set_type(req, type);
}

/* Send HTTP response with the contents of the requested file */
static esp_err_t rest_common_get_handler(httpd_req_t *req)
{
    char filepath[FILE_PATH_MAX];

    ESP_LOGI(TAG,"req->uri: %s",req->uri);
    bool pasa = true;

    char foobar[7] = "/foobar";
    for(int i = 0; i < sizeof(foobar);i++){
    	if(foobar[i]!=req->uri[i]){
    		pasa=false;
    	}
    }

    rest_server_context_t *rest_context = (rest_server_context_t *)req->user_ctx;
    strlcpy(filepath, rest_context->base_path, sizeof(filepath));

    if (req->uri[strlen(req->uri) - 1] == '/' || pasa) {
        strlcat(filepath, "/index.html", sizeof(filepath));
    } else {
        strlcat(filepath, req->uri, sizeof(filepath));
    }

    int fd = open(filepath, O_RDONLY, 0);
    if (fd == -1) {
        ESP_LOGE(REST_TAG, "Failed to open file : %s", filepath);
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to read existing file");
        return ESP_FAIL;
    }

    set_content_type_from_file(req, filepath);

    char *chunk = rest_context->scratch;
    ssize_t read_bytes;
    do {
        /* Read file in chunks into the scratch buffer */
        read_bytes = read(fd, chunk, SCRATCH_BUFSIZE);
        if (read_bytes == -1) {
            ESP_LOGE(REST_TAG, "Failed to read file : %s", filepath);
        } else if (read_bytes > 0) {
            /* Send the buffer contents as HTTP response chunk */
            if (httpd_resp_send_chunk(req, chunk, read_bytes) != ESP_OK) {
                close(fd);
                ESP_LOGE(REST_TAG, "File sending failed!");
                /* Abort sending file */
                httpd_resp_sendstr_chunk(req, NULL);
                /* Respond with 500 Internal Server Error */
                httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to send file");
                return ESP_FAIL;
            }
        }
    } while (read_bytes > 0);
    /* Close file after sending complete */
    close(fd);
    ESP_LOGI(REST_TAG, "File sending complete");
    /* Respond with an empty chunk to signal HTTP response completion */
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

static esp_err_t form_mesh_req_handler(httpd_req_t *req){/*Recepción de formulario de configuración mesh*/

    int total_len = req->content_len;
    int cur_len = 0;
    char *buf = ((rest_server_context_t *)(req->user_ctx))->scratch;
    int received = 0;
    if (total_len >= SCRATCH_BUFSIZE) {
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "content too long");
        return ESP_FAIL;
    }
    while (cur_len < total_len) {
        received = httpd_req_recv(req, buf + cur_len, total_len);
        if (received <= 0) {
            /* Respond with 500 Internal Server Error */
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to post control value");
            return ESP_FAIL;
        }
        cur_len += received;
    }
    buf[total_len] = '\0';

    ESP_LOGI("DEBUG","BUFFER: %s",buf);
    if(fill_form_mesh(buf, &fweb_mesh_config)){
    	set_form_flash_mesh(fweb_mesh_config);
    	httpd_resp_sendstr(req, "Success");
    	return ESP_OK;
    }else{
    	httpd_resp_sendstr(req, "Error");
    	return ESP_FAIL;
    }

}

static esp_err_t login_req_handler(httpd_req_t *req){/*Revisa que el usuario y contraseña del login sea el correcto*/
    int total_len = req->content_len;
    int cur_len = 0;
    char *buf = ((rest_server_context_t *)(req->user_ctx))->scratch;
    int received = 0;
    if (total_len >= SCRATCH_BUFSIZE) {
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "content too long");
        return ESP_FAIL;
    }
    while (cur_len < total_len) {
        received = httpd_req_recv(req, buf + cur_len, total_len);
        if (received <= 0) {
            /* Respond with 500 Internal Server Error */
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to post control value");
            return ESP_FAIL;
        }
        cur_len += received;
    }
    buf[total_len] = '\0';

    ESP_LOGI("DEBUG","BUFFER: %s",buf);

    cJSON *root = cJSON_Parse(buf);

    char* aux_usuario = cJSON_GetObjectItem(root,"usuario")->valuestring;
    char* aux_contrasena = cJSON_GetObjectItem(root,"contrasena")->valuestring;
    if( (strlen(aux_usuario)==strlen(valid_user))&&(strlen(aux_contrasena)==strlen(valid_pass))){
    	if((strcmp(aux_usuario, valid_user)==0)&&(strcmp(aux_contrasena, valid_pass)==0)){
			httpd_resp_sendstr(req, "valido");
		}else{
			httpd_resp_sendstr(req, "content no valido");
		}
    }else{
    	httpd_resp_sendstr(req, "long no valido");
    }
	cJSON_Delete(root);

    return ESP_OK;
}

static esp_err_t form_mesh_get_handler(httpd_req_t *req){/*Toma los datos de la mesh en flash y los sube al formulario mesh*/

	ESP_LOGI(TAG,"req->uri: %s",req->uri);

	get_form_flash_mesh(&fweb_mesh_config);
	httpd_resp_set_status(req, HTTPD_200);
	httpd_resp_set_type(req, HTTPD_TYPE_JSON);

	 char mesh_id [18] = {0,};
	 sprintf(mesh_id,MACSTR,MAC2STR(fweb_mesh_config.mesh_id));

	 cJSON *root = cJSON_CreateObject();
	 cJSON_AddStringToObject(root, "ssid", fweb_mesh_config.ssid);
	 cJSON_AddStringToObject(root, "password", fweb_mesh_config.password);
	 cJSON_AddStringToObject(root, "meshID", mesh_id);
	 cJSON_AddStringToObject(root, "mesh_password", fweb_mesh_config.meshappass);
	 cJSON_AddNumberToObject(root, "max_layer", fweb_mesh_config.max_layer);
	 cJSON_AddNumberToObject(root, "max_sta", fweb_mesh_config.max_sta);
	 cJSON_AddNumberToObject(root, "port", fweb_mesh_config.port);
	 const char *sys_info = cJSON_Print(root);
	 cJSON_Delete(root);
	 ESP_LOGI("JSON","%s",sys_info);
	 ESP_LOGI("DEBUG","%d",strlen(sys_info));

	 httpd_resp_send(req, sys_info, strlen(sys_info));

	return ESP_OK;
}

esp_err_t start_rest_server(const char *base_path)
{
	/* Validate file storage base path */
	if (!base_path || strcmp(base_path, CONFIG_EXAMPLE_WEB_MOUNT_POINT) != 0) {
		ESP_LOGE(REST_TAG, "File server presently supports only '/spiffs' as base path");
		return ESP_ERR_INVALID_ARG;
	}

    rest_server_context_t *rest_context = calloc(1, sizeof(rest_server_context_t));
    REST_CHECK(rest_context, "No memory for rest context", err);

    strlcpy(rest_context->base_path, base_path, sizeof(rest_context->base_path));

    httpd_handle_t server = NULL;
    httpd_config_t config = {
    		.task_priority      = tskIDLE_PRIORITY+5,       \
			.stack_size         = 1024*20,                  \
			.core_id            = tskNO_AFFINITY,           \
			.server_port        = 80,                       \
			.ctrl_port          = 32768,                    \
			.max_open_sockets   = 7,                        \
			.max_uri_handlers   = 8,                        \
			.max_resp_headers   = 8,                        \
			.backlog_conn       = 5,                        \
			.lru_purge_enable   = false,                    \
			.recv_wait_timeout  = 5,                        \
			.send_wait_timeout  = 5,                        \
			.global_user_ctx = NULL,                        \
			.global_user_ctx_free_fn = NULL,                \
			.global_transport_ctx = NULL,                   \
			.global_transport_ctx_free_fn = NULL,           \
			.open_fn = NULL,                                \
			.close_fn = NULL,                               \
			.uri_match_fn = NULL                            \

    };
    config.uri_match_fn = httpd_uri_match_wildcard;

    ESP_LOGI(REST_TAG, "Starting HTTP Server");
    REST_CHECK(httpd_start(&server, &config) == ESP_OK, "Start server failed", err_start);

    /* URI handler for getting web server files */
	httpd_uri_t mesh_get_uri = {
		.uri = "/formMesh",
		.method = HTTP_GET,
		.handler = form_mesh_get_handler,
		.user_ctx = rest_context
	};
	httpd_register_uri_handler(server, &mesh_get_uri);

	/* URI handler for getting web server files */
	httpd_uri_t mesh_form_uri = {
		.uri = "/req_mesh",
		.method = HTTP_POST,
		.handler = form_mesh_req_handler,
		.user_ctx = rest_context
	};
	httpd_register_uri_handler(server, &mesh_form_uri);

	/* URI handler for getting web server files */
	httpd_uri_t login_uri = {
		.uri = "/login",
		.method = HTTP_POST,
		.handler = login_req_handler,
		.user_ctx = rest_context
	};
	httpd_register_uri_handler(server, &login_uri);

    /* URI handler for getting web server files */
    httpd_uri_t common_get_uri = {
        .uri = "/*",
        .method = HTTP_GET,
        .handler = rest_common_get_handler,
        .user_ctx = rest_context
    };
    httpd_register_uri_handler(server, &common_get_uri);


    return ESP_OK;
err_start:
    free(rest_context);
err:
    return ESP_FAIL;
}

// Tarea RTOS para el servidor --------------------------------------------------//
void ServidorHTTP(){
	esp_wifi_restore();
	iniciar_wifi();
	initialise_mdns();
	ESP_ERROR_CHECK(init_fs());
    ESP_ERROR_CHECK(start_rest_server(CONFIG_EXAMPLE_WEB_MOUNT_POINT));
}
/*********************************************************************************/
