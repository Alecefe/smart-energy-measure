/*********************************************************************************/
#include "include/webHTTP.h"
#include "esp_log.h"
#include "math.h"
const char *nvs_tag = "NVS";
const char *http_server = "HTTP SERVER";
static const char *REST_TAG = "esp-rest";
static const char *TAG = "REQUEST";
const char valid_user[] ="admin";
const char valid_pass[] ="admin";

form_login fweb_login;
form_mesh fweb_mesh_config;
form_modbus fweb_modbus;
form_mqtt fweb_mqtt;
form_locwifi fweb_locwifi;


#define REST_CHECK(a, str, goto_tag, ...)                                              \
    do                                                                                 \
    {                                                                                  \
        if (!(a))                                                                      \
        {                                                                              \
            ESP_LOGE(REST_TAG, "%s(%d): " str, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
            goto goto_tag;                                                             \
        }                                                                              \
    } while (0)

#define FILE_PATH_MAX (ESP_VFS_PATH_MAX + 128)
#define SCRATCH_BUFSIZE (10240)

bool ahora = true;
conversion_t conversion[4] = {{rs485,"rs485"},{pulsos,"pulsos"},{chino,"chino"},{enlace,"enlace"}};

typedef struct rest_server_context {
    char base_path[ESP_VFS_PATH_MAX + 1];
    char scratch[SCRATCH_BUFSIZE];
} rest_server_context_t;

tipo_de_medidor str2enum (const char *str)
{    int j;
     for (j = 0;  j < sizeof (conversion) / sizeof (conversion[0]);  ++j)
         if (!strcmp (str, conversion[j].str))
             return conversion[j].val;
     ESP_LOGI(http_server,"Tipo no valido");
     return enlace;
}

#define CHECK_FILE_EXTENSION(filename, ext) (strcasecmp(&filename[strlen(filename) - strlen(ext)], ext) == 0)

/********************** LLENADO DE FORMULARIOS (RAM->FLASH // SET) *************************/

void set_form_flash_mesh(form_mesh form){

	esp_err_t err;
	nvs_handle_t ctrl_mesh;
	err = nvs_open("storage",NVS_READWRITE,&ctrl_mesh);
	if (err != ESP_OK) {
		printf("Error (%s) opening NVS handle!\n\r", esp_err_to_name(err));
	}else{

		ESP_LOGI("FROM_NVS",MACSTR,MAC2STR(form.mesh_id));
		err=nvs_set_blob(ctrl_mesh,"meshid",form.mesh_id,sizeof(form.mesh_id));
		if (err != ESP_OK) {printf("Error (%s) setting in flash mesh id!\n\r", esp_err_to_name(err));}

		ESP_LOGI("FROM_NVS","%s",form.meshappass);
		err=nvs_set_str(ctrl_mesh,"meshpass",form.meshappass);
		if (err != ESP_OK) {printf("Error (%s) setting in flash mesh password!\n\r", esp_err_to_name(err));}

		ESP_LOGI("FROM_NVS","%d",form.max_layer);
		err=nvs_set_u8(ctrl_mesh,"max_layer",form.max_layer);
		if (err != ESP_OK) {printf("Error (%s) setting in flash max layer!\n\r", esp_err_to_name(err));}

		ESP_LOGI("FROM_NVS","%d",form.max_sta);
		err=nvs_set_u8(ctrl_mesh,"max_sta",form.max_sta);
		if (err != ESP_OK) {printf("Error (%s) setting in flash max STA!\n\r", esp_err_to_name(err));}

		ESP_LOGI("FROM_NVS","%d",form.port);
		err=nvs_set_u16(ctrl_mesh,"port",form.port);
		if (err != ESP_OK) {printf("Error (%s) setting in flash port!\n\r", esp_err_to_name(err));}

		err = nvs_commit(ctrl_mesh);
		if (err != ESP_OK) {printf("Error (%s) while the commit stage!\n\r", esp_err_to_name(err));}
	}
	nvs_close(ctrl_mesh);
}

void set_form_flash_modbus(form_modbus form){
	esp_err_t err;
	nvs_handle_t ctrl_modbus;

	err = nvs_open("storage",NVS_READWRITE,&ctrl_modbus);
	if (err != ESP_OK) {
		printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
	}else{

		/*Pub Topic*/
		ESP_LOGI("FROM_NVS","%s",form.tipo);
		err = nvs_set_str(ctrl_modbus," mod-tipo",form.tipo);
		if (err != ESP_OK) {printf("Error (%s) setting in flash meter type!\n\r", esp_err_to_name(err));}

		/*Slave ID*/
		ESP_LOGI("FROM_NVS","%u",form.slaveid);
		nvs_set_u8(ctrl_modbus,"mod-slaveid",form.slaveid);
		if (err != ESP_OK) {printf("Error (%s) setting in flash slave ID!\n\r", esp_err_to_name(err));}

		/*Conversion Factor*/
		ESP_LOGI("FROM_NVS","%u",form.conversion);
		nvs_set_u16(ctrl_modbus,"mod-convfac",form.conversion);
		if (err != ESP_OK) {printf("Error (%s) setting in flash conv. factor!\n\r", esp_err_to_name(err));}

		/*Baud Rate*/
		ESP_LOGI("FROM_NVS","%u",form.baud_rate);
		nvs_set_u32(ctrl_modbus,"mod-brate",form.baud_rate);
		if (err != ESP_OK) {printf("Error (%s) setting in flash Baud Rate!\n\r", esp_err_to_name(err));}

		/*Energía Inicial*/
		ESP_LOGI("FROM_NVS","%llu",form.energia);
		nvs_set_u64(ctrl_modbus,"mod-energia",form.energia);
		if (err != ESP_OK) {printf("Error (%s) setting in flash Initial Energy!\n\r", esp_err_to_name(err));}

		err = nvs_commit(ctrl_modbus);
		if (err != ESP_OK){ESP_LOGW("NVS_COMMIT","Error(%s)", esp_err_to_name(err));}
	}
	nvs_close(ctrl_modbus);
}

void set_form_flash_login(form_login form){

	esp_err_t err;
	nvs_handle_t ctrl_login;
	err = nvs_open("storage",NVS_READWRITE,&ctrl_login);
	if (err != ESP_OK) {
		printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
	}else{

		/*User login*/
		ESP_LOGI("FROM_NVS","%s",form.user);
		err=nvs_set_str(ctrl_login,"log-user",form.user);
		if (err != ESP_OK) {printf("Error (%s) setting in flash user login!\n\r", esp_err_to_name(err));}

		/*User password*/
		ESP_LOGI("FROM_NVS","%s",form.password);
		err=nvs_set_str(ctrl_login,"log-user",form.password);
		if (err != ESP_OK) {printf("Error (%s) setting in flash user password!\n\r", esp_err_to_name(err));}
	}
	nvs_close(ctrl_login);
}

void set_form_flash_locwifi(form_locwifi form){

	esp_err_t err;
	nvs_handle_t ctrl_locwifi;
	err = nvs_open("storage",NVS_READWRITE,&ctrl_locwifi);
	if (err != ESP_OK) {
		printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
	}else{
		/*SSID*/
		ESP_LOGI("FROM_NVS","%s",form.ssid);
		err=nvs_set_str(ctrl_locwifi,"wifi-ssid",form.ssid);
		if (err != ESP_OK) {printf("Error (%s) setting in flash SSID!\n\r", esp_err_to_name(err));}

		/*Password*/
		ESP_LOGI("FROM_NVS","%s",form.ssid);
		err=nvs_set_str(ctrl_locwifi,"wifi-pass",form.ssid);
		if (err != ESP_OK) {printf("Error (%s) setting in flash WiFi Pass!\n\r", esp_err_to_name(err));}
	}
	nvs_close(ctrl_locwifi);
}

void set_form_flash_mqtt(form_mqtt form){

	esp_err_t err;
	nvs_handle_t ctrl_mqtt;
	err = nvs_open("storage",NVS_READWRITE,&ctrl_mqtt);
	if (err != ESP_OK) {
		printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
	}else{

		/*PORT*/
		ESP_LOGI("FROM_NVS","%d",form.port);
		err=nvs_set_u16(ctrl_mqtt,"mqtt-port",form.port);
		if (err != ESP_OK) {printf("Error (%s) setting in flash MQTT port!\n\r", esp_err_to_name(err));}

		/*Pub Topic*/
		ESP_LOGI("FROM_NVS","%s",form.pubtopic);
		err=nvs_set_str(ctrl_mqtt,"mqtt-top",form.pubtopic);
		if (err != ESP_OK) {printf("Error (%s) setting in flash MQTT Pub. Topic!\n\r", esp_err_to_name(err));}

		/*Advanced config*/
		ESP_LOGI("FROM_NVS","%d",form.advance);
		err=nvs_set_u8(ctrl_mqtt,"mqtt-ad",form.advance);
		if (err != ESP_OK) {printf("Error (%s) setting in flash advanced config!\n\r", esp_err_to_name(err));}

		/*URI*/
		ESP_LOGI("FROM_NVS","%s",form.uri);
		err=nvs_set_str(ctrl_mqtt,"mqtt-uri",form.uri);
		if (err != ESP_OK) {printf("Error (%s) setting in flash URI!\n\r", esp_err_to_name(err));}

		/*Type of broker*/
		ESP_LOGI("FROM_NVS","%d",form.type);
		err=nvs_set_u8(ctrl_mqtt,"mqtt-type",form.type);
		if (err != ESP_OK) {printf("Error (%s) setting in flash Type!\n\r", esp_err_to_name(err));}

		/*App Layer*/
		ESP_LOGI("FROM_NVS","%d",form.app_layer);
		err=nvs_set_u8(ctrl_mqtt,"mqtt-app",form.app_layer);
		if (err != ESP_OK) {printf("Error (%s) setting in flash app layer!\n\r", esp_err_to_name(err));}

		/*MQTT user*/
		ESP_LOGI("FROM_NVS","%s",form.user);
		err=nvs_set_str(ctrl_mqtt,"mqtt-user",form.user);
		if (err != ESP_OK) {printf("Error (%s) setting in flash MQTT User!\n\r", esp_err_to_name(err));}

		/*MQTT pass*/
		ESP_LOGI("FROM_NVS","%s",form.password);
		err=nvs_set_str(ctrl_mqtt,"mqtt-pass",form.password);
		if (err != ESP_OK) {printf("Error (%s) setting in flash MQTT Pass!\n\r", esp_err_to_name(err));}
	}
	nvs_close(ctrl_mqtt);
}

/********************** LLENADO DE FORMULARIOS (FLASH->RAM // GET) *************************/

void get_form_flash_mesh(form_mesh *form){
	size_t len;
	char mac[18];
	esp_err_t err;
	nvs_handle_t ctrl_mesh;

	err = nvs_open("storage",NVS_READWRITE,&ctrl_mesh);
	if (err != ESP_OK) {
		printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
	}else{
		/*Mesh ID*/
		err = nvs_get_blob(ctrl_mesh,"meshid",NULL,&len);
		if(err==ESP_OK){
		err = nvs_get_blob(ctrl_mesh,"meshid",form->mesh_id,&len);
		switch(err){
			case ESP_OK:
				sprintf(mac,MACSTR,MAC2STR(form->mesh_id));
				ESP_LOGI(nvs_tag,"Mesh id en flash: %s",mac);
			break;
			case ESP_ERR_NVS_NOT_FOUND:
				ESP_LOGI(nvs_tag,"Mesh id en flash: none");
			break;
			default:
				printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
			break;}
			}

		/*Mesh password*/
		err = nvs_get_str(ctrl_mesh,"meshpass",NULL,&len);
		if(err==ESP_OK){
			err= nvs_get_str(ctrl_mesh,"meshpass",form->meshappass,&len);
		switch(err){
			case ESP_OK:
				ESP_LOGI(nvs_tag,"Mesh password en flash: %s",form->meshappass);
			break;
			case ESP_ERR_NVS_NOT_FOUND:
				ESP_LOGI(nvs_tag,"Mesh password en flash: none");
			break;
			default:
				printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
			break;}
		}

		/*Max Layer*/
		err = nvs_get_u8(ctrl_mesh,"max_layer",&(form->max_layer));
		switch(err){
			case ESP_OK:
				ESP_LOGI(nvs_tag,"Max. Layer en flash: %d",form->max_layer);
			break;
			case ESP_ERR_NVS_NOT_FOUND:
				ESP_LOGI(nvs_tag,"Max. Layer en flash: none");
			break;
			default:
				printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
			break;
		}

		/*Max stations connected*/
		err = nvs_get_u8(ctrl_mesh,"max_sta",&(form->max_sta));
		switch(err){
			case ESP_OK:
				ESP_LOGI(nvs_tag,"Max. Sta en flash: %d",form->max_sta);
			break;
			case ESP_ERR_NVS_NOT_FOUND:
				ESP_LOGI(nvs_tag,"Max. Sta en flash: none");
			break;
			default:
				printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
			break;
		}

		/*Port*/
		err = nvs_get_u16(ctrl_mesh,"port",&(form->port));
		switch(err){
			case ESP_OK:
				ESP_LOGI(nvs_tag,"Port en flash: %d",form->port);
			break;
			case ESP_ERR_NVS_NOT_FOUND:
				ESP_LOGI(nvs_tag,"Port en flash: none");
			break;
			default:
				printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
			break;
		}

//		err = nvs_get_str(ctrl_mesh,"tipo",NULL,&len);
//		if(err==ESP_OK){
//			err= nvs_get_str(ctrl_mesh,"tipo",form->tipo,&len);
//		switch(err){
//			case ESP_OK:
//				ESP_LOGI(nvs_tag,"Tipo en flash: %s",form->tipo);
//			break;
//			case ESP_ERR_NVS_NOT_FOUND:
//				ESP_LOGI(nvs_tag,"Tipo en flash: none");
//			break;
//			default:
//				printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
//			break;
//		}
//		}
//		switch(str2enum(form->tipo)){
//
//		case(rs485):
//				err = nvs_get_u32(ctrl_mesh,"baud",&(form->baud_rate));
//				switch(err){
//					case ESP_OK:
//						ESP_LOGI(nvs_tag,"Baud Rate en flash: %"PRIu32,form->baud_rate);
//					break;
//					case ESP_ERR_NVS_NOT_FOUND:
//						ESP_LOGI(nvs_tag,"Baud Rate en flash: none");
//					break;
//					default:
//						printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
//					break;
//				}
//				break;
//		case(pulsos):
//				err = nvs_get_u64(ctrl_mesh,"energy",&(form->energia));
//				switch(err){
//					case ESP_OK:
//						ESP_LOGI(nvs_tag,"Pulsos en flash: %"PRIu64,form->energia);
//					break;
//					case ESP_ERR_NVS_NOT_FOUND:
//						ESP_LOGI(nvs_tag,"Pulsos en flash: none");
//					break;
//					default:
//						printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
//					break;
//				}
//				err = nvs_get_u8(ctrl_mesh,"slaveid",&(form->slaveid));
//				switch(err){
//					case ESP_OK:
//						ESP_LOGI(nvs_tag,"Slave ID en medidor a pulsos en flash: %d",form->slaveid);
//					break;
//					case ESP_ERR_NVS_NOT_FOUND:
//						ESP_LOGI(nvs_tag,"Slave ID en flash: none");
//					break;
//					default:
//						printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
//					break;
//				}
//
//				err = nvs_get_u16(ctrl_mesh,"conver",&(form->conversion));
//				switch(err){
//					case ESP_OK:
//						ESP_LOGI(nvs_tag,"Factor de conversion en flash: %d",form->conversion);
//					break;
//					case ESP_ERR_NVS_NOT_FOUND:
//						ESP_LOGI(nvs_tag,"Factor de conversion en flash: none");
//					break;
//					default:
//						printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
//					break;
//				}
//				break;
//			case(chino):
//				err = nvs_get_u32(ctrl_mesh,"baud",&(form->baud_rate));
//				switch(err){
//					case ESP_OK:
//						ESP_LOGI(nvs_tag,"Baud Rate en flash: %"PRIu32,form->baud_rate);
//					break;
//					case ESP_ERR_NVS_NOT_FOUND:
//						ESP_LOGI(nvs_tag,"Baud Rate en flash: none");
//					break;
//					default:
//						printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
//					break;
//				}
//				break;
//				case(enlace):
//						break;
//				default:
//					break;
//		}
	}
	nvs_close(ctrl_mesh);
}

void get_form_flash_login(form_login *form){

	size_t len;
	esp_err_t err;
	nvs_handle_t ctrl_login;
	char userdef[] = "admin";
	char passdef[] = "admin";

	err = nvs_open("storage",NVS_READONLY,&ctrl_login);
		if (err != ESP_OK) {
			printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
		}else{
			err = nvs_get_str(ctrl_login,"log-user",NULL,&len);
			if(err==ESP_OK) {
				err = nvs_get_str(ctrl_login,"log-user",form->user,&len);
				switch(err){
					case ESP_OK:
						ESP_LOGI(nvs_tag,"user-log en flash: %s",form->user);
					break;
					case ESP_ERR_NVS_NOT_FOUND:
						ESP_LOGI(nvs_tag,"Tomado el usuario por defecto");
						strncpy(form->user,userdef,strlen(userdef));
					break;
					default:
						printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
					break;
				}
			}

			err = nvs_get_str(ctrl_login,"log-pass",NULL,&len);
			if(err==ESP_OK) {
				err = nvs_get_str(ctrl_login,"log-pass",form->password,&len);
				switch(err){
					case ESP_OK:
						ESP_LOGI(nvs_tag,"user-pass en flash: %s",form->password);
					break;
					case ESP_ERR_NVS_NOT_FOUND:
						ESP_LOGI(nvs_tag,"Tomado el password por defecto");
						strncpy(form->password,passdef,strlen(passdef));
					break;
					default:
						printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
					break;
				}
			}
			nvs_close(ctrl_login);
		}
}

void get_form_flash_modbus(form_modbus *form){

	size_t len;
	esp_err_t err;
	nvs_handle_t ctrl_modbus;
	char deftype[] = "enlace";

	err = nvs_open("storage",NVS_READONLY,&ctrl_modbus);
	if (err != ESP_OK) {
		printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
	}else{
		err = nvs_get_str(ctrl_modbus,"mod-tipo",NULL,&len);
		if(err==ESP_OK) {
			err = nvs_get_str(ctrl_modbus,"mod-tipo",form->tipo,&len);
			switch(err){
				case ESP_OK:
					ESP_LOGI(nvs_tag,"Tipo de medidor en flash: %s",form->tipo);
				break;
				case ESP_ERR_NVS_NOT_FOUND:
					ESP_LOGI(nvs_tag,"Tipo por defecto: enlace");
					strncpy(form->tipo,deftype,strlen(deftype));
				break;
				default:
					printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
				break;
				}
			}
		/*Energía inicial*/
		err = nvs_get_u64(ctrl_modbus,"mod-energia",&form->energia);
		switch(err){
			case ESP_OK:
				ESP_LOGI(nvs_tag,"Energia inicial en flash: %llu",form->energia);
			break;
			case ESP_ERR_NVS_NOT_FOUND:
				ESP_LOGI(nvs_tag,"No se encontró energia inicial, asumiendo 0.0 kWh");
				form->energia = 0;
			break;
			default:
				printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
			break;
			}
		/*Slave ID*/
		err = nvs_get_u8(ctrl_modbus,"mod-slaveid",&form->slaveid);
		switch(err){
			case ESP_OK:
				ESP_LOGI(nvs_tag,"Slave ID en flash: %u",form->slaveid);
			break;
			case ESP_ERR_NVS_NOT_FOUND:
				ESP_LOGI(nvs_tag,"No se encontró slave ID, se tomará el valor por defecto 1");
				form->energia = 1;
			break;
			default:
				printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
			break;
		}
		/*Conversion Factor*/
		err = nvs_get_u16(ctrl_modbus,"mod-convfac",&form->conversion);
		switch(err){
			case ESP_OK:
				ESP_LOGI(nvs_tag,"Factor de conv. en flash: %u",form->conversion);
			break;
			case ESP_ERR_NVS_NOT_FOUND:
				ESP_LOGI(nvs_tag,"No se encontró factor de conversion, asumiendo 1200 [imp/kWh]");
				form->energia = 1200;
			break;
			default:
				printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
			break;
			}
		/*Baud rate*/
		err = nvs_get_u32(ctrl_modbus,"mod-brate",&form->baud_rate);
		switch(err){
			case ESP_OK:
				ESP_LOGI(nvs_tag,"Baud rate en flash: %u",form->baud_rate);
			break;
			case ESP_ERR_NVS_NOT_FOUND:
				ESP_LOGI(nvs_tag,"No se encontró baud rate, asumiendo 9600");
				form->energia = 0;
			break;
			default:
				printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
			break;
			}
	nvs_close(ctrl_modbus);
	}
}

void get_form_flash_locwifi(form_locwifi *form){
	size_t len;
	esp_err_t err;
	nvs_handle_t ctrl_locwifi;

	err = nvs_open("storage",NVS_READONLY,&ctrl_locwifi);
		if (err != ESP_OK) {
			printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
		}else{
			/*SSID*/
			err = nvs_get_str(ctrl_locwifi,"wifi-ssid",NULL,&len);
			if(err==ESP_OK) {
				err = nvs_get_str(ctrl_locwifi,"wifi-ssid",form->ssid,&len);
				switch(err){
					case ESP_OK:
						ESP_LOGI(nvs_tag,"SSID en flash: %s",form->ssid);
					break;
					case ESP_ERR_NVS_NOT_FOUND:
						ESP_LOGI(nvs_tag,"No se ha encontrado un SSID en flash");
					break;
					default:
						printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
					break;
				}
			}
			/*Password*/
			err = nvs_get_str(ctrl_locwifi,"wifi-pass",NULL,&len);
			if(err==ESP_OK) {
				err = nvs_get_str(ctrl_locwifi,"wifi-pass",form->password,&len);
				switch(err){
					case ESP_OK:
						ESP_LOGI(nvs_tag,"WiFi Password en flash: %s",form->password);
					break;
					case ESP_ERR_NVS_NOT_FOUND:
					break;
					default:
						printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
					break;
				}
			}
			nvs_close(ctrl_locwifi);
		}


}

void get_form_flash_mqtt(form_mqtt *form){

	size_t len;
	esp_err_t err;
	nvs_handle_t ctrl_mqtt;


	err = nvs_open("storage",NVS_READONLY,&ctrl_mqtt);
		if (err != ESP_OK) {
			printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
		}else{

			/*PORT*/
			err= nvs_get_u16(ctrl_mqtt,"mqtt-port",&form->port);
			switch(err){
				case ESP_OK:
					ESP_LOGI(nvs_tag,"MQTT Port en flash: %u",form->port);
				break;
				case ESP_ERR_NVS_NOT_FOUND:
					ESP_LOGI(nvs_tag,"No se ha encontrado un MQTT port en flash");
				break;
				default:
					printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
				break;
			}

			/*Pub Topic*/
			err = nvs_get_str(ctrl_mqtt,"mqtt-top",NULL,&len);
			if(err==ESP_OK) {
				err = nvs_get_str(ctrl_mqtt,"mqtt-top",form->pubtopic,&len);
				switch(err){
					case ESP_OK:
						ESP_LOGI(nvs_tag,"MQTT PUB. TOPIC en flash: %s",form->pubtopic);
					break;
					case ESP_ERR_NVS_NOT_FOUND:
						ESP_LOGI(nvs_tag,"No se ha encontrado un TOPIC en flash");
					break;
					default:
						printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
					break;}
			}

			/*Advanced configuration 1=True, 0=False*/
			err = nvs_get_u8(ctrl_mqtt,"mqtt-ad",&form->advance);
			switch(err){
				case ESP_OK:
					if(form->advance ==1){ESP_LOGI(nvs_tag, "%s", "Advanced configuration");}
					else{ESP_LOGI(nvs_tag, "%s", "Basic configuration");}
				break;
				case ESP_ERR_NVS_NOT_FOUND:
					ESP_LOGI(nvs_tag,"Advanced config invalid value");
				break;
				default:
					printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
				break;
			}

			if(form->advance!=1){ //Configuración básica

				/*URI*/
				err = nvs_get_str(ctrl_mqtt,"mqtt-uri",NULL,&len);
				if(err==ESP_OK) {
					err = nvs_get_str(ctrl_mqtt,"mqtt-uri",form->uri,&len);
					switch(err){
						case ESP_OK:
							ESP_LOGI(nvs_tag,"MQTT URI en flash: %s",form->uri);
						break;
						case ESP_ERR_NVS_NOT_FOUND:
							ESP_LOGI(nvs_tag,"No se ha encontrado un URI en flash");
						break;
						default:
							printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
						break;}
				}
			}else{ //Configuración avanzada

				/*Tipo de broker público=1 privado=0*/
				err = nvs_get_u8(ctrl_mqtt,"mqtt-type",&form->type);
				switch(err){
					case ESP_OK:
						if(form->type ==1){ESP_LOGI(nvs_tag, "%s", "Public Broker");}
						else{ESP_LOGI(nvs_tag, "%s", "Private Broker");}
					break;
					case ESP_ERR_NVS_NOT_FOUND:
						ESP_LOGI(nvs_tag,"Broker type invalid value");
					break;
					default:
						printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
					break;
				}

				/*App Layer  TCP=1 SSL=2 WS=3 WSS=4*/

				err = nvs_get_u8(ctrl_mqtt,"mqtt-app",&form->app_layer);
				switch(err){
					case ESP_OK:
						switch(form->app_layer){
							case 1:
								ESP_LOGI(nvs_tag,"Application layer TCP");
								break;
							case 2:
								ESP_LOGI(nvs_tag,"Application layer SSL");
								break;
							case 3:
								ESP_LOGI(nvs_tag,"Application layer WS");
								break;
							case 4:
								ESP_LOGI(nvs_tag,"Application layer WSS");
								break;
						}
					break;
					case ESP_ERR_NVS_NOT_FOUND:
						ESP_LOGI(nvs_tag,"Application layer invalid value");
					break;
					default:
						printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
					break;
				}

				/*MQTT Username*/
				err = nvs_get_str(ctrl_mqtt,"mqtt-user",NULL,&len);
				if(err==ESP_OK) {
					err = nvs_get_str(ctrl_mqtt,"mqtt-user",form->user,&len);
					switch(err){
						case ESP_OK:
							ESP_LOGI(nvs_tag,"MQTT Username en flash: %s",form->user);
						break;
						case ESP_ERR_NVS_NOT_FOUND:
							ESP_LOGI(nvs_tag,"No se ha encontrado un User en flash");
						break;
						default:
							printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
						break;}
				}

				if(form->type == 0){ //Broker privado

					/*MQTT Password*/
					err = nvs_get_str(ctrl_mqtt,"mqtt-pass",NULL,&len);
					if(err==ESP_OK) {
						err = nvs_get_str(ctrl_mqtt,"mqtt-pass",form->password,&len);
						switch(err){
							case ESP_OK:
								ESP_LOGI(nvs_tag,"MQTT Password en flash: %s",form->password);
							break;
							case ESP_ERR_NVS_NOT_FOUND:
								ESP_LOGI(nvs_tag,"No se ha encontrado un password en flash");
							break;
							default:
								printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
							break;}
				}
			}
		}
		close(ctrl_mqtt);
		}
}

/********************** PARSE Y VALIDACION DE ENTRADAS (WEB->RAM) **************************/

bool fill_form_mesh(char * p, form_mesh *form){

    cJSON *root = cJSON_Parse(p);
    char *eptr;
    char *sys_info = cJSON_Print(root);
    ESP_LOGI("FILL MESH","%s",sys_info);

    /*Mesh ID*/
    char *aux_meshID = cJSON_GetObjectItem(root, "meshid")->valuestring;
    if(strlen(aux_meshID)!=17){
    	printf("Mesh ID No válida : %s %d", aux_meshID,strlen(aux_meshID));
    	return false;}
    for(int i = 0; i<17;i++){
    	if((i==2||i==5||i==8||i==11||i==14) && aux_meshID[i]!=':'){
    		printf("Mesh ID No válida aux_meshID[%d]=%c",i,aux_meshID[i]);
    		return false;
    	}else if (!((aux_meshID[i]>=0x30 && aux_meshID[i]<=0x39)||(aux_meshID[i]>=0x61||aux_meshID[i]<=0x66))){
    			printf("Mesh ID No válida caractér no válido [%d]", i);
    			return false;
    	}
    }
    for(int i = 0; i<18;i++){
    	if(aux_meshID[i]>0x60){aux_meshID[i]-=0x57;}
    	if(aux_meshID[i+1]>0x60){aux_meshID[i+1]-=0x57;}
    	form->mesh_id[i/3]=((0x0f&aux_meshID[i])<<4)|(0x0f&aux_meshID[i+1]);
    	i+=2;
    }
    ESP_LOGW("FILL MESH","meshID: "MACSTR ,MAC2STR(form->mesh_id));

    /*Mesh access password*/
    char* aux_mesh_password = cJSON_GetObjectItem(root, "meshpass")->valuestring;
    if(strlen(aux_mesh_password)<6||strlen(aux_mesh_password)>15){
    	printf("Contraseña de la red mesh no válida");
    	return false;
    }else{ for(int i = 0; i<=strlen(aux_mesh_password);i++){form->meshappass[i]=aux_mesh_password[i];}}
    ESP_LOGW("FILL MESH","Mesh access password: %s",form->meshappass);

    /*Max Layer*/
    uint8_t aux_max_layer = (uint8_t) strtoul(cJSON_GetObjectItem(root, "maxlayer")->valuestring,&eptr,10);
    if(aux_max_layer>=10 && aux_max_layer<=25){form->max_layer = aux_max_layer;}else{return false;}
    ESP_LOGW("FILL MESH","Layers: %d", form->max_layer);

    /*Max sta*/
    uint8_t aux_max_sta = (uint8_t) strtoul(cJSON_GetObjectItem(root, "maxsta")->valuestring,&eptr,10);
    if(aux_max_sta>=1 && aux_max_sta <=9){form->max_sta = aux_max_sta;}else{return false;}
    ESP_LOGW("FILL MESH","STA: %d", form->max_sta);

    /*Port*/
    uint32_t aux_port = (uint32_t) strtoul(cJSON_GetObjectItem(root, "port")->valuestring,&eptr,10);
    if(aux_port<=65535){form->port = (uint16_t)aux_port;}else{return false;}
    ESP_LOGW("FILL MESH","Port: %d", form->port);

	cJSON_Delete(root);
	return true;
}

bool fill_form_modbus(char *p, form_modbus * form){

    cJSON *root = cJSON_Parse(p);
    char *eptr;
    char *sys_info = cJSON_Print(root);
    ESP_LOGI("FILL MODBUS","%s",sys_info);

    /*Meter type*/
    char* aux_modbus_type = cJSON_GetObjectItem(root, "type")->valuestring;

    if(!strcmp("enlace",aux_modbus_type)){
    	sprintf(form->tipo,"%s","enlace");
    }else if(!strcmp("logo",aux_modbus_type)){
    	sprintf(form->tipo,"%s","chino");
    }else if(!strcmp("pulsos",aux_modbus_type)){
    	sprintf(form->tipo,"%s","pulsos");
    }else if(!strcmp("standard-rs485",aux_modbus_type)){
    	sprintf(form->tipo,"%s","rs485");
    }else{
    	ESP_LOGW("FILL MODBUS","Meter type invalid");
    	return false;
    }
    ESP_LOGW("FILL MODBUS","Modbus meter type: %s",form->tipo);

    /*Baud rate*/
    uint32_t aux_baud_rate = (uint32_t) strtoul(cJSON_GetObjectItem(root, "baudrate")->valuestring,&eptr,10);
    if(aux_baud_rate>=1 && aux_baud_rate<=19200){form->baud_rate = aux_baud_rate;}else{return false;}
    ESP_LOGW("FILL MODBUS","Baud rate: %u", form->baud_rate);

    /*Conversion factor*/
    uint16_t aux_conversion = (uint16_t) strtoul(cJSON_GetObjectItem(root, "convfac")->valuestring,&eptr,10);
    if(aux_conversion>=1){form->conversion = aux_conversion;}else{return false;}
    ESP_LOGW("FILL MODBUS","Conversion Factor: %u", form->conversion);

    /*Energia inicial*/
    uint64_t aux_iniene = (uint64_t) strtoul(cJSON_GetObjectItem(root, "iniene")->valuestring,&eptr,10);
    if(aux_iniene>=0){form->energia = aux_iniene;}else{return false;}
    ESP_LOGW("FILL MODBUS","Energia inicial: %llu", form->energia);

    /*Slave ID*/
    uint8_t aux_slaveid = (uint8_t) strtoul(cJSON_GetObjectItem(root, "slaveid")->valuestring,&eptr,10);
    if(aux_slaveid>=1 && aux_slaveid<=254){form->slaveid = aux_slaveid;}else{return false;}
    ESP_LOGW("FILL MODBUS","Energia inicial: %u", form->slaveid);

	cJSON_Delete(root);
    return true;
}

bool fill_form_locwifi(char*p, form_locwifi * form){

	cJSON *root = cJSON_Parse(p);
    char *sys_info = cJSON_Print(root);
    ESP_LOGI("FILL LOCWIFI","%s",sys_info);

    /*Local SSID*/
    char* aux_ssid = cJSON_GetObjectItem(root, "ssid")->valuestring;
    if(strlen(aux_ssid)<6||strlen(aux_ssid)>15){
    	printf("SSID no válido");
    	return false;
    }else{ for(int i = 0; i<=strlen(aux_ssid);i++){form->ssid[i]=aux_ssid[i];}}
    ESP_LOGW("FILL LOCWIFI","Local WiFi SSID: %s",form->ssid);

    /*Local password*/
    char* aux_pass = cJSON_GetObjectItem(root, "pass")->valuestring;
    if(strlen(aux_pass)<6||strlen(aux_pass)>15){
    	printf("Password no válido");
    	return false;
    }else{ for(int i = 0; i<=strlen(aux_pass);i++){form->password[i]=aux_pass[i];}}
    ESP_LOGW("FILL LOCWIFI","Local WiFi password: %s",form->password);

	cJSON_Delete(root);
	return true;
}

bool fill_form_login(char*p, form_login * form){

	cJSON *root = cJSON_Parse(p);
    char *sys_info = cJSON_Print(root);
    ESP_LOGI("FILL LOGIN","%s",sys_info);

    /*User Login*/
    char* aux_user = cJSON_GetObjectItem(root, "user")->valuestring;
    if(strlen(aux_user)<6||strlen(aux_user)>15){
    	printf("Usuario no válido");
    	return false;
    }else{ for(int i = 0; i<=strlen(aux_user);i++){form->user[i]=aux_user[i];}}
    ESP_LOGW("FILL LOGIN","Local WiFi SSID: %s",form->user);

    /*Password Login*/
    char* aux_pass = cJSON_GetObjectItem(root, "pass")->valuestring;
    if(strlen(aux_pass)<6||strlen(aux_pass)>15){
    	printf("Password no válido");
    	return false;
    }else{ for(int i = 0; i<=strlen(aux_pass);i++){form->password[i]=aux_pass[i];}}
    ESP_LOGW("FILL LOGIN","Local WiFi password: %s",form->password);

	cJSON_Delete(root);
	return true;
}

bool fill_form_mqtt(char*p, form_mqtt * form){

	cJSON *root = cJSON_Parse(p);
    char *eptr;
    char *sys_info = cJSON_Print(root);
    ESP_LOGI("FILL MQTT","%s",sys_info);

    /*Advance*/
    uint8_t aux_advance = (uint8_t) strtoul(cJSON_GetObjectItem(root, "advance")->valuestring,&eptr,10);
    if(aux_advance==1 || aux_advance==0){form->advance = aux_advance;}else{return false;}
    ESP_LOGW("FILL MQTT","%s",(form->advance==1)?"Advanced settings ON":"Advanced settings OFF");

    /*URI*/
    char* aux_uri = cJSON_GetObjectItem(root, "uri")->valuestring;
    if(strlen(aux_uri)>50){
    	printf("URI no válido");
    	return false;
    }else{ for(int i = 0; i<=strlen(aux_uri);i++){form->uri[i]=aux_uri[i];}}
    ESP_LOGW("FILL MQTT","URI: %s",form->uri);

     /*IP*/
     char* aux_ip = cJSON_GetObjectItem(root, "ip")->valuestring;
     if(strlen(aux_ip)>20){
     	printf("IP no válido");
     	return false;
     }else{ for(int i = 0; i<=strlen(aux_ip);i++){form->ip[i]=aux_ip[i];}}
     ESP_LOGW("FILL MQTT","IP: %s",form->ip);

     /*Port*/
      uint16_t aux_port = (uint16_t) strtoul(cJSON_GetObjectItem(root, "port")->valuestring,&eptr,10);
      if(aux_port>0){form->port = aux_port;}else{return false;}
      ESP_LOGW("FILL MQTT","MQTT Port: %u",form->port);

      /*Publication topic*/
      char* aux_pubtopic = cJSON_GetObjectItem(root, "topic")->valuestring;
      if(strlen(aux_pubtopic)>20){
      	printf("IP no válido");
      	return false;
      }else{ for(int i = 0; i<=strlen(aux_pubtopic);i++){form->pubtopic[i]=aux_pubtopic[i];}}
      ESP_LOGW("FILL MQTT","Publication topic: %s",form->pubtopic);

      /*Type*/
       uint8_t aux_type = (uint8_t) strtoul(cJSON_GetObjectItem(root, "type")->valuestring,&eptr,10);
       if(aux_type==1 || aux_type==0){form->type = aux_type;}else{return false;}
       ESP_LOGW("FILL MQTT","%s",(form->type==1)?"Type of broker: Public":"Type of broker: Private");

       /*User*/
       char* aux_user = cJSON_GetObjectItem(root, "user")->valuestring;
       if(strlen(aux_user)>20){
       	printf("User no válido");
       	return false;
       }else{ for(int i = 0; i<=strlen(aux_user);i++){form->user[i]=aux_user[i];}}
       ESP_LOGW("FILL MQTT","MQTT User: %s",form->user);

       /*User*/
       char* aux_password = cJSON_GetObjectItem(root, "password")->valuestring;
       if(strlen(aux_password)>20){
       	printf("Password no válido");
       	return false;
       }else{ for(int i = 0; i<=strlen(aux_password);i++){form->password[i]=aux_password[i];}}
       ESP_LOGW("FILL MQTT","MQTT Password: %s",form->password);

       /*App Layer*/
       uint8_t aux_applayer = (uint8_t) strtoul(cJSON_GetObjectItem(root, "app_layer")->valuestring,&eptr,10);
       if(aux_applayer==1 || aux_applayer==2 || aux_applayer==3 || aux_applayer==4){form->app_layer = aux_applayer;}else{return false;}
       ESP_LOGW("FILL MQTT","%s",(form->advance==1)?"App Layer: TCP":(form->advance==2)?"App Layer: SSL":(form->advance==3)?"App Layer: WS":"App Layer: WSS");

   	cJSON_Delete(root);
	return true;
}

/*********************** SOCKET HTTP *******************************************************/

/* Set HTTP response content type according to file extension */
static esp_err_t set_content_type_from_file(httpd_req_t *req, const char *filepath)
{
    const char *type = "text/plain";
    if (CHECK_FILE_EXTENSION(filepath, ".html")) {
        type = "text/html";
    }else if (CHECK_FILE_EXTENSION(filepath, ".js")) {
        type = "application/javascript";
    }else if (CHECK_FILE_EXTENSION(filepath, ".css")) {
        type = "text/css";
    } else if (CHECK_FILE_EXTENSION(filepath, ".png")) {
        type = "image/png";
    } else if (CHECK_FILE_EXTENSION(filepath, ".ico")) {
        type = "image/x-icon";
    } else if (CHECK_FILE_EXTENSION(filepath, ".svg")) {
        type = "text/xml";
    }
    return httpd_resp_set_type(req, type);
}

/************************ CALLBACK FUNCTIONS HANDLERS ***************************************************/

/* Send HTTP response with the contents of the requested file */
static esp_err_t rest_common_get_handler(httpd_req_t *req)
{
    char filepath[FILE_PATH_MAX];

    ESP_LOGI(TAG,"req->uri: %s",req->uri);
    bool pasa = true;

    char foobar[7] = "/foobar";
    for(int i = 0; i < sizeof(foobar);i++){
    	if(foobar[i]!=req->uri[i]){
    		pasa=false;
    	}
    }

    rest_server_context_t *rest_context = (rest_server_context_t *)req->user_ctx;
    strlcpy(filepath, rest_context->base_path, sizeof(filepath));

    if (req->uri[strlen(req->uri) - 1] == '/' || pasa) {
        strlcat(filepath, "/login.html", sizeof(filepath));
    } else {
        strlcat(filepath, req->uri, sizeof(filepath));
    }

    int fd = open(filepath, O_RDONLY, 0);
    if (fd == -1) {
        ESP_LOGE(REST_TAG, "Failed to open file : %s", filepath);
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to read existing file");
        return ESP_FAIL;
    }

    set_content_type_from_file(req, filepath);

    char *chunk = rest_context->scratch;
    ssize_t read_bytes;
    do {
        /* Read file in chunks into the scratch buffer */
        read_bytes = read(fd, chunk, SCRATCH_BUFSIZE);
        if (read_bytes == -1) {
            ESP_LOGE(REST_TAG, "Failed to read file : %s", filepath);
        } else if (read_bytes > 0) {
            /* Send the buffer contents as HTTP response chunk */
            if (httpd_resp_send_chunk(req, chunk, read_bytes) != ESP_OK) {
                close(fd);
                ESP_LOGE(REST_TAG, "File sending failed!");
                /* Abort sending file */
                httpd_resp_sendstr_chunk(req, NULL);
                /* Respond with 500 Internal Server Error */
                httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to send file");
                return ESP_FAIL;
            }
        }
    } while (read_bytes > 0);
    /* Close file after sending complete */
    close(fd);
    ESP_LOGI(REST_TAG, "File sending complete");
    /* Respond with an empty chunk to signal HTTP response completion */
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

static esp_err_t login_post_handler(httpd_req_t *req){/*Revisa que el usuario y contraseña del login sea el correcto*/
    int total_len = req->content_len;
    int cur_len = 0;
    char *buf = ((rest_server_context_t *)(req->user_ctx))->scratch;
    int received = 0;
    if (total_len >= SCRATCH_BUFSIZE) {
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "content too long");
        return ESP_FAIL;
    }
    while (cur_len < total_len) {
        received = httpd_req_recv(req, buf + cur_len, total_len);
        if (received <= 0) {
            /* Respond with 500 Internal Server Error */
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to post control value");
            return ESP_FAIL;
        }
        cur_len += received;
    }
    buf[total_len] = '\0';

    ESP_LOGI("DEBUG","BUFFER: %s",buf);

    cJSON *root = cJSON_Parse(buf);

    char* aux_usuario = cJSON_GetObjectItem(root,"usuario")->valuestring;
    char* aux_contrasena = cJSON_GetObjectItem(root,"contrasena")->valuestring;
    if( (strlen(aux_usuario)==strlen(valid_user))&&(strlen(aux_contrasena)==strlen(valid_pass))){
    	if((strcmp(aux_usuario, valid_user)==0)&&(strcmp(aux_contrasena, valid_pass)==0)){
			httpd_resp_sendstr(req, "valido");
		}else{
			httpd_resp_sendstr(req, "content no valido");
		}
    }else{
    	httpd_resp_sendstr(req, "long no valido");
    }
	cJSON_Delete(root);

    return ESP_OK;
}

static esp_err_t form_mesh_get_handler(httpd_req_t *req){/*Toma los datos de la mesh en flash y los sube al formulario mesh*/

	ESP_LOGI(TAG,"req->uri: %s",req->uri);

	get_form_flash_mesh(&fweb_mesh_config);
	httpd_resp_set_status(req, HTTPD_200);
	httpd_resp_set_type(req, HTTPD_TYPE_JSON);

	 char mesh_id [18] = {0,};
	 sprintf(mesh_id,MACSTR,MAC2STR(fweb_mesh_config.mesh_id));

	 cJSON *root = cJSON_CreateObject();
	 cJSON_AddStringToObject(root, "meshid", mesh_id);
	 cJSON_AddStringToObject(root, "meshpass", fweb_mesh_config.meshappass);
	 cJSON_AddNumberToObject(root, "maxlayer", fweb_mesh_config.max_layer);
	 cJSON_AddNumberToObject(root, "maxsta", fweb_mesh_config.max_sta);
	 cJSON_AddNumberToObject(root, "port", fweb_mesh_config.port);
	 const char *sys_info = cJSON_Print(root);
	 cJSON_Delete(root);
	 ESP_LOGI("JSON MESH","%s",sys_info);
	 ESP_LOGI("DEBUG","%d",strlen(sys_info));

	 httpd_resp_send(req, sys_info, strlen(sys_info));

	return ESP_OK;
}

static esp_err_t form_mesh_post_handler(httpd_req_t *req){/*Recepción de formulario de configuración mesh*/

    int total_len = req->content_len;
    int cur_len = 0;
    char *buf = ((rest_server_context_t *)(req->user_ctx))->scratch;
    int received = 0;
    if (total_len >= SCRATCH_BUFSIZE) {
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "content too long");
        return ESP_FAIL;
    }
    while (cur_len < total_len) {
        received = httpd_req_recv(req, buf + cur_len, total_len);
        if (received <= 0) {
            /* Respond with 500 Internal Server Error */
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to post control value");
            return ESP_FAIL;
        }
        cur_len += received;
    }
    buf[total_len] = '\0';

    ESP_LOGI("DEBUG","BUFFER: %s",buf);
    if(fill_form_mesh(buf, &fweb_mesh_config)){
    	set_form_flash_mesh(fweb_mesh_config);
    	httpd_resp_sendstr(req, "safe");
    	return ESP_OK;
    }else{
    	httpd_resp_sendstr(req, "Error");
    	return ESP_FAIL;
    }

}

static esp_err_t form_modbus_get_handler(httpd_req_t *req){

	ESP_LOGI(TAG,"req->uri: %s",req->uri);

	get_form_flash_modbus(&fweb_modbus);
	httpd_resp_set_status(req, HTTPD_200);
	httpd_resp_set_type(req, HTTPD_TYPE_JSON);

	 cJSON *root = cJSON_CreateObject();
	 cJSON_AddStringToObject(root, "type", fweb_modbus.tipo);
	 cJSON_AddNumberToObject(root, "baudrate", fweb_modbus.baud_rate);
	 cJSON_AddNumberToObject(root, "convfac", fweb_modbus.conversion);
	 cJSON_AddNumberToObject(root, "iniene", fweb_modbus.energia);
	 cJSON_AddNumberToObject(root, "slaveid", fweb_modbus.slaveid);
	 const char *sys_info = cJSON_Print(root);
	 cJSON_Delete(root);
	 ESP_LOGI("JSON MODBUS","%s %d",sys_info, strlen(sys_info));

	 httpd_resp_send(req, sys_info, strlen(sys_info));

	return ESP_OK;
}

static esp_err_t form_modbus_post_handler(httpd_req_t *req){

    int total_len = req->content_len;
    int cur_len = 0;
    char *buf = ((rest_server_context_t *)(req->user_ctx))->scratch;
    int received = 0;
    if (total_len >= SCRATCH_BUFSIZE) {
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "content too long");
        return ESP_FAIL;
    }
    while (cur_len < total_len) {
        received = httpd_req_recv(req, buf + cur_len, total_len);
        if (received <= 0) {
            /* Respond with 500 Internal Server Error */
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to post control value");
            return ESP_FAIL;
        }
        cur_len += received;
    }
    buf[total_len] = '\0';

    ESP_LOGI("DEBUG","BUFFER: %s",buf);
    if(fill_form_modbus(buf, &fweb_modbus)){
    	set_form_flash_modbus(fweb_modbus);
    	httpd_resp_sendstr(req, "safe");
    	return ESP_OK;
    }else{
    	httpd_resp_sendstr(req, "Error");
    	return ESP_FAIL;
    }
}

static esp_err_t form_locwifi_get_handler(httpd_req_t *req){

	ESP_LOGI(TAG,"req->uri: %s",req->uri);

	get_form_flash_locwifi(&fweb_locwifi);
	httpd_resp_set_status(req, HTTPD_200);
	httpd_resp_set_type(req, HTTPD_TYPE_JSON);

	 cJSON *root = cJSON_CreateObject();
	 cJSON_AddStringToObject(root, "ssid", fweb_locwifi.ssid);
	 cJSON_AddStringToObject(root, "pass", fweb_locwifi.password);
	 const char *sys_info = cJSON_Print(root);
	 cJSON_Delete(root);
	 ESP_LOGI("JSON WIFILOC","%s %d",sys_info, strlen(sys_info));

	 httpd_resp_send(req, sys_info, strlen(sys_info));

	return ESP_OK;
}

static esp_err_t form_locwifi_post_handler(httpd_req_t *req){

	int total_len = req->content_len;
    int cur_len = 0;
    char *buf = ((rest_server_context_t *)(req->user_ctx))->scratch;
    int received = 0;
    if (total_len >= SCRATCH_BUFSIZE) {
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "content too long");
        return ESP_FAIL;
    }
    while (cur_len < total_len) {
        received = httpd_req_recv(req, buf + cur_len, total_len);
        if (received <= 0) {
            /* Respond with 500 Internal Server Error */
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to post control value");
            return ESP_FAIL;
        }
        cur_len += received;
    }
    buf[total_len] = '\0';

    ESP_LOGI("DEBUG","BUFFER: %s",buf);
    if(fill_form_locwifi(buf, &fweb_locwifi)){
    	set_form_flash_locwifi(fweb_locwifi);
    	httpd_resp_sendstr(req, "safe");
    	return ESP_OK;
    }else{
    	httpd_resp_sendstr(req, "Error");
    	return ESP_FAIL;
    }
}

static esp_err_t form_userlog_get_handler(httpd_req_t *req){

	ESP_LOGI(TAG,"req->uri: %s",req->uri);

	get_form_flash_login(&fweb_login);
	httpd_resp_set_status(req, HTTPD_200);
	httpd_resp_set_type(req, HTTPD_TYPE_JSON);

	 cJSON *root = cJSON_CreateObject();
	 cJSON_AddStringToObject(root, "user", fweb_login.user);
	 cJSON_AddStringToObject(root, "pass", fweb_login.password);
	 const char *sys_info = cJSON_Print(root);
	 cJSON_Delete(root);
	 ESP_LOGI("JSON USERLOG","%s %d",sys_info, strlen(sys_info));

	 httpd_resp_send(req, sys_info, strlen(sys_info));

	return ESP_OK;
}

static esp_err_t form_userlog_post_handler(httpd_req_t *req){

	int total_len = req->content_len;
    int cur_len = 0;
    char *buf = ((rest_server_context_t *)(req->user_ctx))->scratch;
    int received = 0;
    if (total_len >= SCRATCH_BUFSIZE) {
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "content too long");
        return ESP_FAIL;
    }
    while (cur_len < total_len) {
        received = httpd_req_recv(req, buf + cur_len, total_len);
        if (received <= 0) {
            /* Respond with 500 Internal Server Error */
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to post control value");
            return ESP_FAIL;
        }
        cur_len += received;
    }
    buf[total_len] = '\0';

    ESP_LOGI("DEBUG","BUFFER: %s",buf);
    if(fill_form_login(buf, &fweb_login)){
    	set_form_flash_login(fweb_login);
    	httpd_resp_sendstr(req, "safe");
    	return ESP_OK;
    }else{
    	httpd_resp_sendstr(req, "Error");
    	return ESP_FAIL;
    }
}

static esp_err_t form_mqtt_get_handler(httpd_req_t *req){

	ESP_LOGI(TAG,"req->uri: %s",req->uri);

	get_form_flash_mqtt(&fweb_mqtt);
	httpd_resp_set_status(req, HTTPD_200);
	httpd_resp_set_type(req, HTTPD_TYPE_JSON);

	 cJSON *root = cJSON_CreateObject();
	 cJSON_AddNumberToObject(root, "advance", fweb_mqtt.advance);
	 cJSON_AddNumberToObject(root, "app_layer", fweb_mqtt.app_layer);
	 cJSON_AddStringToObject(root, "ip", fweb_mqtt.ip);
	 cJSON_AddStringToObject(root, "password", fweb_mqtt.password);
	 cJSON_AddNumberToObject(root, "port", fweb_mqtt.port);
	 cJSON_AddStringToObject(root, "topic", fweb_mqtt.pubtopic);
	 cJSON_AddNumberToObject(root, "type", fweb_mqtt.type);
	 cJSON_AddStringToObject(root, "uri", fweb_mqtt.uri);
	 cJSON_AddStringToObject(root, "user", fweb_mqtt.user);
	 const char *sys_info = cJSON_Print(root);
	 cJSON_Delete(root);
	 ESP_LOGI("JSON MODBUS","%s %d",sys_info, strlen(sys_info));

	 httpd_resp_send(req, sys_info, strlen(sys_info));

	return ESP_OK;
}

static esp_err_t form_mqtt_post_handler(httpd_req_t *req){

	int total_len = req->content_len;
    int cur_len = 0;
    char *buf = ((rest_server_context_t *)(req->user_ctx))->scratch;
    int received = 0;
    if (total_len >= SCRATCH_BUFSIZE) {
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "content too long");
        return ESP_FAIL;
    }
    while (cur_len < total_len) {
        received = httpd_req_recv(req, buf + cur_len, total_len);
        if (received <= 0) {
            /* Respond with 500 Internal Server Error */
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to post control value");
            return ESP_FAIL;
        }
        cur_len += received;
    }
    buf[total_len] = '\0';

    ESP_LOGI("DEBUG","BUFFER: %s",buf);
    if(fill_form_mqtt(buf, &fweb_mqtt)){
    	set_form_flash_mqtt(fweb_mqtt);
    	httpd_resp_sendstr(req, "safe");
    	return ESP_OK;
    }else{
    	httpd_resp_sendstr(req, "Error");
    	return ESP_FAIL;
    }
}

/************************* INICIO DEL SERVIDOR Y MANEJADORES DE PETICIONES *****************************/

esp_err_t start_rest_server(const char *base_path)
{
	/* Validate file storage base path */
	if (!base_path || strcmp(base_path, CONFIG_EXAMPLE_WEB_MOUNT_POINT) != 0) {
		ESP_LOGE(REST_TAG, "File server presently supports only '/spiffs' as base path");
		return ESP_ERR_INVALID_ARG;
	}

    rest_server_context_t *rest_context = calloc(1, sizeof(rest_server_context_t));
    REST_CHECK(rest_context, "No memory for rest context", err);

    strlcpy(rest_context->base_path, base_path, sizeof(rest_context->base_path));

    httpd_handle_t server = NULL;
    httpd_config_t config = {
    		.task_priority      = tskIDLE_PRIORITY+5,       \
			.stack_size         = 1024*25,                  \
			.core_id            = tskNO_AFFINITY,           \
			.server_port        = 80,                       \
			.ctrl_port          = 32768,                    \
			.max_open_sockets   = 7,                        \
			.max_uri_handlers   = 15,                       \
			.max_resp_headers   = 15,                       \
			.backlog_conn       = 5,                        \
			.lru_purge_enable   = false,                    \
			.recv_wait_timeout  = 5,                        \
			.send_wait_timeout  = 5,                        \
			.global_user_ctx = NULL,                        \
			.global_user_ctx_free_fn = NULL,                \
			.global_transport_ctx = NULL,                   \
			.global_transport_ctx_free_fn = NULL,           \
			.open_fn = NULL,                                \
			.close_fn = NULL,                               \
			.uri_match_fn = NULL                            \

    };
    config.uri_match_fn = httpd_uri_match_wildcard;

    ESP_LOGI(REST_TAG, "Starting HTTP Server");
    REST_CHECK(httpd_start(&server, &config) == ESP_OK, "Start server failed", err_start);


	/* URI handler for Login POST*/
	httpd_uri_t login_uri = {
		.uri = "/login",
		.method = HTTP_POST,
		.handler = login_post_handler,
		.user_ctx = rest_context
	};
	httpd_register_uri_handler(server, &login_uri);

    /* URI handler for form mesh GET */
	httpd_uri_t mesh_get_uri = {
		.uri = "/mesh",
		.method = HTTP_GET,
		.handler = form_mesh_get_handler,
		.user_ctx = rest_context
	};
	httpd_register_uri_handler(server, &mesh_get_uri);

	/* URI handler for form mesh POST*/
	httpd_uri_t mesh_post_uri = {
		.uri = "/mesh",
		.method = HTTP_POST,
		.handler = form_mesh_post_handler,
		.user_ctx = rest_context
	};
	httpd_register_uri_handler(server, &mesh_post_uri);

	/* URI handler for form modbus GET*/
	httpd_uri_t modbus_get_uri = {
		.uri = "/modbus",
		.method = HTTP_GET,
		.handler = form_modbus_get_handler,
		.user_ctx = rest_context
	};
	httpd_register_uri_handler(server, &modbus_get_uri);

	/* URI handler for form modbus POST*/
	httpd_uri_t modbus_post_uri = {
		.uri = "/modbus",
		.method = HTTP_POST,
		.handler = form_modbus_post_handler,
		.user_ctx = rest_context
	};
	httpd_register_uri_handler(server, &modbus_post_uri);

	/* URI handler for form locwifi GET*/
	httpd_uri_t locwifi_get_uri = {
		.uri = "/locwifi",
		.method = HTTP_GET,
		.handler = form_locwifi_get_handler,
		.user_ctx = rest_context
	};
	httpd_register_uri_handler(server, &locwifi_get_uri);

	/* URI handler for form locwifi POST*/
	httpd_uri_t locwifi_post_uri = {
		.uri = "/locwifi",
		.method = HTTP_POST,
		.handler = form_locwifi_post_handler,
		.user_ctx = rest_context
	};
	httpd_register_uri_handler(server, &locwifi_post_uri);

	/* URI handler for form userlog GET*/
	httpd_uri_t userlog_get_uri = {
		.uri = "/userlog",
		.method = HTTP_GET,
		.handler = form_userlog_get_handler,
		.user_ctx = rest_context
	};
	httpd_register_uri_handler(server, &userlog_get_uri);

	/* URI handler for form userlog POST*/
	httpd_uri_t userlog_post_uri = {
		.uri = "/userlog",
		.method = HTTP_POST,
		.handler = form_userlog_post_handler,
		.user_ctx = rest_context
	};
	httpd_register_uri_handler(server, &userlog_post_uri);
	/* URI handler for form MQTT GET*/
	httpd_uri_t mqtt_get_uri = {
		.uri = "/mqtt",
		.method = HTTP_GET,
		.handler = form_mqtt_get_handler,
		.user_ctx = rest_context
	};
	httpd_register_uri_handler(server, &mqtt_get_uri);

	/* URI handler for form mqtt POST*/
	httpd_uri_t mqtt_post_uri = {
		.uri = "/mqtt",
		.method = HTTP_POST,
		.handler = form_mqtt_post_handler,
		.user_ctx = rest_context
	};
	httpd_register_uri_handler(server, &mqtt_post_uri);

    /* URI handler for getting web server files */
    httpd_uri_t common_get_uri = {
        .uri = "/*",
        .method = HTTP_GET,
        .handler = rest_common_get_handler,
        .user_ctx = rest_context
    };
    httpd_register_uri_handler(server, &common_get_uri);


    return ESP_OK;
err_start:
    free(rest_context);
err:
    return ESP_FAIL;
}

// Tarea RTOS para el servidor --------------------------------------------------//
void ServidorHTTP(){
	esp_wifi_restore();
	iniciar_wifi();
	initialise_mdns();
	ESP_ERROR_CHECK(init_fs());
    ESP_ERROR_CHECK(start_rest_server(CONFIG_EXAMPLE_WEB_MOUNT_POINT));
}
/*********************************************************************************/
