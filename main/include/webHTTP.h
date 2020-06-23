/******************************************************************/
#ifndef MAIN_SERVERHTTP_H_
#define MAIN_SERVERHTTP_H_
// Inclusion de librerias ----------------------------------------//
#include <string.h>
#include <fcntl.h>
#include "esp_http_server.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_vfs.h"
#include "cJSON.h"
#include "lwip/api.h"
#include "esp_log.h"
#include "WiFiconfig.h"
#include "pruebaMdns.h"

#define PUERTO 80

// Funcion prototipo RTOS ----------------------------------------//
void ServidorHTTP();
#endif

// ESTRUCTURAS DE DATOS PARA FORMULARIOS
typedef struct form_locwifi{
	char ssid[20];
	char password[20];
}form_locwifi;

typedef struct form_modbus{
	char tipo[7];
	uint64_t energia;
	uint8_t slaveid;
	uint16_t conversion;
	uint32_t baud_rate;
}form_modbus;

typedef struct form_login{
	char user[21];
	char password[21];
}form_login;

typedef struct form_mesh{
	uint8_t mesh_id[6];
	char meshappass[20];
	uint8_t max_layer;
	uint8_t max_sta;
	uint16_t port;
}form_mesh;

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

typedef struct form_mqtt{
	uint8_t advance;
	char uri[30];
	char ip[20];
	uint16_t port;
	char pubtopic[20];
	uint8_t type;
	char user[20];
	char password[20];
	uint8_t app_layer;
}form_mqtt;

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

// MANEJADOR DE HTTP SOCKET
TaskHandle_t http_socket;

// FUNCIONES SET
void set_form_flash_mesh(form_mesh form);
void set_form_flash_modbus(form_modbus form);

//FUNCIONES GET
void get_form_flash_mesh(form_mesh *form);
void get_form_flash_locwifi(form_locwifi *form);
void get_form_flash_modbus(form_modbus *form);

//FUNCIONES FIIL
bool fill_form_mesh(char * p, form_mesh *form);
bool fill_form_modbus(char *p,form_modbus *form);


/******************************************************************/
