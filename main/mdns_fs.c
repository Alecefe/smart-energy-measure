#include <mdns_fs.h>
#include <netdb.h>
#include <string.h>
#include <sys/socket.h>
#include "driver/gpio.h"
#include "driver/sdmmc_host.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_spiffs.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "mdns.h"
#include "nvs_flash.h"
#include "sdkconfig.h"
#include "tcpip_adapter.h"

static const char* TAG = "mdns-test";

char* generate_hostname() {
#ifdef CONFIG_MDNS_ADD_MAC_TO_HOSTNAME
  return strdup(CONFIG_MDNS_HOSTNAME);
#else
  uint8_t mac[6];
  char* hostname;
  esp_read_mac(mac, ESP_MAC_WIFI_SOFTAP);
  if (-1 == asprintf(&hostname, "%s-%02X%02X%02X", CONFIG_MDNS_HOSTNAME, mac[3],
                     mac[4], mac[5])) {
    abort();
  }
  return hostname;
#endif
}

void initialise_mdns(void) {
  char* hostname = generate_hostname();
  // initialize mDNS
  ESP_ERROR_CHECK(mdns_init());
  // set mDNS hostname (required if you want to advertise services)
  ESP_ERROR_CHECK(mdns_hostname_set(hostname));
  ESP_LOGI(TAG, "mdns hostname set to: [%s]", hostname);
  // set default mDNS instance name
  ESP_ERROR_CHECK(mdns_instance_name_set(EXAMPLE_MDNS_INSTANCE));

  // structure with TXT records
  mdns_txt_item_t serviceTxtData[3] = {
      {"board", "esp32"}, {"u", "user"}, {"p", "password"}};

  // initialize service
  ESP_ERROR_CHECK(mdns_service_add("ESP32-WebServer", "_http", "_tcp", 80,
                                   serviceTxtData, 3));
  // add another TXT item
  ESP_ERROR_CHECK(
      mdns_service_txt_item_set("_http", "_tcp", "path", "/foobar"));
  // change TXT item value
  ESP_ERROR_CHECK(mdns_service_txt_item_set("_http", "_tcp", "u", "admin"));
  free(hostname);
}

esp_err_t init_fs(void) {
  esp_vfs_spiffs_conf_t conf = {.base_path = CONFIG_EXAMPLE_WEB_MOUNT_POINT,
                                .partition_label = NULL,
                                .max_files = 5,
                                .format_if_mount_failed = false};
  esp_err_t ret = esp_vfs_spiffs_register(&conf);

  if (ret != ESP_OK) {
    if (ret == ESP_FAIL) {
      ESP_LOGE(TAG, "Failed to mount or format filesystem");
    } else if (ret == ESP_ERR_NOT_FOUND) {
      ESP_LOGE(TAG, "Failed to find SPIFFS partition");
    } else {
      ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
    }
    return ESP_FAIL;
  }

  size_t total = 0, used = 0;
  ret = esp_spiffs_info(NULL, &total, &used);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)",
             esp_err_to_name(ret));
  } else {
    ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
  }
  return ESP_OK;
}
