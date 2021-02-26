#include "mesh.h"
#include <lwip/netdb.h>
#include <sys/param.h>
#include "UART1.h"
#include "driver/gpio.h"
#include "driver/timer.h"
#include "esp_event.h"
#include "esp_system.h"
#include "esp_websocket_client.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "math.h"
#include "meters_data.h"
#include "modbus_master.h"
#include "nvs_rotate.h"
#include "ram-heap.h"
#include "task_verify.h"
#include "tcpip_adapter.h"

#define CONFIG_SLAVE_QUANTITY 3
#define MODBUS_TIMEOUT 1000
#define TX_BUF_SIZE 1024
#define MMTAG "Modbus Master"
static uint8_t tx_buf[TX_SIZE] = {
    0,
};
static uint8_t rx_buf[RX_SIZE] = {
    0,
};
static bool is_mesh_connected = false;
static mesh_addr_t mesh_parent_addr;
static int mesh_layer = -1;

static const char *MESH_TAG = "mesh_main";
static const char *TCP_TAG = "TCP_SERVER";

uint8_t SLAVE_ID;
uint16_t fconv;
uint16_t port;
uint64_t energy_ini;
tipo_de_medidor tipo;
uint32_t baud_rate;

SemaphoreHandle_t smfPulso = NULL;
bool creador = true;

uint16_t start_address = 0x21, quantity = 0x02;
uint8_t slave_id[] = {0x01, 0x02, 0x0f};
mesh_modbus_meter meter[CONFIG_SLAVE_QUANTITY];

mesh_addr_t rt[CONFIG_MESH_ROUTE_TABLE_SIZE];

/********* Tareas del Root ************/

void esp_mesh_tx_to_ext(void *arg) {
  /*
   * Recibe la trama proveniente del nodo solicitado en la red mesh y
   * Envía la trama mediante el socket tcp creado en tcp_server_task
   *
   */

  mesh_addr_t from;
  mesh_data_t data;
  data.data = rx_buf;
  data.size = sizeof(rx_buf);
  data.proto = MESH_PROTO_BIN;
  int flag = 0, sendControl;
  esp_err_t error;
  char trama[tamBUFFER];

  INT_VAL len;
  while (esp_mesh_is_root()) {
    error = esp_mesh_recv(&from, &data, portMAX_DELAY, &flag, NULL, 0);
    if (error == ESP_OK) {
      len.Val = rx_buf[4] + rx_buf[5] + 6;
      for (int i = 0; i < len.Val; i++) {
        trama[i] = (char)rx_buf[i];
      }
      for (int i = 0; i < len.Val; i++) {
        printf("trama[%d] = %02x\r\n", i, trama[i]);
      }
      sendControl = send(men, trama, len.Val, 0);
      if (sendControl < 0 || sendControl != len.Val) {
        ESP_LOGE("Tx to Ext", "Error in send");
      }
    }
  }
  ESP_LOGE(MESH_TAG, "Se elimino tarea TX EXT");
  vTaskDelete(NULL);
}

static void https_client_task(void *pvParameters) {
  /*
   *	Tarea que maneja el servidor TCP, por aquí se realizan conexiones con
   *redes externas
   */

  char rx_buffer[128];
  char addr_str[128];
  int addr_family;
  int ip_protocol;
  int sock = 0;
  struct timeval timeout, timeout_listen;
  timeout.tv_sec = 20;
  timeout.tv_usec = 0;
  timeout_listen.tv_sec = 60 * 5;
  timeout_listen.tv_usec = 0;

#ifdef CONFIG_IPV4
  struct sockaddr_in dest_addr;
  dest_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  dest_addr.sin_family = AF_INET;
  dest_addr.sin_port = htons(port);
  addr_family = AF_INET;
  ip_protocol = IPPROTO_IP;
  inet_ntoa_r(dest_addr.sin_addr, addr_str, sizeof(addr_str) - 1);

#else  // IPV6
  struct sockaddr_in6 dest_addr;
  bzero(&dest_addr.sin6_addr.un, sizeof(dest_addr.sin6_addr.un));
  dest_addr.sin6_family = AF_INET6;
  dest_addr.sin6_port = htons(port);
  addr_family = AF_INET6;
  ip_protocol = IPPROTO_IPV6;
  inet6_ntoa_r(dest_addr.sin6_addr, addr_str, sizeof(addr_str) - 1);
#endif

  int listen_sock =
      socket(addr_family, SOCK_STREAM, ip_protocol);  // Crea el socket
  if (listen_sock < 0) {
    ESP_LOGE(TCP_TAG, "Unable to create socket: errno %d", errno);
  }

  if (setsockopt(listen_sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout_listen,
                 sizeof(timeout_listen)) == 0) {
    printf("Tiempo Listening %d s\r\n", (uint32_t)timeout_listen.tv_sec);
  }

  ESP_LOGI(TCP_TAG, "Socket created");

  int err = bind(listen_sock, (struct sockaddr *)&dest_addr,
                 sizeof(dest_addr));  // Asigna ip al socket
  if (err != 0) {
    ESP_LOGE(TCP_TAG, "Socket unable to bind: errno %d", errno);
  }
  ESP_LOGI(TCP_TAG, "Socket bound, port %d", port);

  while (esp_mesh_is_root()) {
    err = listen(listen_sock, 1);  // A la espera de las posibles conexiones
    if (err != 0) {
      ESP_LOGE(TCP_TAG, "Error occurred during listen: errno %d", errno);
      break;
    }
    ESP_LOGI(TCP_TAG, "Socket listening");

    struct sockaddr_in6 source_addr;  // Large enough for both IPv4 or IPv6
    uint addr_len = sizeof(source_addr);
    sock =
        accept(listen_sock, (struct sockaddr *)&source_addr,
               &addr_len);  // Se acepta la conexión en caso de poder realizarse

    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,
                   sizeof(timeout)) == 0) {
      printf("Configurado Timeout %d s\r\n", (uint32_t)timeout.tv_sec);
    }
    men = sock;

    if (sock < 0) {
      ESP_LOGE(TCP_TAG, "Unable to accept connection: errno %d", errno);
      continue;
    }
    ESP_LOGI(TCP_TAG, "Socket accepted");

    while (esp_mesh_is_root()) {
      printf("Recibiendo TCP...\r\n");
      int len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1,
                     0);  // Se queda esperando en el estado de recepción

      // Error occurred during receiving
      if (len < 0) {
        ESP_LOGE(TCP_TAG, "recv failed: errno %d", errno);
        timer_pause(TIMER_GROUP_0, TIMER_0);

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
          inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr.s_addr,
                      addr_str, sizeof(addr_str) - 1);
        } else if (source_addr.sin6_family == PF_INET6) {
          inet6_ntoa_r(source_addr.sin6_addr, addr_str, sizeof(addr_str) - 1);
        }

        rx_buffer[len] =
            0;  // Null-terminate whatever we received and treat like a string
        ESP_LOGI(TCP_TAG, "Received %d bytes from %s:", len, addr_str);
        ESP_LOGI(TCP_TAG, "%s", rx_buffer);

        xQueueSendToFront(RxSocket, rx_buffer, pdMS_TO_TICKS(1000));
      }
    }

    if (sock != -1) {
      ESP_LOGE(TCP_TAG, "Shutting down socket and restarting...");
      shutdown(sock, 0);
      close(sock);
      // socket(addr_family, SOCK_STREAM, ip_protocol);
    }
  }
  ESP_LOGE(MESH_TAG, "Se Elimino la tarea TCP\r\n");
  close(sock);
  vTaskDelete(NULL);
}

esp_err_t send_query_2all_nodes(uint8_t slave_id) {
  /*Tomar la routing table y enviar el modbus query con el actual slaveID a
   * todos los nodos*/
  esp_err_t err;
  mesh_data_t data;
  data.proto = MESH_PROTO_BIN;
  data.size = tamBUFFER;
  data.data = tx_buf;
  data.tos = MESH_TOS_P2P;

  read_input_register_frame(slave_id, start_address, quantity, data.data);

  int rt_len = 0;
  err =
      esp_mesh_get_routing_table(rt, CONFIG_MESH_ROUTE_TABLE_SIZE * 6, &rt_len);
  for (int j = 1; j < rt_len; j++) {
    err = esp_mesh_send(&rt[j], &data, MESH_DATA_P2P, NULL, 0);
    if (err != ESP_OK) {
      ESP_LOGW(MESH_TAG, "Query error %s slave_id: %02x" MACSTR,
               esp_err_to_name(err), slave_id, MAC2STR(meter[j].mac.addr));
      return err;
    }
  }
  return ESP_OK;
}

esp_err_t recv_node_response(uint8_t slave_id, uint8_t index) {
  /* Recibir los datos provenientes de la red y llenar la estructura de salida*/
  esp_err_t err;
  mesh_addr_t from;
  mesh_data_t data;
  data.data = rx_buf;
  data.size = sizeof(rx_buf);
  int flag = 0, frame_size = 9;

  err = esp_mesh_recv(&from, &data, pdMS_TO_TICKS(MODBUS_TIMEOUT), &flag, NULL,
                      0);
  if (err == ESP_OK) {
    if (CRC16(data.data, frame_size) == 0) {
      ESP_LOGI(MESH_TAG, "Modbus frame verified");
      check_exceptions(data.data);
      meter[index].mac = from;
      meter[index].slave_id = data.data[0];
      save_register(data.data, frame_size, meter[index].energy.energyReg);
    }
  } else {
    ESP_LOGW(MESH_TAG, "Response error %s Slave ID: %d", esp_err_to_name(err),
             slave_id);
    ESP_LOGI(MESH_TAG, "Frame not verified: %d", CRC16(data.data, frame_size));
    //    crc_error_response(dtmp);
    return err;
  }

  return ESP_OK;
}

void mesh_modbus_master(void *Pa) {
  /*
   *
   */
  esp_err_t err;
  while (esp_mesh_is_root()) {
    for (int i = 0; i < CONFIG_SLAVE_QUANTITY; i++) {
      send_query_2all_nodes(slave_id[i]);
      recv_node_response(slave_id[i], i);
    }
  }
  ESP_LOGE(MESH_TAG, "Se elimino tarea Modbus Master");
  vTaskDelete(NULL);
}

/********* Tareas de un Nodo ************/

/****************************/
/**** Bus RS485 Standard ****/
/****************************/

void esp_mesh_p2p_rx_main(void *arg) {
  INT_VAL len;
  INT_VAL CRC;
  esp_err_t err;
  mesh_addr_t from;
  mesh_data_t data;
  int flag = 0;
  data.data = rx_buf;
  data.size = sizeof(rx_buf);
  mesh_rx_pending_t pendientes, auxi = {.toSelf = 0};
  uint8_t trama[tamBUFFER];
  const unsigned char *aux = &trama[4];

  while (!esp_mesh_is_root()) {
    printf("Esperando...\r\n");
    err = esp_mesh_recv(&from, &data, portMAX_DELAY, &flag, NULL, 0);
    esp_mesh_get_rx_pending(&pendientes);
    if (pendientes.toSelf != auxi.toSelf) {
      printf("Pendientes = %d\r\n", pendientes.toSelf);
      auxi.toSelf = pendientes.toSelf;
    }
    switch (err) {
      case ESP_OK: {
        printf("Recibido por Mesh\r\n");

        if (!esp_mesh_is_root()) {
          len.Val = rx_buf[4] + rx_buf[5];

          for (uint16_t i = 0; i < len.Val; i++) {
            trama[i + 4] = rx_buf[6 + i];
          }
          CRC.Val = CRC16(aux, len.Val);
          len.Val = len.Val + 2;
          trama[len.Val + 2] = CRC.byte.LB;
          trama[len.Val + 3] = CRC.byte.HB;
          trama[0] = rx_buf[0];
          trama[1] = rx_buf[1];
          trama[2] = len.byte.HB;
          trama[3] = len.byte.LB;
          xQueueSendToFront(TxRS485, &trama, portMAX_DELAY);
        }
      } break;
      case ESP_ERR_MESH_NOT_START: {
        ESP_LOGE(MESH_TAG, "Aun no esta Iniciada la Mesh\r\n");

      } break;
      case ESP_ERR_MESH_TIMEOUT: {
        ESP_LOGW(MESH_TAG, "Timeout Error\r\n");
      } break;
      default:
        ESP_ERROR_CHECK_WITHOUT_ABORT(err);
        break;
    }
  }

  ESP_LOGW(MESH_TAG, "Tarea Rx main eliminada");
  vTaskDelete(NULL);
}

void bus_rs485(void *arg) {
  INT_VAL longitud;
  BaseType_t ctrl_cola;
  mesh_data_t dataMesh;
  dataMesh.proto = MESH_PROTO_BIN;
  dataMesh.size = tamBUFFER;
  dataMesh.data = tx_buf;
  dataMesh.tos = MESH_TOS_P2P;

  uint16_t txlen, txctrl;

  uint8_t dataTx[tamBUFFER];
  uint8_t *aux = &dataTx[4];

  while (!esp_mesh_is_root()) {
    // Espera a recibir la trama RTU

    xQueueReceive(TxRS485, (uint8_t *)dataTx, portMAX_DELAY);
    printf("Recibido en cola...\r\n");
    txlen = dataTx[2] + dataTx[3];

    txctrl = uart_write_bytes(uart1, (char *)aux, txlen);

    if (txctrl > 0) {
      ESP_LOGI("RS485", "Tx FIFO:%u", txctrl);
      ctrl_cola =
          xQueueReceive(RxRS485, (uint8_t *)tx_buf + 4, pdMS_TO_TICKS(10000));
      if (ctrl_cola == pdTRUE) {
        tx_buf[0] = dataTx[0];
        tx_buf[1] = dataTx[1];
        tx_buf[2] = 0;
        tx_buf[3] = 0;
        longitud.Val = tx_buf[4] + tx_buf[5] - 2;
        tx_buf[4] = longitud.byte.HB;
        tx_buf[5] = longitud.byte.LB;
        tx_buf[4 + longitud.Val] = 0x00;
        tx_buf[4 + longitud.Val + 1] = 0x00;
        for (int i = 0; i < (longitud.Val + 4); i++) {
          printf("tx_buf[%d] = %02x\r\n", i, tx_buf[i]);
        }
        esp_err_t err = esp_mesh_send(NULL, &dataMesh, MESH_DATA_P2P, NULL, 0);
        if (err == ESP_OK) {
          printf("Mandado\r\n");
        }
      } else {
        ESP_LOGW("RS485", "Queue timeout proceeding to UART1 Flush");
        uart_flush(UART_NUM_1);
      }
    }
  }
  vTaskDelete(NULL);
}

/****************************/
/**** Medidor de pulsos *****/
/****************************/

// INTERRUPCIONES
/*Interrupcion de entrada de pulso*/
void IRAM_ATTR INT_GPIO_PULSOS(void *arg) {
  xSemaphoreGiveFromISR(smfPulso, NULL);
}

// TAREAS
/*Tarea de comunicacion P2P con el sistema*/
void modbus_tcpip_pulsos(void *arg) {
  esp_err_t err;
  int flag = 0;
  mesh_addr_t from;
  mesh_data_t data_rx, data_tx;
  data_rx.data = rx_buf;
  data_rx.size = RX_SIZE;
  data_tx.data = tx_buf;
  data_tx.size = TX_SIZE;

  energytype_t energia;

  while (!esp_mesh_is_root()) {
    ESP_LOGI("MODBUS", "Entré aquí");
    err = esp_mesh_recv(&from, &data_rx, portMAX_DELAY, &flag, NULL, 0);
    ESP_LOGI("MODBUS", "Mensaje Recibido");
    ESP_LOGI("MODBUS RX", "%x %x %x %x %x %x %x %x %x %x %x %x", rx_buf[0],
             rx_buf[1], rx_buf[2], rx_buf[3], rx_buf[4], rx_buf[5], rx_buf[6],
             rx_buf[7], rx_buf[8], rx_buf[9], rx_buf[10], rx_buf[11]);

    if ((err == ESP_OK) && (rx_buf[6] == SLAVE_ID) && (rx_buf[7] == 0x03) &&
        (rx_buf[8] == MODBUS_ENERGY_REG_INIT_POS_H) &&
        (rx_buf[9] == MODBUS_ENERGY_REG_INIT_POS_L) &&
        (rx_buf[11] == MODBUS_ENERGY_REG_LEN)) {
      xQueuePeek(Cuenta_de_pulsos, &(energia.tot), portMAX_DELAY);
      tx_buf[0] = rx_buf[0];
      tx_buf[1] = rx_buf[1];
      tx_buf[2] = 0x00;
      tx_buf[3] = 0x00;
      tx_buf[4] = 0x00;      // len hb
      tx_buf[5] = 0x0b;      // len lb
      tx_buf[6] = SLAVE_ID;  // SlaveID
      tx_buf[7] = 0x03;
      tx_buf[8] = 0x08;  // byte count
      tx_buf[9] = energia.u8.lll8;
      tx_buf[10] = energia.u8.llll8;
      tx_buf[11] = energia.u8.l8;
      tx_buf[12] = energia.u8.ll8;
      tx_buf[13] = energia.u8.hh8;
      tx_buf[14] = energia.u8.h8;
      tx_buf[15] = energia.u8.hhhh8;
      tx_buf[16] = energia.u8.hhh8;
      err = esp_mesh_send(NULL, &data_tx, MESH_DATA_P2P, NULL, 0);
      if (err == ESP_OK) {
        ESP_LOGI("MODBUS", "Respuesta enviada");
      }
    }
    ESP_LOGI("MODBUS", "Final del ciclo");
  }
  vTaskDelete(NULL);
}

/*Tarea de guardado en flash de pulsos*/
static void rotar_nvs(void *arg) {
  /* Nomenclatura:
   * 	pf: Página fija. Es la pagina que necesita acceder el micro para saber
   *en qué página variable se encuentra. en ella se encuentra un registro de
   *igual nombre (pf) que contiene el número de paginas variables llenas.
   *
   * 	pv: Página variable. En esta página se guarda la cuenta de cada pulso
   *registrado. Las páginas variables como su nombre lo indica van variando a
   *medida que crece la cuenta de pulsos.
   *
   *	partition: Espacio de memoria flash del micro en la que se encuentra
   *actualmente el último pulso.
   */

  uint8_t partition_number;  // Index de partition name (app#)

  int32_t indexPv,         // Index del namespace (pv#)
      indexEntradaActual,  // Index de la entrada activa en la pv (e#)
      cuentaPulsosPv;      // Cuenta de pulsos en la entrada activa

  uint64_t total_pulsos,  // Total de pulsos: suma de lops pulsos en cada
                          // particion, página y entrada utilizada
      inicial_pulsos;
  char *pvActual,        // String para namespace de la página variable
      *entradaActualpv,  // String para el key de la entrada actual
      *partition_name;   // String para el nombre de la partición

  inicial_pulsos = round((float)energy_ini * (float)fconv);
  xQueueOverwrite(Cuenta_de_pulsos, &total_pulsos);

  esp_err_t err = search_init_partition(&partition_number);

  if (asprintf(&partition_name, "app%d", partition_number) < 0) {
    free(partition_name);
    ESP_LOGE("ROTAR_NVS", "Nombre de particion no fue creado");
  }

  nvs_flash_init_partition(partition_name);

  show_ram_status("Partición iniciada");

  /*
   * Leyendo el valor del contador de la pagina fija. Retorna el namespace y el
   * número de pagina variable se encuentran los datos más recientes
   */
  err = leer_contador_pf(&partition_name, &pvActual, &indexPv);
  if (err != ESP_OK)
    printf("Error (%s) leyendo contador desde la pagina fija!\n",
           esp_err_to_name(err));
  ESP_LOGI("LPF", "pvActual: %s indexPv: %d", pvActual, indexPv);

  ESP_LOGI("DEBUG", "Antes de leer pagina variable");

  /*
   * Revisar pagina variable actual y buscar el ultimo registro escrito
   */
  err = leer_pagina_variable(&partition_name, &pvActual, &indexEntradaActual,
                             &entradaActualpv, &cuentaPulsosPv);
  if (err != ESP_OK)
    printf("Error (%s) buscando el registro escrito más reciente\n",
           esp_err_to_name(err));
  ESP_LOGI("LPV",
           "pvActual: %s indexEntradaActual: %d Entrada actual: %s Cuenta "
           "pulsos: %d",
           pvActual, indexEntradaActual, entradaActualpv, cuentaPulsosPv);

  ESP_LOGI("DEBUG", "Antes del while");

  while (!esp_mesh_is_root()) {
    // Interrupción generada por pulsos
    if (xSemaphoreTake(smfPulso, portMAX_DELAY) == pdTRUE) {
      /*Si se llegó al límite en una particion*/
      if (indexPv == Limite_paginas_por_particion &&
          indexEntradaActual == Limite_entradas_por_pagina &&
          cuentaPulsosPv == Limite_pulsos_por_entrada) {
        partition_number++;

        if (partition_number <= max_particiones) {
          levantar_bandera(partition_name);

          char *aux;
          nvs_handle_t my_handle;

          // Cerrando la particion anterior
          if (asprintf(&aux, "app%d", partition_number - 1) < 0) {
            free(aux);
            ESP_LOGE("ROTAR_NVS", "Nombre de particion no fue creado");
          }
          ESP_LOGI("DEINIT", "%s", aux);
          err = nvs_flash_deinit_partition(aux);
          if (err != ESP_OK)
            ESP_LOGE("CNP", "ERROR (%s) IN DEINIT", esp_err_to_name(err));
          else
            free(aux);

          // Inicializando la nueva partición
          if (asprintf(&partition_name, "app%u", partition_number) < 0) {
            free(partition_name);
            ESP_LOGE("ROTAR_NVS", "Nombre de particion no fue creado");
          }
          ESP_LOGW("CNP", "Partition changed to %s", partition_name);
          nvs_flash_init_partition(partition_name);

          ESP_LOGI("PARTICION ACTUAL", "DEBUG 2 %s", partition_name);

          // Llenando particion
          esp_err_t err = nvs_open_from_partition(partition_name, "storage",
                                                  NVS_READWRITE, &my_handle);
          if (err != ESP_OK) ESP_LOGE("NVS", "ERROR IN NVS_OPEN");
          //		free(*pname);

          // Colocando la bandera de llenado en 0
          err = nvs_set_u8(my_handle, "finished", 0);
          if (err != ESP_OK) ESP_LOGE("NVS", "ERROR IN SET");
          err = nvs_commit(my_handle);
          if (err != ESP_OK) ESP_LOGE("NVS", "ERROR IN COMMIT");

          // Get del número de la partición
          err = nvs_get_u8(my_handle, "pnumber", &partition_number);
          if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND)
            ESP_LOGE("NVS", "ERROR IN GET");
          else if (err == ESP_ERR_NVS_NOT_FOUND) {
            err = nvs_set_u8(my_handle, "pnumber", partition_number);
            if (err != ESP_OK) ESP_LOGE("NVS", "ERROR IN SET");
            err = nvs_commit(my_handle);
            if (err != ESP_OK) ESP_LOGE("NVS", "ERROR IN COMMIT");
          }
          // Close
          nvs_close(my_handle);
        }
      }

      if (partition_number <= max_particiones) {
        err = contar_pulsos_nvs(&partition_name, &partition_number, &indexPv,
                                &indexEntradaActual, &cuentaPulsosPv);

        total_pulsos =
            inicial_pulsos +  // Aporte de los pulsos iniciales
            (partition_number - 1) * Limite_paginas_por_particion *
                Limite_entradas_por_pagina *
                Limite_pulsos_por_entrada +  // Aporte c/u de las particiones
            (indexPv - 1) * Limite_entradas_por_pagina *
                Limite_pulsos_por_entrada +  // Aporte de las paginas anteriores
            (indexEntradaActual - 1) *
                Limite_pulsos_por_entrada +  // Aporte de la página actual
            cuentaPulsosPv;                  // Aporte de la entrada actual

        ESP_LOGI("PULSOS", "PARTICION: %d PV: %d ENT: %d Pulsos: %d",
                 partition_number, indexPv, indexEntradaActual, cuentaPulsosPv);
        ESP_LOGI("TOTAL PULSOS", "%llu", total_pulsos);

        // Enviando los pulsos a otra tarea
        xQueueOverwrite(Cuenta_de_pulsos, &total_pulsos);
      } else
        ESP_LOGE("APP", "All partitions are full");
      show_ram_status("Por pulsos");
    }
  }

  free(pvActual);
  free(entradaActualpv);
  free(partition_name);
  ESP_LOGE("RTOS", "La tarea NVS-Rotative ha sido eliminada");
  vTaskDelete(NULL);
}

/********************************/
/**** Manejadores de eventos ****/
/********************************/

void mesh_event_handler(void *arg, esp_event_base_t event_base,
                        int32_t event_id, void *event_data) {
  char rx_child[9] = "Rx_child";
  char rx_rs485[9] = "Rx_RS485";
  char modbus_pulse[11] = "Commun P2P";
  char count_pulse[] = "Rotative NVS";
  mesh_addr_t id = {
      0,
  };
  static uint8_t last_layer = 0;

  switch (event_id) {
    case MESH_EVENT_STARTED: {
      esp_mesh_get_id(&id);
      ESP_LOGI(MESH_TAG, "<MESH_EVENT_MESH_STARTED>ID:" MACSTR "",
               MAC2STR(id.addr));
      is_mesh_connected = false;
      mesh_layer = esp_mesh_get_layer();
    } break;
    case MESH_EVENT_STOPPED: {
      ESP_LOGI(MESH_TAG, "<MESH_EVENT_STOPPED>");
      is_mesh_connected = false;
      mesh_layer = esp_mesh_get_layer();
    } break;
    case MESH_EVENT_CHILD_CONNECTED: {
      mesh_event_child_connected_t *child_connected =
          (mesh_event_child_connected_t *)event_data;
      ESP_LOGI(MESH_TAG, "<MESH_EVENT_CHILD_CONNECTED>aid:%d, " MACSTR "",
               child_connected->aid, MAC2STR(child_connected->mac));
    } break;
    case MESH_EVENT_CHILD_DISCONNECTED: {
      mesh_event_child_disconnected_t *child_disconnected =
          (mesh_event_child_disconnected_t *)event_data;
      ESP_LOGI(MESH_TAG, "<MESH_EVENT_CHILD_DISCONNECTED>aid:%d, " MACSTR "",
               child_disconnected->aid, MAC2STR(child_disconnected->mac));
    } break;
    case MESH_EVENT_ROUTING_TABLE_ADD: {
      mesh_event_routing_table_change_t *routing_table =
          (mesh_event_routing_table_change_t *)event_data;
      ESP_LOGW(MESH_TAG, "<MESH_EVENT_ROUTING_TABLE_ADD>add %d, new:%d",
               routing_table->rt_size_change, routing_table->rt_size_new);
    } break;
    case MESH_EVENT_ROUTING_TABLE_REMOVE: {
      mesh_event_routing_table_change_t *routing_table =
          (mesh_event_routing_table_change_t *)event_data;
      ESP_LOGW(MESH_TAG, "<MESH_EVENT_ROUTING_TABLE_REMOVE>remove %d, new:%d",
               routing_table->rt_size_change, routing_table->rt_size_new);
    } break;
    case MESH_EVENT_NO_PARENT_FOUND: {
      mesh_event_no_parent_found_t *no_parent =
          (mesh_event_no_parent_found_t *)event_data;
      ESP_LOGI(MESH_TAG, "<MESH_EVENT_NO_PARENT_FOUND>scan times:%d",
               no_parent->scan_times);
    }
    /* TODO handler for the failure */
    break;
    case MESH_EVENT_PARENT_CONNECTED: {
      gpio_set_level(LED_PAPA, 1);
      mesh_event_connected_t *connected = (mesh_event_connected_t *)event_data;
      esp_mesh_get_id(&id);
      mesh_layer = connected->self_layer;
      memcpy(&mesh_parent_addr.addr, connected->connected.bssid, 6);
      ESP_LOGI(
          MESH_TAG,
          "<MESH_EVENT_PARENT_CONNECTED>layer:%d-->%d, parent:" MACSTR
          "%s, ID:" MACSTR "",
          last_layer, mesh_layer, MAC2STR(mesh_parent_addr.addr),
          esp_mesh_is_root() ? "<ROOT>" : (mesh_layer == 2) ? "<layer2>" : "",
          MAC2STR(id.addr));
      last_layer = mesh_layer;
      is_mesh_connected = true;
      if (esp_mesh_is_root()) {
        tcpip_adapter_dhcpc_start(TCPIP_ADAPTER_IF_STA);
      } else {
        switch (tipo) {
          case (rs485):
            creador = vTaskB(rx_child);
            if (creador) {
              xTaskCreatePinnedToCore(esp_mesh_p2p_rx_main, rx_child, 3072 * 2,
                                      NULL, 5, NULL, 0);
            }
            creador = vTaskB(rx_rs485);
            if (creador) {
              xTaskCreatePinnedToCore(bus_rs485, rx_rs485, 3072 * 3, NULL, 5,
                                      NULL, 1);
              iniciarUART(tipo, baud_rate);
            }
            break;
          case (pulsos):
            creador = vTaskB(modbus_pulse);
            if (creador) {
              xTaskCreatePinnedToCore(modbus_tcpip_pulsos, modbus_pulse,
                                      3072 * 2, NULL, 5, NULL, 0);
            }
            creador = vTaskB(count_pulse);
            if (creador)
              xTaskCreatePinnedToCore(rotar_nvs, count_pulse, 4 * 1024, NULL, 5,
                                      NULL, 1);

            //        			creador = vTaskB(nvs_pulse);
            //        			if(creador){
            //        				xTaskCreatePinnedToCore(nvs_pulsos,nvs_pulse,3072*2,NULL,5,NULL,1);
            //        			}
            //        			creador = vTaskB(count_pulse);
            //        			if(creador){
            //        				xTaskCreatePinnedToCore(conteo_pulsos,count_pulse,3072*2,NULL,5,NULL,0);
            //        			}

            break;
          case (chino):
            creador = vTaskB(rx_child);
            if (creador) {
              xTaskCreatePinnedToCore(esp_mesh_p2p_rx_main, rx_child, 3072 * 2,
                                      NULL, 5, NULL, 0);
            }
            creador = vTaskB(rx_rs485);
            if (creador) {
              xTaskCreatePinnedToCore(bus_rs485, rx_rs485, 3072 * 3, NULL, 5,
                                      NULL, 1);
              iniciarUART(tipo, baud_rate);
            }
            break;
          case (enlace):
            break;
        }
      }
    } break;
    case MESH_EVENT_PARENT_DISCONNECTED: {
      gpio_set_level(LED_PAPA, 0);
      mesh_event_disconnected_t *disconnected =
          (mesh_event_disconnected_t *)event_data;
      ESP_LOGI(MESH_TAG, "<MESH_EVENT_PARENT_DISCONNECTED>reason:%d",
               disconnected->reason);
      is_mesh_connected = false;
      mesh_layer = esp_mesh_get_layer();
    } break;
    case MESH_EVENT_LAYER_CHANGE: {
      mesh_event_layer_change_t *layer_change =
          (mesh_event_layer_change_t *)event_data;
      mesh_layer = layer_change->new_layer;
      ESP_LOGI(
          MESH_TAG, "<MESH_EVENT_LAYER_CHANGE>layer:%d-->%d%s", last_layer,
          mesh_layer,
          esp_mesh_is_root() ? "<ROOT>" : (mesh_layer == 2) ? "<layer2>" : "");
      last_layer = mesh_layer;
    } break;
    case MESH_EVENT_ROOT_ADDRESS: {
      mesh_event_root_address_t *root_addr =
          (mesh_event_root_address_t *)event_data;
      ESP_LOGI(MESH_TAG, "<MESH_EVENT_ROOT_ADDRESS>root address:" MACSTR "",
               MAC2STR(root_addr->addr));
      for (int i = 0; i < 6; i++) {
        root_address.addr[i] = root_addr->addr[i];
      }

    } break;
    case MESH_EVENT_VOTE_STARTED: {
      mesh_event_vote_started_t *vote_started =
          (mesh_event_vote_started_t *)event_data;
      ESP_LOGI(
          MESH_TAG,
          "<MESH_EVENT_VOTE_STARTED>attempts:%d, reason:%d, rc_addr:" MACSTR "",
          vote_started->attempts, vote_started->reason,
          MAC2STR(vote_started->rc_addr.addr));
    } break;
    case MESH_EVENT_VOTE_STOPPED: {
      ESP_LOGI(MESH_TAG, "<MESH_EVENT_VOTE_STOPPED>");
      break;
    }
    case MESH_EVENT_ROOT_SWITCH_REQ: {
      mesh_event_root_switch_req_t *switch_req =
          (mesh_event_root_switch_req_t *)event_data;
      ESP_LOGI(MESH_TAG,
               "<MESH_EVENT_ROOT_SWITCH_REQ>reason:%d, rc_addr:" MACSTR "",
               switch_req->reason, MAC2STR(switch_req->rc_addr.addr));
    } break;
    case MESH_EVENT_ROOT_SWITCH_ACK: {
      /* new root */
      mesh_layer = esp_mesh_get_layer();
      esp_mesh_get_parent_bssid(&mesh_parent_addr);
      ESP_LOGI(MESH_TAG,
               "<MESH_EVENT_ROOT_SWITCH_ACK>layer:%d, parent:" MACSTR "",
               mesh_layer, MAC2STR(mesh_parent_addr.addr));
    } break;
    case MESH_EVENT_TODS_STATE: {
      mesh_event_toDS_state_t *toDs_state =
          (mesh_event_toDS_state_t *)event_data;
      ESP_LOGI(MESH_TAG, "<MESH_EVENT_TODS_REACHABLE>state:%d", *toDs_state);
    } break;
    case MESH_EVENT_ROOT_FIXED: {
      mesh_event_root_fixed_t *root_fixed =
          (mesh_event_root_fixed_t *)event_data;
      ESP_LOGI(MESH_TAG, "<MESH_EVENT_ROOT_FIXED>%s",
               root_fixed->is_fixed ? "fixed" : "not fixed");
    } break;
    case MESH_EVENT_ROOT_ASKED_YIELD: {
      mesh_event_root_conflict_t *root_conflict =
          (mesh_event_root_conflict_t *)event_data;
      ESP_LOGI(MESH_TAG,
               "<MESH_EVENT_ROOT_ASKED_YIELD>" MACSTR ", rssi:%d, capacity:%d",
               MAC2STR(root_conflict->addr), root_conflict->rssi,
               root_conflict->capacity);
    } break;
    case MESH_EVENT_CHANNEL_SWITCH: {
      mesh_event_channel_switch_t *channel_switch =
          (mesh_event_channel_switch_t *)event_data;
      ESP_LOGI(MESH_TAG, "<MESH_EVENT_CHANNEL_SWITCH>new channel:%d",
               channel_switch->channel);
    } break;
    case MESH_EVENT_SCAN_DONE: {
      mesh_event_scan_done_t *scan_done = (mesh_event_scan_done_t *)event_data;
      ESP_LOGI(MESH_TAG, "<MESH_EVENT_SCAN_DONE>number:%d", scan_done->number);
    } break;
    case MESH_EVENT_NETWORK_STATE: {
      mesh_event_network_state_t *network_state =
          (mesh_event_network_state_t *)event_data;
      ESP_LOGI(MESH_TAG, "<MESH_EVENT_NETWORK_STATE>is_rootless:%d",
               network_state->is_rootless);
    } break;
    case MESH_EVENT_STOP_RECONNECTION: {
      ESP_LOGI(MESH_TAG, "<MESH_EVENT_STOP_RECONNECTION>");
    } break;
    case MESH_EVENT_FIND_NETWORK: {
      mesh_event_find_network_t *find_network =
          (mesh_event_find_network_t *)event_data;
      ESP_LOGI(MESH_TAG,
               "<MESH_EVENT_FIND_NETWORK>new channel:%d, router BSSID:" MACSTR
               "",
               find_network->channel, MAC2STR(find_network->router_bssid));
    } break;
    case MESH_EVENT_ROUTER_SWITCH: {
      mesh_event_router_switch_t *router_switch =
          (mesh_event_router_switch_t *)event_data;
      ESP_LOGI(MESH_TAG,
               "<MESH_EVENT_ROUTER_SWITCH>new router:%s, channel:%d, " MACSTR
               "",
               router_switch->ssid, router_switch->channel,
               MAC2STR(router_switch->bssid));
    } break;
    default:
      ESP_LOGI(MESH_TAG, "unknown id:%d", event_id);
      break;
  }
}

void ip_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id,
                      void *event_data) {
  char tcp_server[11] = "tcp_server";
  char tx_root[3] = "TX";
  char tx_ext[7] = "TX_EXT";
  ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
  ESP_LOGI(MESH_TAG, "<IP_EVENT_STA_GOT_IP>IP:%s",
           ip4addr_ntoa(&event->ip_info.ip));
  creador = vTaskB(tcp_server);
  if (creador) {
    xTaskCreatePinnedToCore(https_client_task, "https_client_task", 1024 * 4,
                            NULL, 1, NULL, 1);
  }
  creador = vTaskB(tx_root);
  if (creador) {
    xTaskCreatePinnedToCore(mesh_modbus_master, "TX", 1024 * 3, NULL, 5, NULL,
                            0);
  }
  creador = vTaskB(tx_ext);
  if (creador) {
    xTaskCreatePinnedToCore(esp_mesh_tx_to_ext, "TX_EXT", 3072 * 2, NULL, 5,
                            NULL, 0);
  }
}

/****************************************/
/**** Configuracion e inicializacion ****/
/****************************************/

void config_gpio_pulsos(tipo_de_medidor tipo) {
  if (tipo == pulsos) {
    ESP_LOGI(MESH_TAG, "Configurando GPIO para medidor tipo pulsos");
    // Tipo de interrupcion
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    gpio_pad_select_gpio(PULSOS);  // configuro el BOTON_SALVAR como un pin GPIO
    gpio_set_direction(
        PULSOS,
        GPIO_MODE_DEF_INPUT);  // seleciono el PULSOS como pin de entrada
    gpio_isr_handler_add(PULSOS, INT_GPIO_PULSOS,
                         NULL);  // añado el manejador para el servicio ISR
    gpio_set_intr_type(PULSOS, GPIO_INTR_POSEDGE);  // habilito interrupción por
                                                    // flanco descendente (1->0)
    // tg0_timer_init(TIMER_0, TEST_WITH_RELOAD, TIMER_INTERVAL0_SEC);

  } else {
    ESP_LOGI(MESH_TAG, "Configurando GPIO para medidor tipo RS485");
    gpio_pad_select_gpio(RS485);
    gpio_set_direction(RS485, GPIO_MODE_DEF_OUTPUT);
  }
  gpio_pad_select_gpio(LED_PAPA);
  gpio_set_direction(LED_PAPA, GPIO_MODE_DEF_OUTPUT);
}

/*Inicio Mesh*/

void mesh_init(form_mesh fmesh, form_locwifi fwifi, form_modbus fmodbus) {
  esp_err_t err = ESP_OK;
  show_ram_status("Before mesh init");

  // Inicialización de variables necesarias
  port = fmesh.port;              // Puerto para conexiones TCP/IP
  SLAVE_ID = fmodbus.slaveid;     // Modbus slave ID
  baud_rate = fmodbus.baud_rate;  // Modbus baudrate para com. serial
  tipo = str2enum(fmodbus.tipo);  // Tipo de medidor (Pulsos, RS485)
  energy_ini = fmodbus.energia;   // Expresada en kWh
  fconv = fmodbus.conversion;     // Factor de conversión para los pulsos

  RxSocket = xQueueCreate(5, 128);
  TxRS485 = xQueueCreate(5, 128);
  RxlenRS485 = xQueueCreate(5, 2);
  TCPsend = xQueueCreate(5, 128);

  smfPulso = xSemaphoreCreateBinary();
  Cuenta_de_pulsos = xQueueCreate(1, 8);

  // Ajuste de pines según función
  config_gpio_pulsos(tipo);

  /*  tcpip initialization */
  tcpip_adapter_init();

  /* for mesh
   * stop DHCP server on softAP interface by default
   * stop DHCP client on station interface by default
   * */
  tcpip_adapter_dhcps_stop(TCPIP_ADAPTER_IF_AP);
  tcpip_adapter_dhcpc_stop(TCPIP_ADAPTER_IF_STA);

  /*  event initialization */
  esp_event_loop_create_default();

  /*  wifi initialization */
  wifi_init_config_t config = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&config));
  ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                             &ip_event_handler, NULL));
  ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_FLASH));
  ESP_ERROR_CHECK(esp_wifi_start());
  ESP_LOGI("WIFI", "%s", esp_err_to_name(err));

  /*  mesh initialization */
  esp_mesh_init();
  esp_event_handler_register(MESH_EVENT, ESP_EVENT_ANY_ID, &mesh_event_handler,
                             NULL);
  esp_mesh_set_max_layer(fmesh.max_layer);
  esp_mesh_set_vote_percentage(1);
  esp_mesh_set_ap_assoc_expire(10);
  mesh_cfg_t cfg = MESH_INIT_CONFIG_DEFAULT();

  /* mesh ID */
  memcpy((uint8_t *)&cfg.mesh_id, fmesh.mesh_id, 6);

  /* router */
  cfg.channel = CONFIG_MESH_CHANNEL;
  cfg.router.ssid_len = strlen(fwifi.ssid);
  memcpy((uint8_t *)&cfg.router.ssid, fwifi.ssid, cfg.router.ssid_len);
  memcpy((uint8_t *)&cfg.router.password, fwifi.password,
         strlen(fwifi.password));

  /* mesh softAP */
  esp_mesh_set_ap_authmode(CONFIG_MESH_AP_AUTHMODE);
  cfg.mesh_ap.max_connection = fmesh.max_sta;
  memcpy((uint8_t *)&cfg.mesh_ap.password, fmesh.meshappass,
         strlen(fmesh.meshappass));
  esp_mesh_set_config(&cfg);

  /* mesh start */
  esp_mesh_start();
  show_ram_status("After mesh init");
}
