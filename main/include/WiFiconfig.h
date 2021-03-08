/************************************************************/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
// Definiciones de SSID y password -------------------------//
#define nombreWiFi "Estelita2.1"
#define claveWiFi "pancho$5gomita"
#define ssid_ap "EstelitaESP"
#define clave_ap "pancho$5gomita"
#define MAX_STA_CONN 10
#define PIN_CONFIG_1 0
// Funcion prototipo para inicializacion del WiFi ----------//
void iniciar_wifi(void);
void config_gpio();
void config_wifi_ap(void);
void config_wifi_sta(void);
void supervisor_sta(void *arg);
// Definiciones de timer---------//
#define TIMER_DIVIDER 16  //  Hardware timer clock divider
#define TIMER_SCALE \
  (TIMER_BASE_CLK / TIMER_DIVIDER)  // convert counter value to seconds
#define TIMER_INTERVAL0_SEC (30.0)  // sample test interval for the first timer
#define WITHOUT_RELOAD 0            // testing will be done without auto reload
#define WITH_RELOAD 1               // testing will be done with auto reload

uint8_t connection_trys;
TaskHandle_t sup_sta;

/************************************************************/
