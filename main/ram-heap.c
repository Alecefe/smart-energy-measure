#include <ram-heap.h>

void show_ram_status(char* info){
	multi_heap_info_t multiheap;
	printf("\r\n");
	ESP_LOGI("RAM","%s",info);
	heap_caps_get_info(&multiheap, MALLOC_CAP_INTERNAL);
	ESP_LOGW("RAM-INTERNAL","Free heap: %d Allocated bytes: %d", multiheap.total_free_bytes, multiheap.total_allocated_bytes);
//	heap_caps_get_info(&multiheap, MALLOC_CAP_EXEC);
//	ESP_LOGW("RAM-EXEC","Free heap: %d Allocated bytes: %d", multiheap.total_free_bytes, multiheap.total_allocated_bytes);
//	heap_caps_get_info(&multiheap, MALLOC_CAP_32BIT);
//	ESP_LOGW("RAM-32b","Free heap: %d Allocated bytes: %d", multiheap.total_free_bytes, multiheap.total_allocated_bytes);
//	heap_caps_get_info(&multiheap, MALLOC_CAP_8BIT);
//	ESP_LOGW("RAM-8b","Free heap: %d Allocated bytes: %d", multiheap.total_free_bytes, multiheap.total_allocated_bytes);
//	heap_caps_get_info(&multiheap, MALLOC_CAP_DMA);
//	ESP_LOGW("RAM-DMA","Free heap: %d Allocated bytes: %d", multiheap.total_free_bytes, multiheap.total_allocated_bytes);
	heap_caps_get_info(&multiheap, MALLOC_CAP_DEFAULT);
	ESP_LOGW("RAM-DEFAULT","Free heap: %d Allocated bytes: %d", multiheap.total_free_bytes, multiheap.total_allocated_bytes);
	printf("\r\n");
}
