/**************************************************************************/
#include "include/WiFiconfig.h"

SemaphoreHandle_t change_wifi_mode=NULL;

// Interrupcion de timer ------------------------------------------------//
void IRAM_ATTR timer_group0_isr(void *para)
{
    /* Retrieve the interrupt status and the counter value
       from the timer that reported the interrupt */
		TIMERG0.hw_timer[TIMER_0].update = 1;

    /* Clear the interrupt
       and update the alarm time for the timer with without reload */
        TIMERG0.int_clr_timers.t0 = 1;

    /* After the alarm has been triggered
      we need enable it again, so it is triggered the next time */
    TIMERG0.hw_timer[TIMER_0].config.alarm_en = TIMER_ALARM_EN;
    timer_set_counter_value(TIMER_GROUP_0, TIMER_0, 0x00000000ULL);
    xSemaphoreGiveFromISR(change_wifi_mode,NULL);
}

static void tg0_timer_init(int timer_idx,
    bool auto_reload, double timer_interval_sec)
{
    /* Select and initialize basic parameters of the timer */
    timer_config_t config;
    config.divider = TIMER_DIVIDER;
    config.counter_dir = TIMER_COUNT_UP;
    config.counter_en = TIMER_PAUSE;
    config.alarm_en = TIMER_ALARM_EN;
    config.intr_type = TIMER_INTR_LEVEL;
    config.auto_reload = auto_reload;
    timer_init(TIMER_GROUP_0, timer_idx, &config);

    /* Timer's counter will initially start from value below.
       Also, if auto_reload is set, this value will be automatically reload on alarm */
    timer_set_counter_value(TIMER_GROUP_0, timer_idx, 0x00000000ULL);

    /* Configure the alarm value and the interrupt on alarm. */
    timer_set_alarm_value(TIMER_GROUP_0, timer_idx, timer_interval_sec * TIMER_SCALE);
    timer_enable_intr(TIMER_GROUP_0, timer_idx);
    timer_isr_register(TIMER_GROUP_0, timer_idx, timer_group0_isr,
        (void *) timer_idx, ESP_INTR_FLAG_IRAM, NULL);

    timer_start(TIMER_GROUP_0, timer_idx);
}

//Supervisión del timer
void supervisor_sta(void *arg){
	while (1){
			xSemaphoreTake(change_wifi_mode,portMAX_DELAY);
			config_wifi_ap();
			ESP_LOGE("Supervisor","Eliminada la tarea de supervisión cambiando a modo AP");
			vTaskDelete(NULL);
		}
	}

// Manejador de eventos --------------------------------------------------//
static void ManejadorEventos(void* arg, esp_event_base_t event_base,
        int32_t event_id, void* event_data){
	switch(event_id){
	    case SYSTEM_EVENT_STA_START:
	    	xTaskCreate(&supervisor_sta,"TimeoutSTA",2048*1,NULL,5,&sup_sta);
	        tg0_timer_init(TIMER_0, WITHOUT_RELOAD, TIMER_INTERVAL0_SEC);
	        esp_wifi_connect();
	        break;
	    case SYSTEM_EVENT_STA_CONNECTED:
	    	timer_pause(TIMER_GROUP_0, TIMER_0);
	    	timer_set_counter_value(TIMER_GROUP_0, TIMER_0, 0x00000000ULL);
	    	vTaskDelete(sup_sta);
	    	break;
	    case SYSTEM_EVENT_STA_DISCONNECTED:
	    	if (connection_trys<=3){
	    		connection_trys++;
	        printf("\n\nConexion perdida con el Punto de Acceso\n\n");
	        vTaskDelay(1000/portTICK_PERIOD_MS);
	        printf("\nReconectando...\n");
	        esp_wifi_connect();}else{
	        	printf("\n\nNo se pudo conectar a la red\n\n");
	        }
	        break;
	    case WIFI_EVENT_AP_STACONNECTED:{
	            wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
	            ESP_LOGI("SoftAP", "station "MACSTR" join, AID=%d",
	                     MAC2STR(event->mac), event->aid);
	    }
	            break;
	    case WIFI_EVENT_AP_STADISCONNECTED:{
	            wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
	            ESP_LOGI("SoftAP", "station "MACSTR" leave, AID=%d",
	                     MAC2STR(event->mac), event->aid);
	    }
	            break;
	    default:
	        break;
	    }
}

// Funciones para configurar el WiFi -------------------------//

void config_wifi_sta(void){
	esp_wifi_disconnect();
	esp_wifi_stop();
	//Caso STA
	wifi_config_t wifi_config_sta = {
		.sta = {
			.ssid = nombreWiFi,
			.password = claveWiFi
		},
	};
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config_sta);
    esp_wifi_start();
}

void config_wifi_ap(void){
	esp_wifi_disconnect();
	esp_wifi_stop();
    //Caso softAP
    wifi_config_t wifi_config_ap = {
        .ap = {
        		.ssid = ssid_ap,
				.ssid_len = strlen(ssid_ap),
				.password = clave_ap,
				.max_connection = MAX_STA_CONN,
				.authmode = WIFI_AUTH_WPA_WPA2_PSK
        },

    };
    esp_wifi_set_mode(WIFI_MODE_AP);
    esp_wifi_set_config(WIFI_IF_AP, &wifi_config_ap);
    esp_wifi_start();
}

//Funcion de inicializacion del WiFi

void iniciar_wifi(void){
	change_wifi_mode = xSemaphoreCreateBinary();
	tcpip_adapter_init();
	ESP_ERROR_CHECK(esp_event_loop_create_default());
	    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	    esp_wifi_init(&cfg);
	    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &ManejadorEventos, NULL));
	    esp_wifi_set_storage(WIFI_STORAGE_RAM);
	    config_wifi_sta();
}

void config_gpio(){
	gpio_pad_select_gpio(PIN_CONFIG_1);
	gpio_set_direction(PIN_CONFIG_1,GPIO_MODE_DEF_INPUT);
}
/**************************************************************************/
