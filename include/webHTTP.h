/******************************************************************/
#ifndef MAIN_SERVERHTTP_H_
#define MAIN_SERVERHTTP_H_
// Inclusion de librerias ----------------------------------------//
#include "lwip/api.h"
#include "esp_log.h"
#include "WiFiconfig.h"

#define PUERTO 80

// Funcion prototipo RTOS ----------------------------------------//
void tareaSOCKET(void *P);
#endif

struct form_home{
	char ssid[20];
	char password[20];
	uint8_t mesh_id[6];
	char meshappass [20];
	uint8_t max_layer;
	uint8_t max_sta;
	uint16_t port;
	uint64_t energia;
	char tipo[7];
	uint8_t slaveid;
	uint16_t conversion;
	uint32_t baud_rate;
};
TaskHandle_t http_socket;

typedef enum{
	rs485 = 0,
	pulsos,
	chino,
	enlace
} tipo_de_medidor;

typedef struct {
	tipo_de_medidor val;
	const char* str;
} conversion_t;

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



tipo_de_medidor str2enum (const char *str);

void set_form_flash(struct form_home form);
void get_form_flash();
bool Llenar_intro(char *p);
bool Llenar_form_home(char * p, struct form_home form1);
/******************************************************************/
