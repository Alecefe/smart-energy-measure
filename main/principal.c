#include "include/mesh.h"
#include "include/ram-heap.h"
#define Pila 1024

static const char *MESH_INIT = "MESH_INIT";

void app_main(){

	/**************** CONFIGURACION DE GPIO PARA RECONOCIMIENTO DEL PULSADOR DE INICIO ****************/
	config_gpio();

	show_ram_status("Antes de nvs");
	/**************** INICIALIZACION DE LA FLASH (PARTICION DE CONFIGURACION) *******************/
    esp_err_t err = nvs_flash_init_partition("nvs");
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase_partition("nvs"));
        err = nvs_flash_init_partition("nvs");
    }

    /**************** DECLARACION DE VARIABLES ********************************************************/
	form_mesh form_mesh;
	form_modbus form_modbus;
	form_locwifi form_locwifi;

    /**************** EXTRACCIÓN DE VARIABLES EN FLASH ************************************************/
	get_form_flash_mesh(&form_mesh);
	get_form_flash_locwifi(&form_locwifi);
	get_form_flash_modbus(&form_modbus);
	tipo_de_medidor tipo;

    /**************** PROGRAMA PRINCIPAL ********************************************************/
	ESP_LOGE("DEBUG","%s %d",form_modbus.tipo,strlen(form_modbus.tipo));
	tipo = str2enum(form_modbus.tipo);

	if(gpio_get_level(PIN_CONFIG_1)==0){
		ServidorHTTP();
	}else{
		ESP_LOGW(MESH_INIT,"SSID = %s",form_locwifi.ssid);
		ESP_LOGW(MESH_INIT,"PASSWORD = %s",form_locwifi.password);
		ESP_LOGW(MESH_INIT,"MESH ID = "MACSTR,MAC2STR(form_mesh.mesh_id));
		ESP_LOGW(MESH_INIT,"MESH PASSWORD = %s",form_mesh.meshappass);
		ESP_LOGW(MESH_INIT,"MESH MAX LAYER = %d",form_mesh.max_layer);
		ESP_LOGW(MESH_INIT,"MESH MAX STA = %d",form_mesh.max_sta);
		ESP_LOGW(MESH_INIT,"MESH PORT = %d",form_mesh.port);
		ESP_LOGW(MESH_INIT,"Type of Meter: %s",form_modbus.tipo);

		switch(tipo){

		case(rs485):
				ESP_LOGW(MESH_INIT,"MESH METER BAUD RATE = %u",form_modbus.baud_rate);
		break;

		case(pulsos):
			ESP_LOGW(MESH_INIT,"MESH METER INITIAL ENERGY = %"PRIu64"pulsos",form_modbus.energia);
			ESP_LOGW(MESH_INIT,"MESH SLAVE ID = %d",form_modbus.slaveid);
			ESP_LOGW(MESH_INIT,"Conversion Factor: %d imp/kWh",form_modbus.conversion);
		break;

		case(chino):
			ESP_LOGW(MESH_INIT,"MESH METER BAUD RATE = %"PRIu32,form_modbus.baud_rate);
		break;

		case(enlace):
		break;

		default:
		break;
		}

		/**************** INICIALIZACION DE LA FLASH (PARTICION DE APLICACION) *******************/
//		ESP_ERROR_CHECK(nvs_flash_deinit_partition("nvs"));
//		show_ram_status("Luego de cerrar nvs");
//	    esp_err_t err = nvs_flash_init_partition("app1");
//	    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
//	        // NVS partition was truncated and needs to be erased
//	        // Retry nvs_flash_init
//	        ESP_ERROR_CHECK(nvs_flash_erase_partition("app1"));
//	        err = nvs_flash_init_partition("app1");
//	    }
//	    show_ram_status("Luego de app1");

	    /**************** APLICACION MESH *******************/
	    mesh_init(form_mesh, form_locwifi, form_modbus);
	}
}
