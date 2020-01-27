/*********************************************************************************/
#include "webHTTP.h"
// Variables constantes que definen la pagina web -------------------------------//
const static char respuestaHTTP[] = "HTTP/1.1 200 OK\r\nContent-type:"
		"text/html\r\n\r\n";
const static char abreHTML[] = "<html>";
const static char cabeceraHTML[] = "<head>"
        "<title>Control</title>"
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
                    "<label style=\"font-weight: bold;\">ESP32 Configuracion de nodos</label>"
                    "<form name=\"FormConfig\" style=\"margin: 10%;\">"
                        "<label id=\"SSID\" >SSID:</label><br>"
                        "<input class=\"opcion\"type=\"text\" name=\"usuario\"><br>"
                        "<label >Password:</label><br>"
                        "<input class=\"opcion\"type=\"password\" name=""><br>"
                        "<label >Mesh ID:</label><br>"
                        "<input class=\"opcion\"type=\"text\" name=""><br>"
                        "<label >Max. Layers:</label><br>"
                        "<input class=\"opcion\"type=\"text\" name=""><br>"
                        "<label >Max. STA:</label><br>"
                        "<input class=\"opcion\"type=\"text\" name=""><br>"
                        "<label >Puerto (Socket):</label><br>"
                        "<input class=\"opcion\"type=\"text\" name=""><br>"
                        "<input class=\"opcion\"type=\"button\" name=\"enter\" value=\"Submit\"onclick=\"comprobar()\"style=\"margin:10px;\">"
                        "<input class=\"opcion\"type=\"button\" name=\"enter\" value=\"Cancel\"onclick=\"eliminar()\"style=\"margin:10px;\">"
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
// Funcion para recepcion de datos lwIP
static void WEBlocal(struct netconn *conexion){
	struct netbuf *bufferEntrada;
	  char *buffer;
	  u16_t long_buffer;
	  err_t err;
	  err = netconn_recv(conexion, &bufferEntrada);
	  
	  if (err == ERR_OK) {
		  printf("----- Paquete Recibido -----\n");
	    netbuf_data(bufferEntrada, (void**)&buffer, &long_buffer);
	    if (strncmp(buffer,"GET /",5)==0){
	        for(int i=0;buffer[i]!=0;i++)printf("%c",buffer[i]);
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
