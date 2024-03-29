#ifndef MAIN_UART1_H_
#define MAIN_UART1_H_

#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "webHTTP.h"

#define uart1 UART_NUM_1

#define RS485 21
#define Tx1 4
#define Rx1 5
#define tamBUFFER 128

#define EX_UART_NUM UART_NUM_1
#define PATTERN_CHR_NUM                                                       \
  (3) /*!< Set the number of consecutive and identical characters received by \
         \ \ \ \ \ \ receiver which defines a UART pattern*/

#define BUF_SIZE (1024)
#define RD_BUF_SIZE (BUF_SIZE)

QueueHandle_t RxRS485;
EventGroupHandle_t event_uart1;

typedef union {
  uint16_t Val;
  struct {
    uint8_t LB;
    uint8_t HB;
  } byte;
} auxiliar;

// static void uart_event_task(void *pvParameters);

void iniciarUART(tipo_de_medidor tipo, uint32_t baud);
#endif
