#include <stdint.h>
#include "esp_err.h"
#define STORAGE_NAMESPACE "storage"
#define Limite_pulsos_por_entrada 10
#define Limite_entradas_por_pagina 4
#define Limite_paginas_por_particion 3
#define max_particiones 3

esp_err_t guarda_cuenta_pulsos(char *partition_name, char *page_namespace,
                               char *entry_key, int32_t *Pulsos);
esp_err_t activa_entrada(char *partition_name, char *page_namespace,
                         char **entry_key, int32_t *actual_entry_index,
                         int32_t *Pulsos);
esp_err_t abrir_pv(char *partition_name, char **page_namespace,
                   char **entry_key, int32_t *actual_pv_counter,
                   int32_t *actual_entry_index, int32_t *Pulsos);
esp_err_t leer_contador_pf(char **pname, char **pvActual,
                           int32_t *ContadorPaginaFija);
esp_err_t leer_pagina_variable(char **pname, char **pvActual,
                               int32_t *ContadorEntradaActual,
                               char **regActualPaginaVariable,
                               int32_t *valorEntradaActual);
esp_err_t change_to_next_partition(char **pname, uint8_t *partition_number);
esp_err_t levantar_bandera(char *pname);
esp_err_t contar_pulsos_nvs(char **pname, uint8_t *partition_number,
                            int32_t *ContadorPvActual, int32_t *EntradaActual,
                            int32_t *CuentaPulsos);
esp_err_t search_init_partition(uint8_t *pnumber);
