#ifndef MAIN_INCLUDE_UARTACCESO_H_
#define MAIN_INCLUDE_UARTACCESO_H_

#include "UARTconfig.h"
#include "WiFiconfig.h"
#include "webHTTP.h"
#define usuario "1"
#define pase	"1"

enum estado{
	msn_usuario=1,
	espera_usuario,
	msn_clave,
	espera_clave,
	verificacion,
};
void tarea1(void* P);
#endif /* MAIN_INCLUDE_UARTACCESO_H_ */
