/*********************************************************************************/
#include "webHTTP.h"

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
        "<div id=\"pagina\" style=\" display: block;height:100vh;background-color:blue; background-image: linear-gradient(to bottom right, white,blue,white);width: 100%;\">"
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

bool Llenar_intro(char *p){
	char *ini;
	int count;
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

void Llenar_form_home(char * p, struct form_home form1){
	char *ini,aux,cono[2];
	int count;

	/*Extrayendo SSID*/
    ini=strstr(p,"ssid=");
	if(ini!=NULL){
		count = 0;
		for(int i =0;i<20;i++){
			form1.ssid[i]=NULL;
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
		printf("Usuario: %s\r\n",form1.ssid);
	}
	/*Extrayendo Password WIFI*/
    ini=strstr(p,"contrasena=");
	if(ini!=NULL){
		count=0;
		for(int i =0;i<20;i++){
			form1.password[i]=NULL;
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
			printf("Max. Layer: %d\r\n",form1.max_layer);
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
			}
		}
}

// Funcion para recepcion de datos lwIP
static void WEBlocal(struct netconn *conexion){
	struct form_home home;
	struct netbuf *bufferEntrada;
	  char *buffer,aux,ini[2];
	  u16_t long_buffer;
	  err_t err;
	  err = netconn_recv(conexion, &bufferEntrada);
	  
	  if (err == ERR_OK) {
		  printf("----- Paquete Recibido -----\n");
	    netbuf_data(bufferEntrada, (void**)&buffer, &long_buffer);

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
				netconn_write(conexion, cierraHTML, sizeof(cierraHTML)-1, NETCONN_NOCOPY);
	    	}
	    }

	    if (strncmp(buffer,"GET /?ssid",10)==0){

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
	        Llenar_form_home(buffer,home);
	        netconn_write(conexion, respuestaHTTP, sizeof(respuestaHTTP)-1,NETCONN_NOCOPY);
	    }
	    if(strncmp(buffer,"GET / HTTP/1.1",14)==0){
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
