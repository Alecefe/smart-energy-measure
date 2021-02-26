#ifndef MAIN_MESH_H_
#define MAIN_MESH_H_
#include <serverHTTP.h>
#include "CRC.h"
#include "esp_mesh.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"

// DEFINICIONES
#define PULSOS 35
#define SALVAR 22
#define RS485 21
#define LED_PAPA 2
#define RX_SIZE (1500)
#define TX_SIZE (1460)
#define CONFIG_MESH_ROUTE_TABLE_SIZE 50
#define ESP_INTR_FLAG_DEFAULT 0
#define CONFIG_MESH_AP_AUTHMODE WIFI_AUTH_WPA2_PSK
#define CONFIG_MESH_CHANNEL 0
#define MODBUS_ENERGY_REG_INIT_POS_H (0x00)
#define MODBUS_ENERGY_REG_INIT_POS_L (0x18)
#define MODBUS_ENERGY_REG_LEN (0x04)
#define BIT_0 (1 << 0)

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

EventGroupHandle_t prueba;

#ifdef CONFIG_EXAMPLE_IPV4
#define HOST_IP_ADDR CONFIG_EXAMPLE_IPV4_ADDR
#else
#define HOST_IP_ADDR CONFIG_EXAMPLE_IPV6_ADDR
#endif

#define PORT CONFIG_EXAMPLE_PORT

int men;

EventGroupHandle_t prueba;

typedef struct {
  int type;  // the type of timer's event
  int timer_group;
  int timer_idx;
  uint64_t timer_counter_value;
} timer_event_t;

typedef union {
  uint32_t energy;
  uint16_t *energyReg[2];
  struct {
    INT_VAL LowReg;
    INT_VAL HighReg;
  } registers;
} energy_t;

typedef struct {
  mesh_addr_t mac;
  uint8_t slave_id;
  energy_t energy;
  char date[25];
} mesh_modbus_meter;

esp_err_t recv_node_response(uint8_t slave_id, uint8_t index);
esp_err_t frame_qry_2all_nodes(uint8_t slave_id, mesh_data_t *data);
void IRAM_ATTR interrupcionGPIOPULSOS(void *arg);
void esp_mesh_tx_to_ext(void *arg);
void esp_mesh_p2p_tx_main(void *Pa);
void esp_mesh_p2p_rx_main(void *arg);
void bus_rs485(void *arg);
void modbus_tcpip_pulsos(void *arg);
void nvs_pulsos(void *arg);
void conteo_pulsos(void *arg);
void mesh_event_handler(void *arg, esp_event_base_t event_base,
                        int32_t event_id, void *event_data);
void ip_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id,
                      void *event_data);
void mesh_init(form_mesh form_mesh, form_locwifi form_locwifi,
               form_modbus form_modbus);
#endif
