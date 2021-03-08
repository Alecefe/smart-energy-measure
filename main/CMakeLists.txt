idf_component_register(SRCS "esp_rest_main.c" 
                            "rest_server.c"
                    INCLUDE_DIRS ".")

idf_component_register(SRCS "https_mbedtls_example_main.c"
                    INCLUDE_DIRS "."
                    EMBED_TXTFILES server_root_cert.pem)
idf_component_register(SRCS "sntp_example_main.c"
INCLUDE_DIRS ".")

if(CONFIG_EXAMPLE_WEB_DEPLOY_SF)
    set(WEB_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../front/web-demo")
    if(EXISTS ${WEB_SRC_DIR}/dist)
        spiffs_create_partition_image(www ${WEB_SRC_DIR}/dist FLASH_IN_PROJECT)
    else()
        message(FATAL_ERROR "${WEB_SRC_DIR}/dist doesn't exit. Please run 'npm run build' in ${WEB_SRC_DIR}")
    endif()
endif()
