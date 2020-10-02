
#include "include/UART1.h"

static const char *TAG = "uart_events";
static const char *uart_tag = "uart_task";
static QueueHandle_t uart0_queue;

static void uart_event_task(void *pvParameters) {
  INT_VAL tamano;
  uint8_t mensaje[tamBUFFER];
  uart_event_t event;
  uint8_t *dtmp = (uint8_t *)malloc(RD_BUF_SIZE);

  while (!esp_mesh_is_root()) {
    // Waiting for UART event.
    if (xQueueReceive(uart0_queue, (void *)&event,
                      (portTickType)portMAX_DELAY)) {
      bzero(dtmp, RD_BUF_SIZE);

      ESP_LOGI(TAG, "uart[%d] event:", EX_UART_NUM);
      switch (event.type) {
          // Event of UART receving data

        case UART_DATA:

          ESP_LOGI(TAG, "[UART DATA]: %d", event.size);
          uart_read_bytes(EX_UART_NUM, dtmp, event.size, portMAX_DELAY);
          ESP_LOGI(TAG, "[DATA EVT]:");
          tamano.Val = (uint16_t)event.size;
          mensaje[0] = tamano.byte.HB;
          mensaje[1] = tamano.byte.LB;
          for (int i = 0; i < event.size; i++) {
            mensaje[i + 2] = *(dtmp + i);
            printf("mensaje[%d] = %02x\r\n", i, mensaje[i]);
          }

          if (event.size > 1) {
            xQueueSendToFront(RxRS485, mensaje, portMAX_DELAY);
          }
          // uart_write_bytes(EX_UART_NUM, (const char*) dtmp, event.size);
          break;
        // Event of HW FIFO overflow detected
        case UART_FIFO_OVF:
          ESP_LOGI(TAG, "hw fifo overflow");
          // If fifo overflow happened, you should consider adding flow control
          // for your application. The ISR has already reset the rx FIFO, As an
          // example, we directly flush the rx buffer here in order to read more
          // data.
          uart_flush_input(EX_UART_NUM);
          xQueueReset(uart0_queue);
          break;
        // Event of UART ring buffer full
        case UART_BUFFER_FULL:
          ESP_LOGI(TAG, "ring buffer full");
          // If buffer full happened, you should consider encreasing your buffer
          // size As an example, we directly flush the rx buffer here in order
          // to read more data.
          uart_flush_input(EX_UART_NUM);
          xQueueReset(uart0_queue);
          break;
        // Event of UART RX break detected
        case UART_BREAK:
          ESP_LOGI(TAG, "uart rx break");
          break;
        // Event of UART parity check error
        case UART_PARITY_ERR:
          ESP_LOGI(TAG, "uart parity error");
          break;
        // Event of UART frame error
        case UART_FRAME_ERR:
          ESP_LOGI(TAG, "uart frame error");
          break;
        // Others
        default:
          ESP_LOGI(TAG, "uart event type: %d", event.type);
          break;
      }
    }
  }
  free(dtmp);
  dtmp = NULL;
  vTaskDelete(NULL);
}

static void uart_chino(void *arg) {
  int length = 0;
  uint8_t rx_bufchino[128] = {
      0x00,
  };

  while (!esp_mesh_is_root()) {
    uart_get_buffered_data_len(UART_NUM_1, (size_t *)&length);
    if (length > 8) {
      uart_read_bytes(UART_NUM_1, &rx_bufchino[2], length, portMAX_DELAY);
      rx_bufchino[0] = 0x00;
      rx_bufchino[1] = (uint8_t)length;
      printf("Hola 1");
      xQueueSendToFront(RxRS485, rx_bufchino, portMAX_DELAY);
      uart_flush(UART_NUM_1);
    }
    vTaskDelay(pdMS_TO_TICKS(20));
  }
  ESP_LOGE(uart_tag, "Se ha eliminado la tarea del medidor chino");
  vTaskDelete(NULL);
}

void iniciarUART(tipo_de_medidor tipo, uint32_t baud) {  //

  // Configuración para el UART 1
  // -------------------------------------------------------//
  uart_config_t configUART1 = {
      //
      .baud_rate = (int)baud,
      .data_bits = UART_DATA_8_BITS,
      .parity = UART_PARITY_DISABLE,
      .stop_bits = UART_STOP_BITS_1,
  };

  printf("Config Baud rate %d\r\n", configUART1.baud_rate);
  //
  ESP_ERROR_CHECK(uart_param_config(
      uart1, &configUART1));  // Se configuran los terminales para los UART
  ESP_ERROR_CHECK(
      uart_set_pin(uart1, Tx1, Rx1, RS485,
                   UART_PIN_NO_CHANGE));  // Se instalan los controladores UART
                                          // de la configuraci�n anterior
  // --------------------//
  uart_driver_install(uart1, BUF_SIZE * 2, BUF_SIZE * 2, 20, &uart0_queue,
                      0);  //
  // Se establece el modo rs485 //
  ESP_ERROR_CHECK(uart_set_mode(uart1, UART_MODE_RS485_HALF_DUPLEX));
  RxRS485 = xQueueCreate(5, 128);

  switch (tipo) {
    case (rs485):
      xTaskCreate(uart_event_task, "uart_event_task", 2048, NULL, 5, NULL);
      break;
    case (chino):
      xTaskCreate(uart_chino, "UART CHINO", 2048, NULL, 5, NULL);
      break;
    default:
      break;
  }
}
