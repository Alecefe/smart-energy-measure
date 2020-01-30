/*********************************************************************************/
#include "webHTTP.h"
// Variables constantes que definen la pagina web -------------------------------//
const static char respuestaHTTP[] = "HTTP/1.1 200 OK\r\nContent-type:"
		"text/html\r\n\r\n";
const static char abreHTML[] = "<!DOCTYPE html><html lang=\"es\">";
const static char cabeceraHTML[] = "<head>"
        "<title>Control</title>"
		"<meta charset=\"UTF-8\">"
        "<script>"
            "function comprobar(){"
                "if(document.entrada.usuario.value=='admin'&&document.entrada.contrasena.value=='admin'){"
                    "document.getElementById('formInput').style.display=\"none\";"
                    "document.getElementById('pagina').style.display=\"block\";"
                "}else{"
                    "alert('Usuario o Password Incorrecto');"
                "}"
            "}"
        "</script>"
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
const static char cuerpoHTML[] = "<body id=\"todo\">"
        "<div id=\"formInput\" style=\"margin: 0 auto;margin-top:20%;display:inline-block;text-align: center;display: block;padding: 2%;width: 25%;border-radius: 2% 2% 2% 2%; border: 5px ridge #000000; background-color: white;\">"
                "<form name=\"entrada\">"
                    "<label id=\"usuario\">User:</label><br>"
                    "<input type=\"text\" name=\"usuario\"><br>"
                    "<label>Password</label><br>"
                    "<input type=\"password\" name=\"contrasena\"><br>"
                    "<input type=\"button\" name=\"enter\" value=\"Submit\"onclick=\"comprobar()\"style=\"margin:10px;\">"
                    "<input type=\"button\" name=\"enter\" value=\"Cancel\"onclick=\"eliminar()\"style=\"margin:10px;\">"
                "</form>"
                "</div>"
        "<div id=\"pagina\" style=\" display: none;height:100vh;background-color:blue; background-image: linear-gradient(to bottom right, white,blue,white);width: 100%;\">"
            "<aside id=\"menu\" style=\"width:20%;float:left; background-color: #eee;\">"
                    "<div class=\"vertical-menu\" style=\"height: 100vh;\">"
                            "<a href=\"#\" class=\"active\">Home</a>"
                            "<a href=\"#\">Link 1</a>"
                            "<a href=\"#\">Link 2</a>"
                            "<a href=\"#\">Link 3</a>"
                            "<a href=\"#\">Link 4</a>"
                        "</div>"
            "</aside>"
            "<div id=\"FormHome\" style=\"height: 80vh;position:relative;left: 10%;top: 5%;margin: 0 auto;display:inline-block;text-align: center;display: block;padding: 2%;width: 25%;border-radius: 2% 2% 2% 2%; border: 5px ridge #000000; background-color: white;\">"
                    "<label style=\"font-weight: bold;\">ESP32 Node Configuration</label>"
                    "<form name=\"FormConfig\" style=\"margin: 10%;\">"
                        "<label id=\"SSID\" >SSID:</label><br>"
                        "<input class=\"opcion\"type=\"text\" name=\"ssid\" required><br>"
                        "<label >Password:</label><br>"
                        "<input class=\"opcion\"type=\"password\" name=\"contrasena\" required><br>"
                        "<label >Mesh ID:</label><br>"
                        "<input class=\"opcion\"type=\"text\" name=\"meshID\" pattern=\"^([0-9a-f][0-9a-f]:){5}([0-9a-f][0-9a-f])$\" placeholder=\"77:77:77:77:77:77\" required><br>"
                        "<label >Max. Layers:</label><br>"
                        "<input class=\"opcion\"type=\"number\" name=\"layers\" min=\"10\" max=\"25\"placeholder=\"Between 10 - 25\"required><br>"
                        "<label >Max. STA:</label><br>"
                        "<input class=\"opcion\"type=\"number\" name=\"estaciones\" min=\"1\" max=\"9\"placeholder=\"Between 1 - 9\"required><br>"
                        "<label >Puerto (Socket):</label><br>"
                        "<input class=\"opcion\"type=\"number\" name=\"port\" min=\"1\" max=\"65535\" placeholder=\"Between 0 - 65536\"><br>"
                        "<input class=\"opcion\"type=\"submit\" name=\"submit\" value=\"Submit\"style=\"margin:10px;\">"
                        "<input class=\"opcion\"type=\"button\" name=\"enter\" value=\"Cancel\"style=\"margin:10px;\">"
                    "</form>"
            "</div>"
        "</div>"
    "</body>";
const static char cierraHTML[] = "</html>";
static void configurarGPIO(){
	gpio_set_direction(LEDg, GPIO_MODE_INPUT_OUTPUT);
	gpio_set_direction(LEDr, GPIO_MODE_INPUT_OUTPUT);
	gpio_set_direction(LEDb, GPIO_MODE_INPUT_OUTPUT);
}
void Llenar_form_home(char * p, struct form_home form1){
	char *ini,aux;
	int count;

	/*Extrayendo SSID*/
    ini=strstr(p,"ssid=");
	if(ini!=NULL){
		count = 0;
		ini+=sizeof("ssid=")-1;
		for(int i=0; ini[i]!='&';i++){
			if(strncmp(&ini[i],"%",1)==0){
				aux = ((0x0f&ini[i+1])<<4)|(0x0f&ini[i+2]);
				form1.ssid[count]=aux;
				count++;
				i+=2;
			}else{
				form1.ssid[count]=ini[i];
				count++;
				}
		}
		printf("Usuario: %s\r\n",form1.ssid);
	}
	/*Extrayendo Password WIFI*/
    ini=strstr(p,"contrasena=");
	if(ini!=NULL){
		count=0;
		ini+=sizeof("contrasena=")-1;
		for(int i=0; ini[i]!='&';i++){
			if(strncmp(&ini[i],"%",1)==0){
				aux = ((0x0f&ini[i+1])<<4)|(0x0f&ini[i+2]);
				form1.password[count]=aux;
				count++;
				i+=2;
			}else{
				form1.password[count]=ini[i];
				count++;}
		}
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
		printf("meshID:"MACSTR"\r\n",MAC2STR(form1.mesh_id));
	}
	/*Extrayendo Layers*/
    ini=strstr(p,"layers=");
	if(ini!=NULL){
		ini+=sizeof("layers=")-1;
		aux = ((*ini)-'0')*10+(*(ini+1)-'0');
		if(aux<25) {
			form1.max_layer=aux;
			printf("Max. Layer: %d\n",form1.max_layer);
		}
	}
	/*Extrayendo STA*/
	    ini=strstr(p,"estaciones=");
		if(ini!=NULL){
			ini+=sizeof("estaciones=")-1;
			aux = *(ini)-'0';
			if(aux<=9) {
				form1.max_sta=aux;
				printf("Max. sta: %d\r\n",form1.max_sta);
			}
		}
		/*Extrayendo PORT*/
		ini=strstr(p,"port=");
		if(ini!=NULL){
			count=0;
			char *puerto = (char*)malloc(sizeof(char)*5);
			ini+=sizeof("port=")-1;
			for(int i = 0; ini[i]=='&';i++){
				puerto[i]=ini[i];
				count++;
			}

			strtol();

			if(aux<=65536) {
				form1.max_sta=aux;
				printf("Max. sta: %d\r\n",form1.max_sta);
			}
		}
}

// Funcion para recepcion de datos lwIP
static void WEBlocal(struct netconn *conexion){
	struct form_home home;
	struct netbuf *bufferEntrada;
	  char *buffer,aux,*ini;
	  u16_t long_buffer;
	  err_t err;
	  err = netconn_recv(conexion, &bufferEntrada);
	  
	  if (err == ERR_OK) {
		  printf("----- Paquete Recibido -----\n");
	    netbuf_data(bufferEntrada, (void**)&buffer, &long_buffer);
	    if (strncmp(buffer,"GET /",5)==0){
	    	//Llenar_form_home(buffer,home);

	        for(int i=0;buffer[i]!=0;i++){
	        	/*
	        	if(strncmp(&buffer[i],"%",1)==0){

	        		ini=buffer;
	        		if(ini[i]>0x60&&ini[i]<0x67){
						ini[i]-= 0x57;
					}
					if(ini[i+1]>0x60&&ini[i+1]<0x67){
						ini[i+1]-=0x57;
					}
					if(ini[i]>0x40&&ini[i]<0x47){
						ini[i]-= 0x37;
					}
					if(ini[i+1]>0x40&&ini[i+1]<0x47){
						ini[i+1]-=0x37;
					}
	        		aux = ((0x0f&buffer[i+1])<<4)|(0x0f&buffer[i+2]);
	        		printf("%c",aux);
	        		i+=2;
	        	}else{
	        	printf("%c",buffer[i]);}*/
	        	printf("%c",buffer[i]);}
	        printf("\n");

	        if(strncmp(buffer,"GET /LEDg",9)==0){
	        	gpio_set_level(LEDg,!gpio_get_level(LEDg));
	        }
	        if(strncmp(buffer,"GET /LEDr",9)==0){
	        	gpio_set_level(LEDr,!gpio_get_level(LEDr));
	        }
	        if(strncmp(buffer,"GET /LEDb",9)==0){
	        	gpio_set_level(LEDb,!gpio_get_level(LEDb));
	        }
	        netconn_write(conexion, respuestaHTTP, sizeof(respuestaHTTP)-1,NETCONN_NOCOPY);
	        netconn_write(conexion, abreHTML, sizeof(abreHTML)-1, NETCONN_NOCOPY);
	        netconn_write(conexion, cabeceraHTML, sizeof(cabeceraHTML)-1, NETCONN_NOCOPY);
	        netconn_write(conexion, cuerpoHTML, sizeof(cuerpoHTML)-1, NETCONN_NOCOPY);
	        netconn_write(conexion, cierraHTML, sizeof(cierraHTML)-1, NETCONN_NOCOPY);
	    }
	  }
	  netconn_close(conexion);
	  netbuf_delete(bufferEntrada);
}
// Tarea RTOS para el servidor --------------------------------------------------//
void tareaSOCKET(void *P){
	configurarGPIO();
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
	    	 WEBlocal(NuevaCon);
	       netconn_delete(NuevaCon);
	     }
	   } while(err == ERR_OK);
	   netconn_close(conectar);
	   netconn_delete(conectar);
}
/*********************************************************************************/
