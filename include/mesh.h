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

#define PULSOS
#define SALVAR
#define RS485 21
#define LED_PAPA 2
#define RX_SIZE          (1500)
#define TX_SIZE          (1460)
#define CONFIG_MESH_ROUTE_TABLE_SIZE 50
#define ESP_INTR_FLAG_DEFAULT 0
#define CONFIG_MESH_AP_AUTHMODE WIFI_AUTH_WPA2_PSK
#define CONFIG_MESH_CHANNEL 0

/*******************************************************
 *                Variable Definitions
 *******************************************************/


QueueHandle_t RxSocket;
QueueHandle_t TxRS485;

QueueHandle_t RxlenRS485;
QueueHandle_t TCPsend;
TaskHandle_t timer;

QueueHandle_t Cuenta_de_pulsos;
SemaphoreHandle_t smfPulso = NULL;
SemaphoreHandle_t smfNVS = NULL;

mesh_addr_t root_address;

#ifdef CONFIG_EXAMPLE_IPV4
#define HOST_IP_ADDR CONFIG_EXAMPLE_IPV4_ADDR
#else
#define HOST_IP_ADDR CONFIG_EXAMPLE_IPV6_ADDR
#endif

#define PORT CONFIG_EXAMPLE_PORT

int men;

typedef union{
	uint64_t tot;
	struct{
		uint32_t l32;
		uint32_t h32;
	}u32;
	struct{
		uint16_t ll16;
		uint16_t l16;
		uint16_t h16;
		uint16_t hh16;
	}u16;
	struct{
		uint8_t llll8;
		uint8_t lll8;
		uint8_t ll8;
		uint8_t l8;
		uint8_t h8;
		uint8_t hh8;
		uint8_t hhh8;
		uint8_t hhhh8;
	}u8;
}energytype_t;

typedef enum{
	rs485 = 0,
	pulsos,
	chino,
	enlace
} tipo_de_medidor;

struct {
	tipo_de_medidor val;
	const char* str;
} conversion[] = {{rs485,"rs485"},{pulsos,"pulsos"},{chino,"chino"},{enlace,"enlace"},};

tipo_de_medidor str2enum (const char *str);

/*Root*/
void esp_mesh_tx_to_ext(void *arg);
void esp_mesh_p2p_tx_main(void *Pa);
/*Nodo medidor por RS485*/
void esp_mesh_p2p_rx_main(void *arg);
void bus_rs485(void *arg);
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

