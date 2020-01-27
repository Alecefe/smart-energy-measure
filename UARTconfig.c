/********************************************************************************
**********   Archivo fuente .c para configurar UART e interrupciones   **********
********************************************************************************/
// Inclusión de bibliotecas ---------------------------------------------------//
#include "UARTconfig.h"
// Variables globales ---------------------------------------------------------//
static intr_handle_t manejadorUART0;
uint8_t bufferRx0[128];

/* Rutina de atención de interrupción en UART0 --------------------------------*/
static void IRAM_ATTR intUART0(void *arg){
	uint16_t longFIFORx0,i=0;
	  longFIFORx0 = UART0.status.rxfifo_cnt; // se lee el numero de bytes que llegaron
	  while(longFIFORx0){
		  bufferRx0[i++] = UART0.fifo.rw_byte; // Se leen todos los byte recibidos
		  longFIFORx0--;
	 }
	 // Luego de recibir los bytes se limpia el estado de la bandera de interrupción
	 uart_clear_intr_status(UART_NUM_0, UART_RXFIFO_FULL_INT_CLR|UART_RXFIFO_TOUT_INT_CLR);
	 xQueueSendFromISR(Cola_UART, &bufferRx0,0/portTICK_RATE_MS);
}

// Función para inicializar el UART 0
void iniciarUART(){
	const int uart0 = UART_NUM_0;
	uart_config_t configUART0 = {
	    .baud_rate = 115200,
	    .data_bits = UART_DATA_8_BITS,
	    .parity = UART_PARITY_DISABLE,
	    .stop_bits = UART_STOP_BITS_1,
	    .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
	};
	uart_param_config(uart0, &configUART0);
	uart_set_pin(uart0, (-1), (-1), (-1), (-1));
	uart_driver_install(uart0, tamBUFFER * 2, 0, 0, NULL, 0);
	// Comandos para liberar interrupciones y evitar errores
	uart_isr_free(UART_NUM_0);
	// Comando para registrar la interrupción correspondiente a UART0
	uart_isr_register(UART_NUM_0,intUART0, NULL, ESP_INTR_FLAG_IRAM, &manejadorUART0);
	// Se deben habilitar las interrupciones necesarias
	uart_enable_rx_intr(UART_NUM_0);
}
/************************************************************************************/
