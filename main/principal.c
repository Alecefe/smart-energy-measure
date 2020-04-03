#include "include/mesh.h"
#define Pila 1024

static const char *MESH_INIT = "MESH_INIT";

void app_main(){

	config_gpio();
	nvs_flash_init();
	struct form_home form;
	get_form_flash_mesh(&form);
	tipo_de_medidor tipo;
	tipo = str2enum(form.tipo);
	if(gpio_get_level(PIN_CONFIG_1)==0){
		ServidorHTTP();
	}else{
		ESP_LOGW(MESH_INIT,"SSID = %s",form.ssid);
		ESP_LOGW(MESH_INIT,"PASSWORD = %s",form.password);
		ESP_LOGW(MESH_INIT,"MESH ID = "MACSTR,MAC2STR(form.mesh_id));
		ESP_LOGW(MESH_INIT,"MESH PASSWORD = %s",form.meshappass);
		ESP_LOGW(MESH_INIT,"MESH MAX LAYER = %d",form.max_layer);
		ESP_LOGW(MESH_INIT,"MESH MAX STA = %d",form.max_sta);
		ESP_LOGW(MESH_INIT,"MESH PORT = %d",form.port);
		ESP_LOGW(MESH_INIT,"Type of Meter: %s",form.tipo);
		switch(tipo){

		case(rs485):
				ESP_LOGW(MESH_INIT,"MESH METER BAUD RATE = %"PRIu32,form.baud_rate);
		break;

		case(pulsos):
			ESP_LOGW(MESH_INIT,"MESH METER INITIAL ENERGY = %"PRIu64"pulsos",form.energia);
			ESP_LOGW(MESH_INIT,"MESH SLAVE ID = %d",form.slaveid);
			ESP_LOGW(MESH_INIT,"Conversion Factor: %d imp/kWh",form.conversion);
		break;

		case(chino):
			ESP_LOGW(MESH_INIT,"MESH METER BAUD RATE = %"PRIu32,form.baud_rate);
		break;

		case(enlace):
		break;

		default:
		break;
		}
		mesh_init(form);
	}
}
