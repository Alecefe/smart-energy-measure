#include "include/mesh.h"
#define Pila 1024

static const char *MESH_INIT = "MESH_INIT";

void app_main(){
	config_gpio();
	nvs_flash_init();
	struct form_home form;
	get_form_flash(&form);
	if(gpio_get_level(PIN_CONFIG_1)==0){
		xTaskCreatePinnedToCore(&tareaSOCKET,"SOCKET_HTTP",Pila*3,NULL,3,&http_socket,1);
	}else{
		ESP_LOGW(MESH_INIT,"SSID = %s",form.ssid);
		ESP_LOGW(MESH_INIT,"PASSWORD = %s",form.password);
		ESP_LOGW(MESH_INIT,"MESH ID = "MACSTR,MAC2STR(form.mesh_id));
		ESP_LOGW(MESH_INIT,"MESH PASSWORD = %s",form.meshappass);
		ESP_LOGW(MESH_INIT,"MESH MAX LAYER = %d",form.max_layer);
		ESP_LOGW(MESH_INIT,"MESH MAX STA = %d",form.max_sta);
		ESP_LOGW(MESH_INIT,"MESH PORT = %d",form.port);
		ESP_LOGW(MESH_INIT,"MESH METER INITIAL ENERGY = %"PRIu64"kWh",form.energia);
		ESP_LOGW(MESH_INIT,"MESH SLAVE ID = %d",form.slaveid);
		ESP_LOGW(MESH_INIT,"Tipo de Medidor en flash: %s",form.tipo);
		ESP_LOGW(MESH_INIT,"Factor de conversion en flash: %d imp/kWh",form.conversion);
		mesh_init(form);
	}
}
