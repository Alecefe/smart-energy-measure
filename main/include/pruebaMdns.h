#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "tcpip_adapter.h"
#include "mdns.h"
#include "driver/gpio.h"
#include <sys/socket.h>
#include <netdb.h>
#define CONFIG_MDNS_HOSTNAME "polilla"
#define EXAMPLE_MDNS_INSTANCE "ESP32-WebDemo"
#define CONFIG_EXAMPLE_WEB_MOUNT_POINT "/www"

void initialise_mdns(void);
esp_err_t init_fs(void);
char* generate_hostname();
