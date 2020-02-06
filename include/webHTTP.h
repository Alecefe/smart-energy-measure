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
};
TaskHandle_t http_socket;

void set_form_flash(struct form_home form);
void get_form_flash();
bool Llenar_intro();
bool Llenar_form_home(char * p, struct form_home form1);
/******************************************************************/
