PROJECT_NAME:=webhttp

include $(IDF_PATH)/make/project.mk

WEB_SRC_DIR = $(shell pwd)/main/front
SPIFFS_IMAGE_FLASH_IN_PROJECT := 1
$(eval $(call spiffs_create_partition_image,www,$(WEB_SRC_DIR)))
