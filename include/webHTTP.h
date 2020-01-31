/******************************************************************/
#ifndef MAIN_SERVERHTTP_H_
#define MAIN_SERVERHTTP_H_
// Inclusion de librerias ----------------------------------------//
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lwip/api.h"
#include <string.h>
#include "WiFiconfig.h"
#define PUERTO 80
#define LEDg 23
#define LEDb 22
#define LEDr 21

// Funcion prototipo RTOS ----------------------------------------//
void tareaSOCKET(void *P);
#endif
struct form_home{
	char ssid[20];
	char password[20];
	uint8_t mesh_id[6];
	uint8_t max_layer;
	uint8_t max_sta;
	uint16_t port;
};

void Llenar_form_home(char * p, struct form_home form1);
/******************************************************************/
