#include "esp_err.h"
#define CONFIG_MDNS_HOSTNAME "polilla"
#define EXAMPLE_MDNS_INSTANCE "ESP32-WebDemo"
#define CONFIG_EXAMPLE_WEB_MOUNT_POINT "/www"

void initialise_mdns(void);
esp_err_t init_fs(void);
char* generate_hostname();
