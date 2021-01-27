/******************************************************************/
#ifndef MAIN_SERVERHTTP_H_
#define MAIN_SERVERHTTP_H_
// Inclusion de librerias ----------------------------------------//
//#include "WiFiconfig.h"
#include <stdbool.h>
#include "WiFiconfig.h"
#include "forms.h"

#define PUERTO 80

// Funcion prototipo RTOS ----------------------------------------//
void ServidorHTTP();
#endif

tipo_de_medidor str2enum(const char *str);

// MANEJADOR DE HTTP SOCKET
TaskHandle_t http_socket;

// FUNCIONES SET
void set_form_flash_mesh(form_mesh form);
void set_form_flash_modbus(form_modbus form);

// FUNCIONES GET
void get_form_flash_mesh(form_mesh *form);
void get_form_flash_locwifi(form_locwifi *form);
void get_form_flash_modbus(form_modbus *form);

// FUNCIONES FIIL
bool fill_form_mesh(char *p, form_mesh *form);
bool fill_form_modbus(char *p, form_modbus *form);

/******************************************************************/
