#include "WiFiconfig.h"
#include "webHTTP.h"
#define Pila 1024
const char*MESH_TAG = "MESH";

void app_main(){
	config_gpio();
	nvs_flash_init();
	struct form_home form;
	get_form_flash(&form);
	if(gpio_get_level(PIN_CONFIG_1)==0){
		xTaskCreatePinnedToCore(&tareaSOCKET,"SOCKET_HTTP",Pila*3,NULL,3,&http_socket,1);
	}else{
		ESP_LOGW(MESH_TAG,"SSID = %s",form.ssid);
		ESP_LOGW(MESH_TAG,"PASSWORD = %s",form.password);
		ESP_LOGW(MESH_TAG,"MESH ID = "MACSTR,MAC2STR(form.mesh_id));
		ESP_LOGW(MESH_TAG,"MESH PASSWORD = %s",form.meshappass);
		ESP_LOGW(MESH_TAG,"MESH MAX LAYER = %d",form.max_layer);
		ESP_LOGW(MESH_TAG,"MESH MAX STA = %d",form.max_sta);
		ESP_LOGW(MESH_TAG,"MESH PORT = %d",form.port);
	}
}
