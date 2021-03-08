/* LwIP SNTP example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include "esp_attr.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "esp_sntp.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "nvs_flash.h"

static const char *TAG = "sntp";

static void initialize_sntp(void);
static void obtain_time(void);

void print_time(time_t now, struct tm t) {
  time(&now);
  localtime_r(&now, &t);
  ESP_LOGW("simple_ntp", "%04i-%02i-%02i %02i:%02i:%02i -0400",
           t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min,
           t.tm_sec);
}

char *time_string(time_t *now, struct tm *t) {
  static char string[64];
  time(now);
  localtime_r(now, t);
  sprintf(string, "%04i-%02i-%02i %02i:%02i:%02i -0400", t->tm_year + 1900,
          t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
  return string;
}

void time_sync_notification_cb(struct timeval *tv) {
  ESP_LOGI(TAG, "Notification of a time synchronization event");
}

void simple_ntp() {
  time_t now;
  struct tm timeinfo;
  time(&now);
  localtime_r(&now, &timeinfo);
  // Is time set? If not, tm_year will be (1970 - 1900).
  if (timeinfo.tm_year < (2016 - 1900)) {
    ESP_LOGI(
        TAG,
        "Time is not set yet. Connecting to WiFi and getting time over NTP.");
    obtain_time();
    // update 'now' variable with current time
    time(&now);
  }
  // Venezuela standard time
  setenv("TZ", "GMT", 1);
  tzset();
  localtime_r(&now, &timeinfo);
  ESP_LOGW("Tiempo", "%04i-%02i-%02i %02i:%02i:%02i", timeinfo.tm_year + 1900,
           timeinfo.tm_mon + 1, timeinfo.tm_mday, timeinfo.tm_hour,
           timeinfo.tm_min, timeinfo.tm_sec);

  if (sntp_get_sync_mode() == SNTP_SYNC_MODE_SMOOTH) {
    struct timeval outdelta;
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_IN_PROGRESS) {
      adjtime(NULL, &outdelta);
      ESP_LOGI(
          TAG,
          "Waiting for adjusting time ... outdelta = %li sec: %li ms: %li us",
          outdelta.tv_sec, outdelta.tv_usec / 1000, outdelta.tv_usec % 1000);
      vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
  }
}

static void obtain_time(void) {
  initialize_sntp();
  // wait for time to be set
  time_t now = 0;
  struct tm timeinfo = {0};
  int retry = 0;
  const int retry_count = 10;
  while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET &&
         ++retry < retry_count) {
    ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry,
             retry_count);
    vTaskDelay(2000 / portTICK_PERIOD_MS);
  }
  time(&now);
  localtime_r(&now, &timeinfo);
}

static void initialize_sntp(void) {
  ESP_LOGI(TAG, "Initializing SNTP");
  sntp_setoperatingmode(SNTP_OPMODE_POLL);
  sntp_setservername(0, "pool.ntp.org");
  sntp_set_time_sync_notification_cb(time_sync_notification_cb);
  sntp_init();
}
