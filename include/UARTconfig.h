/******************************************************************************
********** Archivo de cabecera para configurar UART e interrupciones **********
******************************************************************************/
#ifndef MAIN_UARTCONFIG_H_
#define MAIN_UARTCONFIG_H_
// Inclusión de bibliotecas ------------------------------------------------//
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "freertos/queue.h"
#define tamCOLA 2
#define tamMSN 2
// Definiciones -------------------------------------------------------------//
#define tamBUFFER 1024
// Variables globales -------------------------------------------------------//
xQueueHandle Cola_UART; // Manejador de cola para mensajes entre tareas
// Funciones prototipo ------------------------------------------------------//
void iniciarUART();
#endif
//*****************************************************************************
