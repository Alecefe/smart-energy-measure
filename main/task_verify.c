#include "task_verify.h"
#include <string.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#define TAG "TASK VERIFY"
bool vTaskB(char *nombre_tarea) {
  TaskStatus_t *pxTaskStatusArray;
  UBaseType_t uxArraySize;
  uint32_t ulTotalRunTime;

  // Take a snapshot of the number of tasks in case it changes while this
  // function is executing.
  uxArraySize = uxTaskGetNumberOfTasks();

  // Allocate a TaskStatus_t structure for each task.  An array could be
  // allocated statically at compile time.
  pxTaskStatusArray = pvPortMalloc(uxArraySize * sizeof(TaskStatus_t));

  if (pxTaskStatusArray != NULL) {
    // Generate raw status information about each task.
    uxTaskGetSystemState(pxTaskStatusArray, uxArraySize, &ulTotalRunTime);
    for (int i = 0; i < uxArraySize; i++) {
      // ESP_LOGW(MESH_TAG,"Tarea %d %s",i,pxTaskStatusArray[i].pcTaskName);
      if (strcmp(pxTaskStatusArray[i].pcTaskName, nombre_tarea) == 0) {
        printf("Tarea %s, numero %d ya existe\r\n",
               pxTaskStatusArray[i].pcTaskName, i);
        return false;
      }
    }
  }
  ESP_LOGW(TAG, "Creando Tarea: %s", (char *)nombre_tarea);
  return true;
}
