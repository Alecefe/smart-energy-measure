#include "UARTacceso.h"

static char salvarU[30],salvarC[30];
static uint8_t variable = msn_usuario, cuenta = 0;

static void habilitador(uint8_t estado){
	char Rx[2];
	const char errorU1[] = "\n\n***************ERROR***************\n\r"
			"*  Combinacion incorrecta         *\n\r";
	const char errorU2[] = "*  Acceso: DENEGADO               *\n\r"
			"***********************************\n\n\r";
	const char insertaUSUARIO[] = "===================================\n\r"
			"    Escriba el nombre de usuario\n\r"
			"-----------------------------------\n\r";
	const char insertaCLAVE[] = "\n\r===================================\n\r"
			"    Escriba la clave\n\r"
			"-----------------------------------\n\r";
	const char paseC1[] = "\n\n\r===================================\n\r"
			"#   Usuario y clave correctos     #\n\r"
			"#   Iniciando modulo WiFi         #\n\r";
	const char paseC2[] = "#   Conectando - ***********      #\n\r"
			"#   Clave      - **********       #\n\r"
			"===================================\n\n\r";
	switch(estado){
	case msn_usuario:
		uart_write_bytes(UART_NUM_0,(const char*)insertaUSUARIO,strlen(insertaUSUARIO));
		variable = espera_usuario;
		break;
	case espera_usuario:
		if(xQueueReceive(Cola_UART,&Rx,1000/portTICK_RATE_MS)==pdTRUE){
			uart_write_bytes(UART_NUM_0,(const char*)Rx,strlen(Rx));
			salvarU[cuenta]=Rx[0];
			cuenta++;
			if(cuenta>29 || Rx[0]==0x0D){
				salvarU[cuenta-1] = '\0';
				variable = msn_clave;
				cuenta=0;
			}
		}
		break;
	case msn_clave:
		uart_write_bytes(UART_NUM_0,(const char*)insertaCLAVE,strlen(insertaCLAVE));
		variable = espera_clave;
		break;
	case espera_clave:
		if(xQueueReceive(Cola_UART,&Rx,1000/portTICK_RATE_MS)==pdTRUE){
			uart_write_bytes(UART_NUM_0,(const char*)Rx,strlen(Rx));
			salvarC[cuenta]=Rx[0];
			cuenta++;
			if(cuenta>29 || Rx[0]==0x0D){
				salvarC[cuenta-1] = '\0';
				variable = verificacion;
				cuenta = 0;
				uart_write_bytes(UART_NUM_0,(const char*)"\n\r===================================\n\r",39);
				vTaskDelay(10/portTICK_PERIOD_MS);
			}
		}
		break;
	case verificacion:
		if(strcmp(salvarU,usuario)==0 && strcmp(salvarC,pase)==0){
			uart_write_bytes(UART_NUM_0,(const char*)paseC1,strlen(paseC1));
			vTaskDelay(10/portTICK_PERIOD_MS);
			uart_write_bytes(UART_NUM_0,(const char*)paseC2,strlen(paseC2));
			vTaskDelay(500/portTICK_PERIOD_MS);
			xTaskCreatePinnedToCore(&tareaSOCKET,"SOCKET_HTTP",1024*3,NULL,3,NULL,1);
			vTaskDelete(NULL);
		}else{
			uart_write_bytes(UART_NUM_0,(const char*)errorU1,strlen(errorU1));
			vTaskDelay(10/portTICK_PERIOD_MS);
			uart_write_bytes(UART_NUM_0,(const char*)errorU2,strlen(errorU2));
			vTaskDelay(10/portTICK_PERIOD_MS);
			for(int i=0;i<30;i++){
				salvarU[i] = 0;
				salvarC[i] = 0;
			}
			variable = msn_usuario;
		}
		break;
	default:
		break;
	}
}
void tarea1(void* P){
	for(;;){
		habilitador(variable);
	}
}
