#include "nvs_rotate.h"
#include <string.h>
#include "driver/gpio.h"
#include "esp_log.h"
#include "nvs_flash.h"
// Rotative nvs define

esp_err_t guarda_cuenta_pulsos(char *partition_name, char *page_namespace,
                               char *entry_key, int32_t *Pulsos) {
  nvs_handle_t my_handle;

  // Abriendo la página variable
  esp_err_t err = nvs_open_from_partition(partition_name, page_namespace,
                                          NVS_READWRITE, &my_handle);
  if (err != ESP_OK) return err;
  // Write
  err = nvs_set_i32(my_handle, entry_key, *Pulsos);
  // Commit
  err = nvs_commit(my_handle);
  if (err != ESP_OK) return err;
  // Close
  nvs_close(my_handle);

  return ESP_OK;
}

esp_err_t activa_entrada(char *partition_name, char *page_namespace,
                         char **entry_key, int32_t *actual_entry_index,
                         int32_t *Pulsos) {
  nvs_handle_t my_handle;

  free(*entry_key);
  if (asprintf(entry_key, "e%d", *actual_entry_index) <
      0) {  // Aquí se crea el keyvalue
    free(*entry_key);
    return ESP_FAIL;
  }

  // Abriendo la página variable
  esp_err_t err = nvs_open_from_partition(partition_name, page_namespace,
                                          NVS_READWRITE, &my_handle);
  if (err != ESP_OK) return err;
  // Write
  err = nvs_set_i32(my_handle, *entry_key, *Pulsos);
  // Commit
  err = nvs_commit(my_handle);
  if (err != ESP_OK) return err;
  // Close
  nvs_close(my_handle);

  return ESP_OK;
}

esp_err_t abrir_pv(char *partition_name, char **page_namespace,
                   char **entry_key, int32_t *actual_pv_counter,
                   int32_t *actual_entry_index, int32_t *Pulsos) {
  nvs_handle_t my_handle;

  esp_err_t err = nvs_open_from_partition(partition_name, STORAGE_NAMESPACE,
                                          NVS_READWRITE, &my_handle);
  if (err != ESP_OK) return err;
  // Write
  nvs_set_i32(my_handle, "pf", *actual_pv_counter);
  if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) return err;
  // Commit
  err = nvs_commit(my_handle);
  if (err != ESP_OK) return err;
  // Close
  nvs_close(my_handle);

  free(*page_namespace);
  if (asprintf(page_namespace, "pv%d", *actual_pv_counter) <
      0) {  // Aquí se crea el namespace para abrir la página
    free(*page_namespace);
    return ESP_FAIL;
  }

  activa_entrada(partition_name, *page_namespace, entry_key, actual_entry_index,
                 Pulsos);

  return ESP_OK;
}

esp_err_t leer_contador_pf(char **pname, char **pvActual,
                           int32_t *ContadorPaginaFija) {
  // Esta función lee el contador desde la pagina fija por defecto llamada
  // "storage", y se retorna a la ejecución del programa

  nvs_handle_t my_handle;
  esp_err_t err;

  ESP_LOGW("DEBUG LCPF", "%s", *pname);

  // Open
  err = nvs_open_from_partition(*pname, "storage", NVS_READWRITE, &my_handle);
  if (err != ESP_OK) return err;

  // Read
  err = nvs_get_i32(my_handle, "pf", ContadorPaginaFija);
  if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND)
    return err;
  else if (err == ESP_ERR_NVS_NOT_FOUND) {
    *ContadorPaginaFija = 1;  // Valor por defecto en lugar de no estar asignado

    // Write
    err = nvs_set_i32(my_handle, "pf", *ContadorPaginaFija);

    // Commit
    err = nvs_commit(my_handle);
    if (err != ESP_OK) return err;
  }

  // Close
  nvs_close(my_handle);
  if (asprintf(pvActual, "pv%d", *ContadorPaginaFija) <
      0) {  // Aquí se crea el namespace para abrir la página
    free(pvActual);
    return ESP_FAIL;
  }
  return ESP_OK;
}

esp_err_t leer_pagina_variable(char **pname, char **pvActual,
                               int32_t *ContadorEntradaActual,
                               char **regActualPaginaVariable,
                               int32_t *valorEntradaActual) {
  /*	*** Leer_pagina_variable ***
   *	Esta función lee la pagina variable referenciada por leer_contador_pf()
   *para encontrar la entrada más reciente escrita y su valor.
   *
   *	Entradas:
   *		pvActual: Espacio de memoria >= sizeof(char)*5 para guardar el
   *namespace de la página actual regActualPaginaVariable: Espacio de memoria >=
   *sizeof(char)*5 para guardar el keyvalue de la entrada más reciente
   *		ContadorEntradaActual,valorEntradaActual: Punteros para ser
   *rellenados
   *
   *	Retorna:
   *		ContadorEntradaActual: Número que coincide con el
   *regActualPaginaVariable pero de naturaleza int32, permite construir el
   *namespace o modificarlo en otras funciones de ser necesario
   *		regActualPaginaVariable: Keyvalue de la entrada más reciente
   *		valorEntradaActual: valor guardado en la entrada más reciente,
   *representa el último conteó de pulsos registrado
   */

  nvs_handle_t my_handle;
  int32_t entradasEnPv = 0;

  // Buscando en toda la memoria
  nvs_iterator_t it = nvs_entry_find(*pname, *pvActual, NVS_TYPE_ANY);
  while (it != NULL) {
    entradasEnPv++;
    nvs_entry_info_t info;
    nvs_entry_info(it, &info);
    it = nvs_entry_next(it);
    printf("key '%s', type '%d' \n", info.key, info.type);
  };
  nvs_release_iterator(it);

  if (entradasEnPv == 0)
    entradasEnPv = 1;  // En caso de no encontrar nada en la página actual se
                       // asigna 1 para crear la entrada e1.

  *ContadorEntradaActual = entradasEnPv;

  // Creando el key del último registro encontrado
  if (asprintf(regActualPaginaVariable, "e%d", entradasEnPv) < 0) {
    free(regActualPaginaVariable);
    return ESP_FAIL;
  }

  // Abriendo la página variable
  esp_err_t err =
      nvs_open_from_partition(*pname, *pvActual, NVS_READWRITE, &my_handle);
  if (err != ESP_OK) return err;

  // Leyendo el último registro encontrado
  err = nvs_get_i32(my_handle, *regActualPaginaVariable, valorEntradaActual);
  if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND)
    return err;
  else if (err == ESP_ERR_NVS_NOT_FOUND) {
    *valorEntradaActual = 0;  // Valor por defecto en lugar de no estar asignado
    // Write
    err = nvs_set_i32(my_handle, *regActualPaginaVariable, *valorEntradaActual);
    // Commit
    err = nvs_commit(my_handle);
    if (err != ESP_OK) return err;
  }
  // Close
  nvs_close(my_handle);

  ESP_LOGI("PV Last entry", "%d", *valorEntradaActual);
  if (*valorEntradaActual > 50000) {
    return ESP_FAIL;
  }
  return ESP_OK;
}

esp_err_t change_to_next_partition(char **pname, uint8_t *partition_number) {
  nvs_handle_t my_handle;
  esp_err_t err;
  char *aux;

  ESP_LOGI("PARTICION ACTUAL", "%s %d", *pname, *partition_number);

  // Abriendo particion y levantando la bandera
  err = nvs_open_from_partition(*pname, "storage", NVS_READWRITE, &my_handle);
  if (err != ESP_OK) {
    ESP_LOGE("NVS", "ERROR IN NVS_OPEN");
    return ESP_FAIL;
  } else {
    err = nvs_set_u8(my_handle, "finished", 1);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND)
      ESP_LOGE("NVS", "ERROR IN GET Finished Flag");
    err = nvs_commit(my_handle);
    if (err != ESP_OK) ESP_LOGE("NVS", "ERROR IN COMMIT");
  }
  // Close
  nvs_close(my_handle);

  ESP_LOGI("PARTICION ACTUAL", "DEBUG 0");

  free(*pname);

  ESP_LOGI("PARTICION ACTUAL", "DEBUG 1");

  if (*partition_number <= 3) {
    // Cerrando la particion anterior
    if (asprintf(&aux, "app%d", *partition_number - 1) < 0) {
      free(aux);
      ESP_LOGE("ROTAR_NVS", "Nombre de particion no fue creado");
    }
    printf("%s", aux);
    err = nvs_flash_deinit_partition(aux);
    if (err != ESP_OK)
      ESP_LOGE("CNP", "ERROR (%s) IN DEINIT", esp_err_to_name(err));
    else
      free(aux);

    // Inicializando la nueva partición
    if (asprintf(pname, "app%u", *partition_number) < 0) {
      free(*pname);
      ESP_LOGE("ROTAR_NVS", "Nombre de particion no fue creado");
    }
    ESP_LOGW("CNP", "Partition changed to %s", *pname);
    nvs_flash_init_partition(*pname);

    ESP_LOGI("PARTICION ACTUAL", "DEBUG 2 %s", *pname);

    // Llenando particion
    esp_err_t err =
        nvs_open_from_partition(*pname, "storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) ESP_LOGE("NVS", "ERROR IN NVS_OPEN");
    //		free(*pname);

    // Colocando la bandera de llenado en 0
    err = nvs_set_u8(my_handle, "finished", 0);
    if (err != ESP_OK) ESP_LOGE("NVS", "ERROR IN SET");
    err = nvs_commit(my_handle);
    if (err != ESP_OK) ESP_LOGE("NVS", "ERROR IN COMMIT");

    // Get del número de la partición
    err = nvs_get_u8(my_handle, "pnumber", partition_number);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND)
      ESP_LOGE("NVS", "ERROR IN GET");
    else if (err == ESP_ERR_NVS_NOT_FOUND) {
      err = nvs_set_u8(my_handle, "pnumber", *partition_number);
      if (err != ESP_OK) ESP_LOGE("NVS", "ERROR IN SET");
      err = nvs_commit(my_handle);
      if (err != ESP_OK) ESP_LOGE("NVS", "ERROR IN COMMIT");
    }
    // Close
    nvs_close(my_handle);
    return ESP_OK;
  } else {
    ESP_LOGE("APP", "All partitions full");
    return ESP_FAIL;
  }
}

esp_err_t levantar_bandera(char *pname) {
  nvs_handle_t my_handle;
  esp_err_t err;

  // Abriendo particion y levantando la bandera
  err = nvs_open_from_partition(pname, "storage", NVS_READWRITE, &my_handle);
  if (err != ESP_OK) {
    ESP_LOGE("NVS", "ERROR IN NVS_OPEN");
    return ESP_FAIL;
  } else {
    err = nvs_set_u8(my_handle, "finished", 1);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND)
      ESP_LOGE("NVS", "ERROR IN GET Finished Flag");
    err = nvs_commit(my_handle);
    if (err != ESP_OK) ESP_LOGE("NVS", "ERROR IN COMMIT");
  }
  // Close
  nvs_close(my_handle);

  ESP_LOGI("PARTICION ACTUAL", "DEBUG 1");

  return ESP_OK;
}

esp_err_t contar_pulsos_nvs(char **pname, uint8_t *partition_number,
                            int32_t *ContadorPvActual, int32_t *EntradaActual,
                            int32_t *CuentaPulsos) {
  /*	***Contar_pulsos_nvs***
   * 	Conteo de pulsos donde se aumenta la variables de numeración de pulsos,
   * entrada o página segun sea necesario.
   */

  char *key, *namespace;

  //	namespace = malloc(sizeof("pv##"));
  //	if(*namespace == 0x00) {
  //		free(namespace);
  //		return ESP_FAIL;
  //	}else sprintf(namespace, "pv%d", *ContadorPvActual);

  if (asprintf(&namespace, "pv%d", *ContadorPvActual) <
      0) {  // Aquí se crea el namespace para abrir la página
    free(namespace);
    return ESP_FAIL;
  }

  if (asprintf(&key, "e%d", *EntradaActual) < 0) {  // Aquí se crea el keyvalue
    free(key);
    return ESP_FAIL;
  }

  (*CuentaPulsos)++;

  if (*CuentaPulsos <= Limite_pulsos_por_entrada) {
    guarda_cuenta_pulsos(*pname, namespace, key,
                         CuentaPulsos);  // Guarda cada pulso

  } else {
    (*EntradaActual)++;  // Límite de pulsos por entrada superado, cambiando a
                         // siguiente entrada
    if (*EntradaActual <= Limite_entradas_por_pagina) {
      *CuentaPulsos = 1;
      activa_entrada(*pname, namespace, &key, EntradaActual, CuentaPulsos);

    } else {
      (*ContadorPvActual)++;  // Límite de entradas por pagina superado se debe
                              // cambiar de página.
      if (*ContadorPvActual <= Limite_paginas_por_particion) {
        *EntradaActual = 1;
        *CuentaPulsos = 1;
        abrir_pv(*pname, &namespace, &key, ContadorPvActual, EntradaActual,
                 CuentaPulsos);

      } else {
        if (*partition_number <= max_particiones) {
          *ContadorPvActual = 1;
          *EntradaActual = 1;
          *CuentaPulsos = 1;
        } else
          ESP_LOGE("PARTITIONS", "All partitions are full");
      }
    }
  }
  free(key);
  free(namespace);
  return ESP_OK;
}

esp_err_t search_init_partition(uint8_t *pnumber) {
  char *pname;
  nvs_stats_t info;
  nvs_handle_t my_handle;
  uint8_t partition_full;
  esp_err_t err;

  *pnumber = 0;

  for (uint8_t i = 1; i <= 3; i++) {
    if (asprintf(&pname, "app%d", i) < 0) {
      free(pname);
      ESP_LOGE("ROTAR_NVS", "Nombre de particion no fue creado");
      return ESP_FAIL;
    }

    ESP_LOGE("ROTAR_NVS", "%s", pname);
    err = nvs_flash_init_partition(pname);
    if (err == ESP_ERR_NVS_NO_FREE_PAGES ||
        err == ESP_ERR_NVS_NEW_VERSION_FOUND ||
        gpio_get_level(GPIO_NUM_0) == 0) {
      // NVS partition was truncated and needs to be erased
      // Retry nvs_flash_init
      ESP_ERROR_CHECK(nvs_flash_erase_partition("app1"));
      ESP_ERROR_CHECK(nvs_flash_erase_partition("app2"));
      ESP_ERROR_CHECK(nvs_flash_erase_partition("app3"));
      ESP_ERROR_CHECK(nvs_flash_init_partition(pname));
    }

    err = nvs_get_stats(pname, &info);
    if (err == ESP_OK)
      ESP_LOGE("NVS INFO",
               "\n Total entries: %d\n Used entries:%d\n Free entries: %d\n "
               "Namespace count: %d",
               info.total_entries, info.used_entries, info.free_entries,
               info.namespace_count);

    // Revisando si la partición está llena
    esp_err_t err =
        nvs_open_from_partition(pname, "storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) ESP_LOGE("NVS", "ERROR IN NVS_OPEN");

    err = nvs_get_u8(my_handle, "finished", &partition_full);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND)
      ESP_LOGE("NVS", "ERROR (%s) IN GET", esp_err_to_name(err));
    else if (err == ESP_ERR_NVS_NOT_FOUND) {
      partition_full = 0;  // Valor por defecto en lugar de no estar asignado
      // Write
      err = nvs_set_u8(my_handle, "finished", partition_full);
      // Commit
      err = nvs_commit(my_handle);
      if (err != ESP_OK) ESP_LOGE("NVS", "ERROR IN COMMIT");
    }

    // Seteando el número de la partición
    err = nvs_get_u8(my_handle, "pnumber", pnumber);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND)
      ESP_LOGE("NVS", "ERROR IN GET");
    else if (err == ESP_ERR_NVS_NOT_FOUND) {
      // Write
      err = nvs_set_u8(my_handle, "pnumber", i);
      ESP_LOGW("SIP", "Partition number setted first time: %d", i);
      // Commit
      err = nvs_commit(my_handle);
      if (err != ESP_OK) ESP_LOGE("NVS", "ERROR IN COMMIT");
    } else {
      ESP_LOGW("DEBUG", "Partition number setted: %u", *pnumber);
    }

    // Close
    nvs_close(my_handle);
    nvs_flash_deinit_partition(pname);
    if (*pnumber == i) {
      ESP_LOGI("SIP", "OK");
    }

    if (partition_full == 1) {
      ESP_LOGW("SIP", "Partition number %d is full, trying with next", i);
      if (i == 3) {
        ESP_LOGE("SIP", "All partitions are full");
        return ESP_FAIL;
      }
    } else if (partition_full == 0) {
      ESP_LOGW("SIP", "NVS init can be done in partition app%d", i);
      *pnumber = i;
      break;
    }
  }
  return ESP_OK;
}
