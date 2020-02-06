/************************************************************/
#ifndef MAIN_WIFICONFIG_H_
#define MAIN_WIFICONFIG_H_
// Inclusion de librerias ----------------------------------//
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
// Definiciones de SSID y password -------------------------//
#define nombreWiFi "PruebaDeAcceso"
#define claveWiFi  "12345678"
#define MAX_STA_CONN 10
#define PIN_CONFIG_1 0
// Funcion prototipo para inicializacion del WiFi ----------//
void iniciar_wifi(void);
void config_gpio();
#endif
/************************************************************/