#include "UARTconfig.h"
#include "UARTacceso.h"
#include "WiFiconfig.h"
#define Pila 1024
void app_main(){
	nvs_flash_init();
	Cola_UART = xQueueCreate(tamCOLA,tamMSN);
	iniciarUART();
	xTaskCreatePinnedToCore(&tarea1,"PaseUART",Pila*2,NULL,3,NULL,0);
}
