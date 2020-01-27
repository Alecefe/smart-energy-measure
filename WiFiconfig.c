/**************************************************************************/
#include "WiFiconfig.h"

// Manejador de eventos --------------------------------------------------//
static esp_err_t ManejadorEventos(void *ctx, system_event_t *evento){
	switch(evento->event_id){
	    case SYSTEM_EVENT_STA_START:
	        esp_wifi_connect();
	        break;
	    case SYSTEM_EVENT_STA_DISCONNECTED:
	        printf("\n\nConexion perdida con el Punto de Acceso\n\n");
	        vTaskDelay(1000/portTICK_PERIOD_MS);
	        printf("\nReconectando...\n");
	        esp_wifi_connect();
	        break;
	    default:
	        break;
	    }
	return ESP_OK;
}
// Funcion para configurar el WiFi como estacion -------------------------//
void iniciar_wifi(void){
	tcpip_adapter_init();
	    esp_event_loop_init(ManejadorEventos, NULL);
	    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	    esp_wifi_init(&cfg);
	    esp_wifi_set_storage(WIFI_STORAGE_RAM);
	    wifi_config_t wifi_config = {
	        .ap = {
	        		.ssid = nombreWiFi,
					.ssid_len = strlen(nombreWiFi),
					.password = claveWiFi,
					.max_connection = MAX_STA_CONN,
					.authmode = WIFI_AUTH_WPA_WPA2_PSK
	        },
	    };

	    esp_wifi_set_mode(WIFI_MODE_AP);
	    esp_wifi_set_config(WIFI_IF_AP, &wifi_config);
	    esp_wifi_start();
}
/**************************************************************************/
