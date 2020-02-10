/*********************************************************************************/
#include "include/webHTTP.h"
const char *nvs_tag = "NVS";
const char *http_server = "HTTP SERVER";

// Variables constantes que definen la pagina web -------------------------------//
const static char respuestaHTTP[] = "HTTP/1.1 200 OK\r\nContent-type:"
		"text/html\r\n\r\n";

const static char abreHTML[] = "<!DOCTYPE html><html lang=\"es\">";

const static char cabeceraHTML[] = "<head>"
        "<title>Control</title>"
		"<meta charset=\"UTF-8\">"
        "<style>"
            ".vertical-menu a {"
            "background-color: #eee;" /* Grey background color */
            "color: black;" /* Black text color */
            "display: block;" /* Make the links appear below each other */
            "padding: 12px;" /* Add some padding */
            "text-decoration: none;" /* Remove underline from links */
            "}"

            ".vertical-menu a:hover {"
            "background-color: #ccc;" /* Dark grey background on mouse-over */
            "}"

            ".vertical-menu a.active {"
            "background-color: blue;" /* Add a green color to the "active/current" link */
            "color: white;"
            "}"

            ".opcion{"
                    "margin-bottom: 5%;"
            "}"
        "</style>"
		"</head>";

const static char cuerpoALERTAok[] ="<script>"
				"alert('Data stored successfully')"
				"</script>";

const static char cuerpoALERTApaga[] ="<script>"
				"alert('Please Call Estelio in order to activate this section')"
				"</script>";

const static char cuerpoALERTAnok[] ="<script>"
				"alert('Data corrupted')"
				"</script>";

const static char cuerpoALERTAintro[] ="<script>"
				"alert('User or Password Incorrect)"
				"</script>";
const static char cuerpoReinicio[] ="<script>"
				"alert('ESP32 Mesh Start')"
				"</script>";

const static char cuerpoHTML[] = "<body id=\"todo\">"
        "<div id=\"formInput\" style=\"margin: 0 auto;margin-top:20%;display:inline-block;text-align: center;display: block;padding: 2%;width: 25%;border-radius: 2% 2% 2% 2%; border: 5px ridge #000000; background-color: white;\">"
                "<form name=\"entrada\">"
                    "<label id=\"usuario\">User:</label><br>"
                    "<input type=\"text\" name=\"usuario\"><br>"
                    "<label>Password</label><br>"
                    "<input type=\"password\" name=\"contrasena\"><br>"
                    "<input type=\"submit\" name=\"enter\" value=\"Submit\"style=\"margin:10px;\">"
                    "<input type=\"button\" name=\"enter\" value=\"Cancel\"style=\"margin:10px;\">"
                "</form>"
                "</div>"
		"</body>";
const static char cuerpoHTML_INI[] =
		"<body id=\"todo\">"
        "<div id=\"pagina\" style=\" display: block;height:100vh;padding=1%;background-color:blue; background-image: linear-gradient(to bottom right, white,blue,white);width: 100%;\">"
            "<aside id=\"menu\" style=\"width:20%;float:left; background-color: #eee;\">"
                    "<div class=\"vertical-menu\" style=\"height: 100vh;\">"
                            "<a href=\"mesh\" class=\"active\">Mesh Configuration</a>"
                            "<a href=\"modbus\">MODBUS Parameters</a>"
                            "<a href=\"mqtt\">MQTT Parameters</a>"
                            "<a href=\"about\">About us</a>"
                        "</div>"
            "</aside>"
            "<div id=\"FormHome\" style=\"padding=10%;position:relative;left: 10%;top: 5%;margin: 0 auto;display:inline-block;text-align: center;display: block;padding: 2%;width: 25%;border-radius: 2% 2% 2% 2%; border: 5px ridge #000000; background-color: white;\">"
                    "<label style=\"font-weight: bold;\">ESP32 Node Configuration</label>"
                    "<form name=\"FormConfig\" style=\"margin: 10%;id=\"init\"\">"
                        "<label id=\"SSID\" >SSID:</label><br>"
                        "<input class=\"opcion\"type=\"text\" name=\"ssid\" required><br>"
                        "<label >Password:</label><br>"
                        "<input class=\"opcion\"type=\"password\" name=\"contrasena\" required><br>"
						"<label >Mesh ID:</label><br>"
                        "<input class=\"opcion\"type=\"text\" name=\"meshID\" pattern=\"^([0-9a-f][0-9a-f]:){5}([0-9a-f][0-9a-f])$\" placeholder=\"77:77:77:77:77:77\" required><br>"
						"<label >Mesh AP Password:</label><br>"
						"<input class=\"opcion\"type=\"password\" name=\"meshAPpass\" required><br>"
						"<label >Max. Layers:</label><br>"
                        "<input class=\"opcion\"type=\"number\" name=\"layers\" min=\"10\" max=\"25\"placeholder=\"Between 10 - 25\"required><br>"
                        "<label >Max. STA:</label><br>"
                        "<input class=\"opcion\"type=\"number\" name=\"estaciones\" min=\"1\" max=\"9\"placeholder=\"Between 1 - 9\"required><br>"
                        "<label >Port (Socket):</label><br>"
                        "<input class=\"opcion\"type=\"number\" name=\"port\" min=\"1\" max=\"65535\" placeholder=\"Between 0 - 65536\"><br>"

						"<input class=\"opcion\"type=\"submit\" name=\"submit\" value=\"Submit\"style=\"margin:10px;\">"
                        "<input class=\"opcion\"type=\"button\" name=\"enter\" value=\"Cancel\"style=\"margin:10px;\">"

                    	"</form>"
					"<form name=\"Reset ESP\">"
						"<input type=\"submit\" id=\"restart\" style=\"padding: 10%;\" name=\"reinicio\" value=\"Start Mesh\"runat=\"server\"formmethod=\"post\">"
					"</form>"
            "</div>"
        "</div>"
    "</body>";

const static char cuerpoHTML_MODBUS[] =
		"<body id=\"todo\">"
        "<div id=\"pagina\" style=\" display: block;height:100vh;padding=1%;background-color:blue; background-image: linear-gradient(to bottom right, white,blue,white);width: 100%;\">"
            "<aside id=\"menu\" style=\"width:20%;float:left; background-color: #eee;\">"
                    "<div class=\"vertical-menu\" style=\"height: 100vh;\">"
                            "<a href=\"mesh\" >Mesh Configuration</a>"
                            "<a href=\"modbus\" class=\"active\">MODBUS Parameters</a>"
                            "<a href=\"mqtt\">MQTT Parameters</a>"
                            "<a href=\"about\">About us</a>"
                        "</div>"
            "</aside>"
            "<div id=\"FormHome\" style=\"padding=10%;position:relative;left: 10%;top: 5%;margin: 0 auto;display:inline-block;text-align: center;display: block;padding: 2%;width: 25%;border-radius: 2% 2% 2% 2%; border: 5px ridge #000000; background-color: white;\">"
                    "<label style=\"font-weight: bold;\">MODBUS Configuration</label>"
                    "<form name=\"FormMODBUS\" style=\"margin: 10%;id=\"init\"\">"
						"<label >Conversion factor[imp/kWh]:</label><br>"
						"<input class=\"opcion\"type=\"number\" name=\"conversion\" placeholder=\"imp/kWh\"><br>"
						"<label >Initial Energy [kWh]:</label><br>"
						"<input class=\"opcion\"type=\"number\" name=\"energia\" placeholder=\"Initial kWh\"><br>"
						"<select name = \"modoOutput\">"
						"<option value = \"rs485\">Standard RS485 Output</option>"
						"<option value = \"pulsos\">Pulse Output</option>"
						"<option value = \"chino\">LogoMeter RS485 Output</option>"
						"<option value = \"enlace\">Link node</option>"
						"</select><br>"
						"<label >Slave ID:</label><br>"
						"<input class=\"opcion\"type=\"number\" name=\"slaveid\" placeholder=\"Id for pulse meter\"><br>"
						"<input class=\"opcion\"type=\"submit\" name=\"submit\" value=\"Submit\"style=\"margin:10px;\">"
                        "<input class=\"opcion\"type=\"button\" name=\"enter\" value=\"Cancel\"style=\"margin:10px;\">"

                    	"</form>"
					"<form name=\"Reset ESP\">"
						"<input type=\"submit\" id=\"restart\" style=\"padding: 10%;\" name=\"reinicio\" value=\"Start Mesh\"runat=\"server\"formmethod=\"post\">"
					"</form>"
            "</div>"
        "</div>"
    "</body>";

const static char cierraHTML[] = "</html>";

void set_form_flash_init(struct form_home form){
	esp_err_t err;
	nvs_handle_t ctrl_flash;
	err = nvs_open("storage",NVS_READWRITE,&ctrl_flash);
	if (err != ESP_OK) {
		printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
	}else{
		nvs_set_str(ctrl_flash,"ssid",form.ssid);

		nvs_set_str(ctrl_flash,"password",form.password);

		nvs_set_blob(ctrl_flash,"meshid",form.mesh_id,sizeof(form.mesh_id));

		nvs_set_str(ctrl_flash,"meshpass",form.meshappass);

		nvs_set_u8(ctrl_flash,"max_layer",form.max_layer);

		nvs_set_u8(ctrl_flash,"max_sta",form.max_sta);

		nvs_set_u16(ctrl_flash,"port",form.port);

		err = nvs_commit(ctrl_flash);
	}

	nvs_close(ctrl_flash);
}

void set_form_flash_modbus(struct form_home form){
	esp_err_t err;
	nvs_handle_t ctrl_flash;
	err = nvs_open("storage",NVS_READWRITE,&ctrl_flash);
	if (err != ESP_OK) {
		printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
	}else{
		nvs_set_u64(ctrl_flash,"energy",form.energia);

		nvs_set_str(ctrl_flash,"tipo",form.tipo);

		nvs_set_u8(ctrl_flash,"slaveid",form.slaveid);

		nvs_set_u16(ctrl_flash,"conver",form.conversion);
		err = nvs_commit(ctrl_flash);
	}
	nvs_close(ctrl_flash);
}

void get_form_flash(struct form_home *form){
	size_t len;
	char mac[17];
	esp_err_t err;
	nvs_handle_t ctrl_flash, ctrl_prueba;
	err = nvs_open("storage",NVS_READWRITE,&ctrl_flash);
	if (err != ESP_OK) {
		printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
	}else{
		err = nvs_get_str(ctrl_flash,"ssid",NULL,&len);
		if(err==ESP_OK) {
			err = nvs_get_str(ctrl_flash,"ssid",form->ssid,&len);
			switch(err){
				case ESP_OK:
					ESP_LOGI(nvs_tag,"SSID en flash: %s",form->ssid);
				break;
				case ESP_ERR_NVS_NOT_FOUND:
					ESP_LOGI(nvs_tag,"SSID en flash: none");
				break;
				default:
					printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
				break;
			}
		}

		err = nvs_get_str(ctrl_flash,"password",NULL,&len);
		if(err==ESP_OK){
			err= nvs_get_str(ctrl_flash,"password",form->password,&len);
		switch(err){
			case ESP_OK:
				ESP_LOGI(nvs_tag,"Password en flash: %s",form->password);
			break;
			case ESP_ERR_NVS_NOT_FOUND:
				ESP_LOGI(nvs_tag,"Password en flash: none");
			break;
			default:
				printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
			break;
		}
		}
		err = nvs_get_blob(ctrl_flash,"meshid",NULL,&len);
		if(err==ESP_OK){
		err = nvs_get_blob(ctrl_flash,"meshid",form->mesh_id,&len);
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
			break;
		}
		}
	}nvs_close(ctrl_flash);

	err = nvs_open("storage",NVS_READWRITE,&ctrl_prueba);

		if (err != ESP_OK) {
			printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
		}else{
			err = nvs_get_str(ctrl_prueba,"meshpass",NULL,&len);
			if(err==ESP_OK){
				err= nvs_get_str(ctrl_prueba,"meshpass",form->meshappass,&len);
			switch(err){
				case ESP_OK:
					ESP_LOGI(nvs_tag,"Mesh password en flash: %s",form->meshappass);
				break;
				case ESP_ERR_NVS_NOT_FOUND:
					ESP_LOGI(nvs_tag,"Mesh password en flash: none");
				break;
				default:
					printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
				break;
			}
			}
			nvs_get_u8(ctrl_prueba,"max_layer",&(form->max_layer));
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
			nvs_get_u8(ctrl_prueba,"max_sta",&(form->max_sta));
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
			nvs_get_u16(ctrl_prueba,"port",&(form->port));
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
			nvs_get_u64(ctrl_prueba,"energy",&(form->energia));
			switch(err){
				case ESP_OK:
					ESP_LOGI(nvs_tag,"Medicion de energia en flash: %"PRIu64,form->energia);
				break;
				case ESP_ERR_NVS_NOT_FOUND:
					ESP_LOGI(nvs_tag,"Medicion de energia en flash: none");
				break;
				default:
					printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
				break;
			}
			nvs_get_u8(ctrl_prueba,"slaveid",&(form->slaveid));
			switch(err){
				case ESP_OK:
					ESP_LOGI(nvs_tag,"Slave ID en medidor a pulsos en flash: %d",form->slaveid);
				break;
				case ESP_ERR_NVS_NOT_FOUND:
					ESP_LOGI(nvs_tag,"Slave ID en flash: none");
				break;
				default:
					printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
				break;
			}
			err = nvs_get_str(ctrl_prueba,"tipo",NULL,&len);
			if(err==ESP_OK){
				err= nvs_get_str(ctrl_prueba,"tipo",form->tipo,&len);
			switch(err){
				case ESP_OK:
					ESP_LOGI(nvs_tag,"Tipo en flash: %s",form->tipo);
				break;
				case ESP_ERR_NVS_NOT_FOUND:
					ESP_LOGI(nvs_tag,"Tipo en flash: none");
				break;
				default:
					printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
				break;
			}
			}
			nvs_get_u16(ctrl_prueba,"conver",&(form->conversion));
			switch(err){
				case ESP_OK:
					ESP_LOGI(nvs_tag,"Factor de conversion en flash: %d",form->conversion);
				break;
				case ESP_ERR_NVS_NOT_FOUND:
					ESP_LOGI(nvs_tag,"Factor de conversion en flash: none");
				break;
				default:
					printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
				break;
			}
	}
	nvs_close(ctrl_prueba);
}

bool Llenar_intro(char *p){
	char *ini;
	char usuario[9]="mesh+user";
	char password[9]="polilla22";

	ini = strstr(p,"usuario=");
	if(ini!=NULL){
	ini+=sizeof("usuario=")-1;
		for(int i = 0;i<9;i++){
			if(ini[i]!=usuario[i]){
				return false;
			}
		}
	}

	ini = strstr(p,"contrasena=");
	if(ini!=NULL){
	ini+=sizeof("contrasena=")-1;
		for(int i = 0;i<9;i++){
				if(ini[i]!=password[i]){
					return false;
				}
			}
	}
	return true;
	}

bool Llenar_form_home(char * p, struct form_home form1){
	char *ini,aux,cono[2];
	int count;

	/*Extrayendo SSID*/
    ini=strstr(p,"ssid=");
	if(ini!=NULL){
		count = 0;
		for(int i =0;i<20;i++){
			form1.ssid[i]=0x00;
		}
		ini+=sizeof("ssid=")-1;
		for(int i=0; ini[i]!='&';i++){
			if(strncmp(&ini[i],"%",1)==0){
				cono[0]=ini[i+1];
				cono[1]=ini[i+2];
				if(cono[0]>0x40&&cono[0]<0x47){
					cono[0]-= 0x37;
				}
				if(cono[1]>0x40&&cono[1]<0x47){
					cono[1]-=0x37;
				}
				aux = ((0x0f&cono[0])<<4)|(0x0f&cono[1]);
				form1.ssid[count]=aux;
				i+=2;
				count++;
			}else{
				form1.ssid[count]=ini[i];
				count++;
				}
		}
		if (count == 0) return false;
		printf("Usuario: %s\r\n",form1.ssid);
	}
	/*Extrayendo Password WIFI*/
    ini=strstr(p,"contrasena=");
	if(ini!=NULL){
		count=0;
		for(int i =0;i<20;i++){
			form1.password[i]=0x00;
		}
		ini+=sizeof("contrasena=")-1;
		for(int i=0; ini[i]!='&';i++){
			if(strncmp(&ini[i],"%",1)==0){
				cono[0]=ini[i+1];
				cono[1]=ini[i+2];
				if(cono[0]>0x40&&cono[0]<0x47){
					cono[0]-= 0x37;
				}
				if(cono[1]>0x40&&cono[1]<0x47){
					cono[1]-=0x37;
				}
				aux = ((0x0f&cono[0])<<4)|(0x0f&cono[1]);
				form1.password[count]=aux;
				i+=2;
				count++;
			}else{
				form1.password[count]=ini[i];
				count++;}
		}
		if (count == 0) return false;
		printf("Contrasena: %s\r\n",form1.password);
	}
	/*Extrayendo Mesh ID   */
    ini=strstr(p,"meshID=");
	if(ini!=NULL){
		count=0;
		ini+=sizeof("meshID=")-1;
		for(int i=0; ini[i]!='&';i+=2){
			if(ini[i]=='%'){
				i++;
			}else{
				if(ini[i]>0x60){
					ini[i]-= 0x57;
				}
				if(ini[i+1]>0x60){
					ini[i+1]-=0x57;
				}
				aux = ((0x0f&ini[i])<<4)|(0x0f&ini[i+1]);
				form1.mesh_id[count]=aux;
				count++;
			}
		}
		if (count == 0) return false;
		printf("meshID:"MACSTR"\r\n",MAC2STR(form1.mesh_id));
	}
	/*Extrayendo Mesh AP Password*/
    ini=strstr(p,"meshAPpass=");
	if(ini!=NULL){
		count=0;
		for(int i =0;i<20;i++){
			form1.meshappass[i]=0x00;
		}
		ini+=sizeof("meshAPpass=")-1;
		for(int i=0; ini[i]!='&';i++){
			if(strncmp(&ini[i],"%",1)==0){
				cono[0]=ini[i+1];
				cono[1]=ini[i+2];
				if(cono[0]>0x40&&cono[0]<0x47){
					cono[0]-= 0x37;
				}
				if(cono[1]>0x40&&cono[1]<0x47){
					cono[1]-=0x37;
				}
				aux = ((0x0f&cono[0])<<4)|(0x0f&cono[1]);
				form1.meshappass[count]=aux;
				i+=2;
				count++;
			}else{
				form1.meshappass[count]=ini[i];
				count++;}
		}
		if (count == 0) return false;
		printf("Mesh AP Password: %s\r\n",form1.meshappass);
	}


	/*Extrayendo Layers*/
    ini=strstr(p,"layers=");
	if(ini!=NULL){
		ini+=sizeof("layers=")-1;
		aux = ((*ini)-'0')*10+(*(ini+1)-'0');
		if(aux<25) {
			form1.max_layer=aux;
			printf("Max. Layer: %d\r\n",form1.max_layer);
		}else return false;
	}
	/*Extrayendo STA*/
	    ini=strstr(p,"estaciones=");
		if(ini!=NULL){
			ini+=sizeof("estaciones=")-1;
			aux = *(ini)-'0';
			if(aux<=9) {
				form1.max_sta=aux;
				printf("Max. sta: %d\r\n",form1.max_sta);
			}else return false;
		}
		/*Extrayendo PORT*/
		ini=strstr(p,"port=");

		if(ini!=NULL){
			int aux1;
			char puerto[5];
			ini+=sizeof("port=")-1;
			for(int i = 0; ini[i]!='&';i++){
				puerto[i]=ini[i];
			}
			aux1 = atoi(puerto);
			if(aux1<=65536) {
				form1.port=(uint16_t)aux1;
				printf("PORT: %d\r\n",form1.port);
			}else return false;
		}
		set_form_flash_init(form1);
		return true;
}

bool Llenar_form_modbus(char *p,struct form_home form){
	char *ini;
	int count;
	/*Extrayendo Medida Inicial*/
	ini=strstr(p,"energia=");
	if(ini!=NULL){
		ini+=sizeof("energia=")-1;
		char energy[]="";
		uint64_t backup=0;
		for(int i = 0; ini[i]!='&';i++){
			energy[i]=ini[i];
		}
		backup = atoi(energy);
		if(backup>=0){
			form.energia = backup;
			printf("Medicion inicial:%"PRIu64"kWh\r\n",form.energia);
		}
	}
	/*Extrayendo Modo Output*/
	ini = strstr(p,"modoOutput=");
	if(ini!=NULL){
		ini +=sizeof("modoOutput=")-1;
		for(int i =0;ini[i]!='&';i++){
			form.tipo[i]=ini[i];
		}
		printf("Modo de Nodo: %s\r\n",form.tipo);
	}
	else{return false;}
	/*Extrayendo Slave IDl*/
	ini=strstr(p,"slaveid=");
	if(ini!=NULL){
		ini+=sizeof("slaveid=")-1;
		char slaveid[3];
		count = 0;
		int auxSlave;
		for(int i = 0; ini[i]!='&';i++){
			slaveid[i]=ini[i];
			count++;
		}
		slaveid[count]=0;
		auxSlave = atoi(slaveid);
		if(auxSlave>=0){
			form.slaveid =(uint8_t)auxSlave;
			printf("Slave ID:%d\r\n",form.slaveid);
		}else{
			return false;
		}
	}
	/*Extrayendo Factor de Conversion*/
		ini=strstr(p,"conversion=");
		if(ini!=NULL){
			ini+=sizeof("conversion=")-1;
			char conversion[5];
			count = 0;
			int cont;
			for(int i = 0; ini[i]!='&';i++){
				conversion[i]=ini[i];
				count++;
			}
			conversion[count]=0;
			cont = atoi(conversion);
			if(cont>=0){
				form.conversion =(uint16_t)cont;
				printf("Factor de conversion: %d imp/kWh\r\n",form.conversion);
			}else{
				return false;
			}
		}
	set_form_flash_modbus(form);
	return true;
}

// Funcion para recepcion de datos lwIP
static void WEBlocal(struct netconn *conexion,struct netconn *close){
	char *auxi;
	struct form_home home;
	struct netbuf *bufferEntrada;
	  char *buffer,aux,ini[2];
	  u16_t long_buffer;
	  err_t err;
	  err = netconn_recv(conexion, &bufferEntrada);
	  
	  if (err == ERR_OK) {
		  printf("----- Paquete Recibido -----\n");
	    netbuf_data(bufferEntrada, (void**)&buffer, &long_buffer);

	    if (strncmp(buffer,"GET /modbus HTTP/1.1",sizeof("GET /modbus HTTP/1.1")-1)==0){
	    	netconn_write(conexion, respuestaHTTP, sizeof(respuestaHTTP)-1,NETCONN_NOCOPY);
			netconn_write(conexion, abreHTML, sizeof(abreHTML)-1, NETCONN_NOCOPY);
			netconn_write(conexion, cabeceraHTML, sizeof(cabeceraHTML)-1, NETCONN_NOCOPY);
			netconn_write(conexion, cuerpoHTML_MODBUS,sizeof(cuerpoHTML_MODBUS)-1, NETCONN_NOCOPY);
			netconn_write(conexion, cierraHTML, sizeof(cierraHTML)-1, NETCONN_NOCOPY);
	    }

	    if (strncmp(buffer,"GET /mesh HTTP/1.1",sizeof("GET /mesh HTTP/1.1")-1)==0){
	    	netconn_write(conexion, respuestaHTTP, sizeof(respuestaHTTP)-1,NETCONN_NOCOPY);
			netconn_write(conexion, abreHTML, sizeof(abreHTML)-1, NETCONN_NOCOPY);
			netconn_write(conexion, cabeceraHTML, sizeof(cabeceraHTML)-1, NETCONN_NOCOPY);
			netconn_write(conexion, cuerpoHTML_INI, sizeof(cuerpoHTML_INI)-1, NETCONN_NOCOPY);
			netconn_write(conexion, cierraHTML, sizeof(cierraHTML)-1, NETCONN_NOCOPY);
	    }

	    if (strncmp(buffer,"GET /mqtt HTTP/1.1",sizeof("GET /mqtt HTTP/1.1")-1)==0){
			netconn_write(conexion, respuestaHTTP, sizeof(respuestaHTTP)-1,NETCONN_NOCOPY);
			netconn_write(conexion, abreHTML, sizeof(abreHTML)-1, NETCONN_NOCOPY);
			netconn_write(conexion, cabeceraHTML, sizeof(cabeceraHTML)-1, NETCONN_NOCOPY);
			netconn_write(conexion, cuerpoHTML_INI, sizeof(cuerpoHTML_INI)-1, NETCONN_NOCOPY);
			netconn_write(conexion, cierraHTML, sizeof(cierraHTML)-1, NETCONN_NOCOPY);
			netconn_write(conexion, cuerpoALERTApaga, sizeof(cuerpoALERTApaga)-1, NETCONN_NOCOPY);
		}

	    if (strncmp(buffer,"GET /modbus?conversion",sizeof("GET /modbus?conversion")-1)==0){
	    	for(int i=0;buffer[i]!=0;i++){
				if(strncmp(&buffer[i],"%",1)==0){

					ini[0]=buffer[i+1];
					ini[1]=buffer[i+2];
					if(ini[0]>0x40&&ini[0]<0x47){
						ini[0]-= 0x37;
					}
					if(ini[1]>0x40&&ini[1]<0x47){
						ini[1]-=0x37;
					}
					aux = ((0x0f&ini[0])<<4)|(0x0f&ini[1]);
					printf("%c",aux);
					i+=2;
				}else{
					printf("%c",buffer[i]);}
			}
			printf("\n");
			if (Llenar_form_modbus(buffer,home)){
				netconn_write(conexion, respuestaHTTP, sizeof(respuestaHTTP)-1,NETCONN_NOCOPY);
				netconn_write(conexion, abreHTML, sizeof(abreHTML)-1, NETCONN_NOCOPY);
				netconn_write(conexion, cabeceraHTML, sizeof(cabeceraHTML)-1, NETCONN_NOCOPY);
				netconn_write(conexion, cuerpoHTML_MODBUS, sizeof(cuerpoHTML_MODBUS)-1, NETCONN_NOCOPY);
				netconn_write(conexion, cuerpoALERTAok, sizeof(cuerpoALERTAok)-1, NETCONN_NOCOPY);
				netconn_write(conexion, cierraHTML, sizeof(cierraHTML)-1, NETCONN_NOCOPY);
			}
			else{
				netconn_write(conexion, respuestaHTTP, sizeof(respuestaHTTP)-1,NETCONN_NOCOPY);
				netconn_write(conexion, abreHTML, sizeof(abreHTML)-1, NETCONN_NOCOPY);
				netconn_write(conexion, cabeceraHTML, sizeof(cabeceraHTML)-1, NETCONN_NOCOPY);
				netconn_write(conexion, cuerpoALERTAnok, sizeof(cuerpoALERTAnok)-1, NETCONN_NOCOPY);
				netconn_write(conexion, cuerpoHTML_MODBUS, sizeof(cuerpoHTML_MODBUS)-1, NETCONN_NOCOPY);
				netconn_write(conexion, cierraHTML, sizeof(cierraHTML)-1, NETCONN_NOCOPY);

				}
	  	    }

	    if (strncmp(buffer,"GET /?usuario",13)==0){
	    	if(Llenar_intro(buffer)){
	    		netconn_write(conexion, respuestaHTTP, sizeof(respuestaHTTP)-1,NETCONN_NOCOPY);
				netconn_write(conexion, abreHTML, sizeof(abreHTML)-1, NETCONN_NOCOPY);
				netconn_write(conexion, cabeceraHTML, sizeof(cabeceraHTML)-1, NETCONN_NOCOPY);
				netconn_write(conexion, cuerpoHTML_INI, sizeof(cuerpoHTML_INI)-1, NETCONN_NOCOPY);
				netconn_write(conexion, cierraHTML, sizeof(cierraHTML)-1, NETCONN_NOCOPY);
	    	}else{
	    		netconn_write(conexion, respuestaHTTP, sizeof(respuestaHTTP)-1,NETCONN_NOCOPY);
				netconn_write(conexion, abreHTML, sizeof(abreHTML)-1, NETCONN_NOCOPY);
				netconn_write(conexion, cabeceraHTML, sizeof(cabeceraHTML)-1, NETCONN_NOCOPY);
				netconn_write(conexion, cuerpoHTML, sizeof(cuerpoHTML)-1, NETCONN_NOCOPY);
				netconn_write(conexion, cuerpoALERTAintro, sizeof(cuerpoALERTAintro)-1, NETCONN_NOCOPY);
				netconn_write(conexion, cierraHTML, sizeof(cierraHTML)-1, NETCONN_NOCOPY);
	    	}
	    }

	    if (strncmp(buffer,"GET /mesh?ssid=",sizeof("GET /mesh?ssid=")-1)==0){

	        for(int i=0;buffer[i]!=0;i++){
	        	if(strncmp(&buffer[i],"%",1)==0){

	        		ini[0]=buffer[i+1];
	        		ini[1]=buffer[i+2];
					if(ini[0]>0x40&&ini[0]<0x47){
						ini[0]-= 0x37;
					}
					if(ini[1]>0x40&&ini[1]<0x47){
						ini[1]-=0x37;
					}
	        		aux = ((0x0f&ini[0])<<4)|(0x0f&ini[1]);
	        		printf("%c",aux);
	        		i+=2;
				}else{
	        		printf("%c",buffer[i]);}
	        }
	        printf("\n");
	        if (Llenar_form_home(buffer,home)){
	        	netconn_write(conexion, respuestaHTTP, sizeof(respuestaHTTP)-1,NETCONN_NOCOPY);
	        	netconn_write(conexion, abreHTML, sizeof(abreHTML)-1, NETCONN_NOCOPY);
	        	netconn_write(conexion, cabeceraHTML, sizeof(cabeceraHTML)-1, NETCONN_NOCOPY);
	        	netconn_write(conexion, cuerpoHTML_INI, sizeof(cuerpoHTML_INI)-1, NETCONN_NOCOPY);
	        	netconn_write(conexion, cuerpoALERTAok, sizeof(cuerpoALERTAok)-1, NETCONN_NOCOPY);
	        	netconn_write(conexion, cierraHTML, sizeof(cierraHTML)-1, NETCONN_NOCOPY);
	        }
	        else{
	        	netconn_write(conexion, respuestaHTTP, sizeof(respuestaHTTP)-1,NETCONN_NOCOPY);
				netconn_write(conexion, abreHTML, sizeof(abreHTML)-1, NETCONN_NOCOPY);
				netconn_write(conexion, cabeceraHTML, sizeof(cabeceraHTML)-1, NETCONN_NOCOPY);
				netconn_write(conexion, cuerpoALERTAnok, sizeof(cuerpoALERTAnok)-1, NETCONN_NOCOPY);
				netconn_write(conexion, cuerpoHTML_INI, sizeof(cuerpoHTML_INI)-1, NETCONN_NOCOPY);
				netconn_write(conexion, cierraHTML, sizeof(cierraHTML)-1, NETCONN_NOCOPY);

	        }
	    }
	    if(strncmp(buffer,"GET / HTTP/1.1",sizeof("GET / HTTP/1.1")-1)==0){
	    	for(int i=0;buffer[i]!=0;i++){
	    		if(strncmp(&buffer[i],"%",1)==0){

					ini[0]=buffer[i+1];
					ini[1]=buffer[i+2];
					if(ini[0]>0x40&&ini[0]<0x47){
						ini[0]-= 0x37;
					}
					if(ini[1]>0x40&&ini[1]<0x47){
						ini[1]-=0x37;
					}
					aux = ((0x0f&ini[0])<<4)|(0x0f&ini[1]);
					printf("%c",aux);
					i+=2;
				}else{
					printf("%c",buffer[i]);}
			}
			printf("\n");
	        netconn_write(conexion, respuestaHTTP, sizeof(respuestaHTTP)-1,NETCONN_NOCOPY);
	        netconn_write(conexion, abreHTML, sizeof(abreHTML)-1, NETCONN_NOCOPY);
	        netconn_write(conexion, cabeceraHTML, sizeof(cabeceraHTML)-1, NETCONN_NOCOPY);
	        netconn_write(conexion, cuerpoHTML, sizeof(cuerpoHTML)-1, NETCONN_NOCOPY);
	        netconn_write(conexion, cierraHTML, sizeof(cierraHTML)-1, NETCONN_NOCOPY);
	    }
	    if(strncmp(buffer,"POST /",sizeof("POST /")-1)==0){
	    	auxi = strstr(buffer,"reinicio=");
	    	if(auxi!=NULL){
	    		netconn_write(conexion, respuestaHTTP, sizeof(respuestaHTTP)-1,NETCONN_NOCOPY);
				netconn_write(conexion, abreHTML, sizeof(abreHTML)-1, NETCONN_NOCOPY);
				netconn_write(conexion, cabeceraHTML, sizeof(cabeceraHTML)-1, NETCONN_NOCOPY);
				netconn_write(conexion, cuerpoReinicio, sizeof(cuerpoReinicio)-1, NETCONN_NOCOPY);
				netconn_write(conexion, cierraHTML, sizeof(cierraHTML)-1, NETCONN_NOCOPY);
	    		netconn_close(conexion);
	    		netbuf_delete(bufferEntrada);
	    		netconn_delete(conexion);
	    		netconn_close(close);
	    		netconn_delete(close);
	    		esp_restart();
	    	}
	    	for(int i=0;buffer[i]!=0;i++){
			if(strncmp(&buffer[i],"%",1)==0){

				ini[0]=buffer[i+1];
				ini[1]=buffer[i+2];
				if(ini[0]>0x40&&ini[0]<0x47){
					ini[0]-= 0x37;
				}
				if(ini[1]>0x40&&ini[1]<0x47){
					ini[1]-=0x37;
				}
				aux = ((0x0f&ini[0])<<4)|(0x0f&ini[1]);
				printf("%c",aux);
				i+=2;
			}else{
				printf("%c",buffer[i]);}
		}
		printf("\n");

	    }
	  }
	  netconn_close(conexion);
	  netbuf_delete(bufferEntrada);
}

// Tarea RTOS para el servidor --------------------------------------------------//
void tareaSOCKET(void *P){
	esp_wifi_restore();
	iniciar_wifi();
	struct netconn *conectar, *NuevaCon;
	  err_t err;
	  conectar = netconn_new(NETCONN_TCP);
	  netconn_bind(conectar, NULL, PUERTO); // se asigno el puerto 80
	  netconn_listen(conectar);
	  do {
	     err = netconn_accept(conectar, &NuevaCon);
	     if (err == ERR_OK) {
	    	 WEBlocal(NuevaCon,conectar);
	    	 netconn_delete(NuevaCon);
	     }
	   } while(err == ERR_OK);

	   netconn_close(conectar);
	   netconn_delete(conectar);
}
/*********************************************************************************/
