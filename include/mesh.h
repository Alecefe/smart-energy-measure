#ifndef MAIN_MESH_H_
#define MAIN_MESH_H_
#include <sys/param.h>
#include "esp_system.h"
#include "esp_event.h"
#include "tcpip_adapter.h"
#include "esp_websocket_client.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>
#include "UART1.h"
#include "driver/timer.h"
#include "math.h"

#define PULSOS 0//23
#define SALVAR 22
#define RS485 21
#define LED_PAPA 2
#define RX_SIZE          (1500)
#define TX_SIZE          (1460)
#define CONFIG_MESH_ROUTE_TABLE_SIZE 50
#define ESP_INTR_FLAG_DEFAULT 0
#define CONFIG_MESH_AP_AUTHMODE WIFI_AUTH_WPA2_PSK
#define CONFIG_MESH_CHANNEL 0
#define MODBUS_ENERGY_REG_INIT_POS_H (0x00)
#define MODBUS_ENERGY_REG_INIT_POS_L (0x18)
#define MODBUS_ENERGY_REG_LEN (0x04)

/*******************************************************
 *                Variable Definitions
 *******************************************************/

QueueHandle_t RxSocket;
QueueHandle_t TxRS485;

QueueHandle_t RxlenRS485;
QueueHandle_t TCPsend;
TaskHandle_t timer;


QueueHandle_t Cuenta_de_pulsos;

mesh_addr_t root_address;

#ifdef CONFIG_EXAMPLE_IPV4
#define HOST_IP_ADDR CONFIG_EXAMPLE_IPV4_ADDR
#else
#define HOST_IP_ADDR CONFIG_EXAMPLE_IPV6_ADDR
#endif

#define PORT CONFIG_EXAMPLE_PORT

int men;



void IRAM_ATTR interrupcion_pulsos (void* arg);

void IRAM_ATTR guadado_en_flash(void* arg);

bool vTaskB( char *nombre_tarea );

/*Root*/
void esp_mesh_tx_to_ext(void *arg);

void esp_mesh_p2p_tx_main(void *Pa);


/*Nodo medidor por RS485*/
void esp_mesh_p2p_rx_main(void *arg);

void bus_rs485(void *arg);

void modbus_tcpip_pulsos(void *arg);

/*Nodo medidor por pulsos*/

void nvs_pulsos(void *arg);

void conteo_pulsos (void *arg);

/*Manejadores de eventos*/
void mesh_event_handler(void *arg, esp_event_base_t event_base,
                        int32_t event_id, void *event_data);

void ip_event_handler(void *arg, esp_event_base_t event_base,
                      int32_t event_id, void *event_data);

/*Inicializacion*/
void mesh_init(struct form_home form);

#endif

