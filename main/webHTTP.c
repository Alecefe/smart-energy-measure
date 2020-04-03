/*********************************************************************************/
#include "include/webHTTP.h"
#include "esp_log.h"
#include "math.h"
const char *nvs_tag = "NVS";
const char *http_server = "HTTP SERVER";
static const char *REST_TAG = "esp-rest";
static const char *TAG = "REQUEST";
const char valid_user[]="usuario";
const char valid_pass[]="contrasena";

#define REST_CHECK(a, str, goto_tag, ...)                                              \
    do                                                                                 \
    {                                                                                  \
        if (!(a))                                                                      \
        {                                                                              \
            ESP_LOGE(REST_TAG, "%s(%d): " str, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
            goto goto_tag;                                                             \
        }                                                                              \
    } while (0)

#define FILE_PATH_MAX (ESP_VFS_PATH_MAX + 128)
#define SCRATCH_BUFSIZE (10240)

bool ahora = true;
conversion_t conversion[4] = {{rs485,"rs485"},{pulsos,"pulsos"},{chino,"chino"},{enlace,"enlace"}};
uint8_t hola_mundo;
char ssid[20] = "M3M0";
char password[20] = "12050808";
uint8_t meshID[6] = {0x77,0x77,0x77,0x77,0x77,0x77};
char mesh_password[20] = "-Capacho10";
uint8_t max_layer = 12;
uint8_t max_sta = 8;
uint16_t port = 8080;

typedef struct rest_server_context {
    char base_path[ESP_VFS_PATH_MAX + 1];
    char scratch[SCRATCH_BUFSIZE];
} rest_server_context_t;


tipo_de_medidor str2enum (const char *str)
{    int j;
     for (j = 0;  j < sizeof (conversion) / sizeof (conversion[0]);  ++j)
         if (!strcmp (str, conversion[j].str))
             return conversion[j].val;
     ESP_LOGI(http_server,"Tipo no valido");
     return enlace;
}

#define CHECK_FILE_EXTENSION(filename, ext) (strcasecmp(&filename[strlen(filename) - strlen(ext)], ext) == 0)

/* Set HTTP response content type according to file extension */
static esp_err_t set_content_type_from_file(httpd_req_t *req, const char *filepath)
{
    const char *type = "text/plain";
    if (CHECK_FILE_EXTENSION(filepath, ".html")) {
        type = "text/html";
    }else if (CHECK_FILE_EXTENSION(filepath, ".js")) {
        type = "application/javascript";
    }else if (CHECK_FILE_EXTENSION(filepath, ".css")) {
        type = "text/css";
    } else if (CHECK_FILE_EXTENSION(filepath, ".png")) {
        type = "image/png";
    } else if (CHECK_FILE_EXTENSION(filepath, ".ico")) {
        type = "image/x-icon";
    } else if (CHECK_FILE_EXTENSION(filepath, ".svg")) {
        type = "text/xml";
    }
    return httpd_resp_set_type(req, type);
}

/* Send HTTP response with the contents of the requested file */
static esp_err_t rest_common_get_handler(httpd_req_t *req)
{
    char filepath[FILE_PATH_MAX];

    ESP_LOGI(TAG,"req->uri: %s",req->uri);
    bool pasa = true;

    char foobar[7] = "/foobar";
    for(int i = 0; i < sizeof(foobar);i++){
    	if(foobar[i]!=req->uri[i]){
    		pasa=false;
    	}
    }

    rest_server_context_t *rest_context = (rest_server_context_t *)req->user_ctx;
    strlcpy(filepath, rest_context->base_path, sizeof(filepath));

    if (req->uri[strlen(req->uri) - 1] == '/' || pasa) {
        strlcat(filepath, "/index.html", sizeof(filepath));
    } else {
        strlcat(filepath, req->uri, sizeof(filepath));
    }

    int fd = open(filepath, O_RDONLY, 0);
    if (fd == -1) {
        ESP_LOGE(REST_TAG, "Failed to open file : %s", filepath);
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to read existing file");
        return ESP_FAIL;
    }

    set_content_type_from_file(req, filepath);

    char *chunk = rest_context->scratch;
    ssize_t read_bytes;
    do {
        /* Read file in chunks into the scratch buffer */
        read_bytes = read(fd, chunk, SCRATCH_BUFSIZE);
        if (read_bytes == -1) {
            ESP_LOGE(REST_TAG, "Failed to read file : %s", filepath);
        } else if (read_bytes > 0) {
            /* Send the buffer contents as HTTP response chunk */
            if (httpd_resp_send_chunk(req, chunk, read_bytes) != ESP_OK) {
                close(fd);
                ESP_LOGE(REST_TAG, "File sending failed!");
                /* Abort sending file */
                httpd_resp_sendstr_chunk(req, NULL);
                /* Respond with 500 Internal Server Error */
                httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to send file");
                return ESP_FAIL;
            }
        }
    } while (read_bytes > 0);
    /* Close file after sending complete */
    close(fd);
    ESP_LOGI(REST_TAG, "File sending complete");
    /* Respond with an empty chunk to signal HTTP response completion */
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

uint16_t get_real_size(char* arg){
	uint16_t result = 0;
	for(int i = 0; arg[i]!=0x00; i++){
		result++;
	}
	return result;
}

void save_data(char *arg, char* where){
	uint16_t cuanto = get_real_size(arg);

	for(int i = 0; i<cuanto; i++){
		where[i]=arg[i];
		ESP_LOGI("DEBUG","%c",where[i]);
	}
}


static esp_err_t form_mesh_req_handler(httpd_req_t *req){
//Simulador de datos en flash aqui iria funcion de tomar datos de la flash

	//ESP_LOGI("DEBUG","DENTRO DE FUNCION");

    int total_len = req->content_len;
    int cur_len = 0;
    char *buf = ((rest_server_context_t *)(req->user_ctx))->scratch;
    int received = 0;
    if (total_len >= SCRATCH_BUFSIZE) {
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "content too long");
        return ESP_FAIL;
    }
    while (cur_len < total_len) {
        received = httpd_req_recv(req, buf + cur_len, total_len);
        if (received <= 0) {
            /* Respond with 500 Internal Server Error */
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to post control value");
            return ESP_FAIL;
        }
        cur_len += received;
    }
    buf[total_len] = '\0';

    ESP_LOGI("DEBUG","BUFFER: %s",buf);

    cJSON *root = cJSON_Parse(buf);
    char *aux_ssid = cJSON_GetObjectItem(root, "ssid")->valuestring;
    char *aux_password = cJSON_GetObjectItem(root, "password")->string;
    char *aux_meshID = cJSON_GetObjectItem(root, "meshID")->string;
    char* aux_mesh_password = cJSON_GetObjectItem(root, "mesh_password")->string;
    max_layer = cJSON_GetObjectItem(root, "max_layer")->valueint;
    max_sta = cJSON_GetObjectItem(root, "max_sta")->valueint;
    port = cJSON_GetObjectItem(root, "port")->valueint;
    ESP_LOGI("JSON","SSID: %s, PASSWORD: %s, MESH_ID: %s, MESH_PASSWORD: %s, MAX_LAYER: %d,MAX_STA: %d, PORT: %d",aux_ssid,aux_password,aux_meshID,aux_mesh_password,max_layer,max_sta,port);
	cJSON_Delete(root);
    httpd_resp_sendstr(req, "Post control value successfully");
    return ESP_OK;
}

static esp_err_t login_req_handler(httpd_req_t *req){
//Simulador de datos en flash aqui iria funcion de tomar datos de la flash

	//ESP_LOGI("DEBUG","DENTRO DE FUNCION");

    int total_len = req->content_len;
    int cur_len = 0;
    char *buf = ((rest_server_context_t *)(req->user_ctx))->scratch;
    int received = 0;
    if (total_len >= SCRATCH_BUFSIZE) {
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "content too long");
        return ESP_FAIL;
    }
    while (cur_len < total_len) {
        received = httpd_req_recv(req, buf + cur_len, total_len);
        if (received <= 0) {
            /* Respond with 500 Internal Server Error */
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to post control value");
            return ESP_FAIL;
        }
        cur_len += received;
    }
    buf[total_len] = '\0';

    ESP_LOGI("DEBUG","BUFFER: %s",buf);

    cJSON *root = cJSON_Parse(buf);

    char* aux_usuario = cJSON_GetObjectItem(root,"usuario")->valuestring;
    char* aux_contrasena = cJSON_GetObjectItem(root,"contrasena")->valuestring;
    if( (strlen(aux_usuario)==strlen(valid_user))&&(strlen(aux_contrasena)==strlen(valid_pass))){
    	if((strcmp(aux_usuario, valid_user)==0)&&(strcmp(aux_contrasena, valid_pass)==0)){
			httpd_resp_sendstr(req, "valido");
		}else{
			httpd_resp_sendstr(req, "content no valido");
		}
    }else{
    	httpd_resp_sendstr(req, "long no valido");
    }
    //const char *sys_info = cJSON_Print(root);
	cJSON_Delete(root);

    return ESP_OK;
}

static esp_err_t form_mesh_get_handler(httpd_req_t *req){
//Simulador de datos en flash aqui iria funcion de tomar datos de la flash

	ESP_LOGI(TAG,"req->uri: %s",req->uri);

	httpd_resp_set_status(req, HTTPD_200);
	httpd_resp_set_type(req, HTTPD_TYPE_JSON);

	 char mesh_id [18] = {0,};
	 sprintf(mesh_id,MACSTR,MAC2STR(meshID));

	 cJSON *root = cJSON_CreateObject();
	 cJSON_AddStringToObject(root, "ssid", ssid);
	 cJSON_AddStringToObject(root, "password", password);
	 cJSON_AddStringToObject(root, "meshID", mesh_id);
	 cJSON_AddStringToObject(root, "mesh_password", mesh_password);
	 cJSON_AddNumberToObject(root, "max_layer", max_layer);
	 cJSON_AddNumberToObject(root, "max_sta", max_sta);
	 cJSON_AddNumberToObject(root, "port", port);
	 const char *sys_info = cJSON_Print(root);
	 cJSON_Delete(root);
	 ESP_LOGI("JSON","%s",sys_info);

	 uint16_t cuanto = strlen(sys_info);
	 ESP_LOGI("DEBUG","%d",cuanto);

	 httpd_resp_send(req, sys_info, strlen(sys_info));

	return ESP_OK;
}


esp_err_t start_rest_server(const char *base_path)
{
	/* Validate file storage base path */
	if (!base_path || strcmp(base_path, CONFIG_EXAMPLE_WEB_MOUNT_POINT) != 0) {
		ESP_LOGE(REST_TAG, "File server presently supports only '/spiffs' as base path");
		return ESP_ERR_INVALID_ARG;
	}

    rest_server_context_t *rest_context = calloc(1, sizeof(rest_server_context_t));
    REST_CHECK(rest_context, "No memory for rest context", err);

    strlcpy(rest_context->base_path, base_path, sizeof(rest_context->base_path));

    httpd_handle_t server = NULL;
    httpd_config_t config = {
    		.task_priority      = tskIDLE_PRIORITY+5,       \
			.stack_size         = 1024*20,                  \
			.core_id            = tskNO_AFFINITY,           \
			.server_port        = 80,                       \
			.ctrl_port          = 32768,                    \
			.max_open_sockets   = 7,                        \
			.max_uri_handlers   = 8,                        \
			.max_resp_headers   = 8,                        \
			.backlog_conn       = 5,                        \
			.lru_purge_enable   = false,                    \
			.recv_wait_timeout  = 5,                        \
			.send_wait_timeout  = 5,                        \
			.global_user_ctx = NULL,                        \
			.global_user_ctx_free_fn = NULL,                \
			.global_transport_ctx = NULL,                   \
			.global_transport_ctx_free_fn = NULL,           \
			.open_fn = NULL,                                \
			.close_fn = NULL,                               \
			.uri_match_fn = NULL                            \

    };
    config.uri_match_fn = httpd_uri_match_wildcard;

    ESP_LOGI(REST_TAG, "Starting HTTP Server");
    REST_CHECK(httpd_start(&server, &config) == ESP_OK, "Start server failed", err_start);

    /* URI handler for getting web server files */
	httpd_uri_t mesh_get_uri = {
		.uri = "/formMesh",
		.method = HTTP_GET,
		.handler = form_mesh_get_handler,
		.user_ctx = rest_context
	};
	httpd_register_uri_handler(server, &mesh_get_uri);

	/* URI handler for getting web server files */
	httpd_uri_t mesh_form_uri = {
		.uri = "/req_mesh",
		.method = HTTP_POST,
		.handler = form_mesh_req_handler,
		.user_ctx = rest_context
	};
	httpd_register_uri_handler(server, &mesh_form_uri);

	/* URI handler for getting web server files */
	httpd_uri_t login_uri = {
		.uri = "/login",
		.method = HTTP_POST,
		.handler = login_req_handler,
		.user_ctx = rest_context
	};
	httpd_register_uri_handler(server, &login_uri);

    /* URI handler for getting web server files */
    httpd_uri_t common_get_uri = {
        .uri = "/*",
        .method = HTTP_GET,
        .handler = rest_common_get_handler,
        .user_ctx = rest_context
    };
    httpd_register_uri_handler(server, &common_get_uri);


    return ESP_OK;
err_start:
    free(rest_context);
err:
    return ESP_FAIL;
}

void set_form_flash_init(struct form_home form){

	esp_err_t err;
	nvs_handle_t ctrl_flash;
	err = nvs_open("storage",NVS_READWRITE,&ctrl_flash);
	if (err != ESP_OK) {
		printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
	}else{

		nvs_set_str(ctrl_flash,"ssid",form.ssid);

		nvs_set_str(ctrl_flash,"password",form.password);

		nvs_set_blob(ctrl_flash,"meshid",form.mesh_id,sizeof(form.mesh_id));

		nvs_set_str(ctrl_flash,"meshpass",form.meshappass);

		nvs_set_u8(ctrl_flash,"max_layer",form.max_layer);

		nvs_set_u8(ctrl_flash,"max_sta",form.max_sta);

		nvs_set_u16(ctrl_flash,"port",form.port);

		err = nvs_commit(ctrl_flash);
	}
	nvs_close(ctrl_flash);
}

void set_form_flash_modbus(struct form_home form){
	esp_err_t err;
	nvs_handle_t ctrl_flash;
	tipo_de_medidor tipo;
	err = nvs_open("storage",NVS_READWRITE,&ctrl_flash);
	if (err != ESP_OK) {
		printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
	}else{
		nvs_set_str(ctrl_flash,"tipo",form.tipo);

		tipo = str2enum(form.tipo);
		switch(tipo){

			case(rs485):
					nvs_set_u32(ctrl_flash,"baud",form.baud_rate);
			break;
			case(pulsos):
					nvs_set_u64(ctrl_flash,"energy",form.energia);

					nvs_set_u8(ctrl_flash,"slaveid",form.slaveid);

					nvs_set_u16(ctrl_flash,"conver",form.conversion);
			break;
			case(chino):
					nvs_set_u32(ctrl_flash,"baud",form.baud_rate);
			break;
			case(enlace):
			break;

			}
		err = nvs_commit(ctrl_flash);
	}
	nvs_close(ctrl_flash);
}

void get_form_flash(struct form_home *form){
	size_t len;
	char mac[18];
	esp_err_t err;
	nvs_handle_t ctrl_flash, ctrl_prueba;

	err = nvs_open("storage",NVS_READWRITE,&ctrl_flash);
	if (err != ESP_OK) {
		printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
	}else{
		err = nvs_get_str(ctrl_flash,"ssid",NULL,&len);
		if(err==ESP_OK) {
			err = nvs_get_str(ctrl_flash,"ssid",form->ssid,&len);
			switch(err){
				case ESP_OK:
					ESP_LOGI(nvs_tag,"SSID en flash: %s",form->ssid);
				break;
				case ESP_ERR_NVS_NOT_FOUND:
					ESP_LOGI(nvs_tag,"SSID en flash: none");
				break;
				default:
					printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
				break;
			}
		}

		err = nvs_get_str(ctrl_flash,"password",NULL,&len);
		if(err==ESP_OK){
			err= nvs_get_str(ctrl_flash,"password",form->password,&len);
		switch(err){
			case ESP_OK:
				ESP_LOGI(nvs_tag,"Password en flash: %s",form->password);
			break;
			case ESP_ERR_NVS_NOT_FOUND:
				ESP_LOGI(nvs_tag,"Password en flash: none");
			break;
			default:
				printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
			break;
		}
		}
		err = nvs_get_blob(ctrl_flash,"meshid",NULL,&len);
		if(err==ESP_OK){
		err = nvs_get_blob(ctrl_flash,"meshid",form->mesh_id,&len);
		switch(err){
			case ESP_OK:
				sprintf(mac,MACSTR,MAC2STR(form->mesh_id));
				ESP_LOGI(nvs_tag,"Mesh id en flash: %s",mac);
			break;
			case ESP_ERR_NVS_NOT_FOUND:
				ESP_LOGI(nvs_tag,"Mesh id en flash: none");
			break;
			default:
				printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
			break;
		}
		}
	}nvs_close(ctrl_flash);

	err = nvs_open("storage",NVS_READWRITE,&ctrl_prueba);

		if (err != ESP_OK) {
			printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
		}else{
			err = nvs_get_str(ctrl_prueba,"meshpass",NULL,&len);
			if(err==ESP_OK){
				err= nvs_get_str(ctrl_prueba,"meshpass",form->meshappass,&len);
			switch(err){
				case ESP_OK:
					ESP_LOGI(nvs_tag,"Mesh password en flash: %s",form->meshappass);
				break;
				case ESP_ERR_NVS_NOT_FOUND:
					ESP_LOGI(nvs_tag,"Mesh password en flash: none");
				break;
				default:
					printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
				break;
			}
			}
			err = nvs_get_u8(ctrl_prueba,"max_layer",&(form->max_layer));
			switch(err){
				case ESP_OK:
					ESP_LOGI(nvs_tag,"Max. Layer en flash: %d",form->max_layer);
				break;
				case ESP_ERR_NVS_NOT_FOUND:
					ESP_LOGI(nvs_tag,"Max. Layer en flash: none");
				break;
				default:
					printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
				break;
			}
			err = nvs_get_u8(ctrl_prueba,"max_sta",&(form->max_sta));
			switch(err){
				case ESP_OK:
					ESP_LOGI(nvs_tag,"Max. Sta en flash: %d",form->max_sta);
				break;
				case ESP_ERR_NVS_NOT_FOUND:
					ESP_LOGI(nvs_tag,"Max. Sta en flash: none");
				break;
				default:
					printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
				break;
			}
			err = nvs_get_u16(ctrl_prueba,"port",&(form->port));
			switch(err){
				case ESP_OK:
					ESP_LOGI(nvs_tag,"Port en flash: %d",form->port);
				break;
				case ESP_ERR_NVS_NOT_FOUND:
					ESP_LOGI(nvs_tag,"Port en flash: none");
				break;
				default:
					printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
				break;
			}
			err = nvs_get_str(ctrl_prueba,"tipo",NULL,&len);
			if(err==ESP_OK){
				err= nvs_get_str(ctrl_prueba,"tipo",form->tipo,&len);
			switch(err){
				case ESP_OK:
					ESP_LOGI(nvs_tag,"Tipo en flash: %s",form->tipo);
				break;
				case ESP_ERR_NVS_NOT_FOUND:
					ESP_LOGI(nvs_tag,"Tipo en flash: none");
				break;
				default:
					printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
				break;
			}
			}
			switch(str2enum(form->tipo)){

			case(rs485):
					err = nvs_get_u32(ctrl_prueba,"baud",&(form->baud_rate));
					switch(err){
						case ESP_OK:
							ESP_LOGI(nvs_tag,"Baud Rate en flash: %"PRIu32,form->baud_rate);
						break;
						case ESP_ERR_NVS_NOT_FOUND:
							ESP_LOGI(nvs_tag,"Baud Rate en flash: none");
						break;
						default:
							printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
						break;
					}
					break;
			case(pulsos):
					err = nvs_get_u64(ctrl_prueba,"energy",&(form->energia));
					switch(err){
						case ESP_OK:
							ESP_LOGI(nvs_tag,"Pulsos en flash: %"PRIu64,form->energia);
						break;
						case ESP_ERR_NVS_NOT_FOUND:
							ESP_LOGI(nvs_tag,"Pulsos en flash: none");
						break;
						default:
							printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
						break;
					}
					err = nvs_get_u8(ctrl_prueba,"slaveid",&(form->slaveid));
					switch(err){
						case ESP_OK:
							ESP_LOGI(nvs_tag,"Slave ID en medidor a pulsos en flash: %d",form->slaveid);
						break;
						case ESP_ERR_NVS_NOT_FOUND:
							ESP_LOGI(nvs_tag,"Slave ID en flash: none");
						break;
						default:
							printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
						break;
					}

					err = nvs_get_u16(ctrl_prueba,"conver",&(form->conversion));
					switch(err){
						case ESP_OK:
							ESP_LOGI(nvs_tag,"Factor de conversion en flash: %d",form->conversion);
						break;
						case ESP_ERR_NVS_NOT_FOUND:
							ESP_LOGI(nvs_tag,"Factor de conversion en flash: none");
						break;
						default:
							printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
						break;
					}
					break;
				case(chino):
					err = nvs_get_u32(ctrl_prueba,"baud",&(form->baud_rate));
					switch(err){
						case ESP_OK:
							ESP_LOGI(nvs_tag,"Baud Rate en flash: %"PRIu32,form->baud_rate);
						break;
						case ESP_ERR_NVS_NOT_FOUND:
							ESP_LOGI(nvs_tag,"Baud Rate en flash: none");
						break;
						default:
							printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
						break;
					}
					break;
					case(enlace):
							break;
					default:
						break;
			}
	}
	nvs_close(ctrl_prueba);
}

bool Llenar_intro(char *p){
	char *ini;
	char usuario[9]="mesh+user";
	char password[9]="polilla22";

	ini = strstr(p,"usuario=");
	if(ini!=NULL){
	ini+=sizeof("usuario=")-1;
		for(int i = 0;ini[i]!='&';i++){

			if(ini[i]!=usuario[i]){
				return false;
			}
		}
	}else{printf("nada\r\n");}

	ini = strstr(p,"contrasena=");
	if(ini!=NULL){
	ini+=sizeof("contrasena=")-1;
		for(int i = 0;ini[i]!='&';i++){

				if(ini[i]!=password[i]){

					return false;
				}
			}
	}else{printf("nada\r\n");}
	return true;
	}

bool Llenar_form_home(char * p, struct form_home form1){
	char *ini,aux,cono[2];
	int count;

	/*Extrayendo SSID*/
    ini=strstr(p,"ssid=");
	if(ini!=NULL){
		count = 0;
		for(int i =0;i<20;i++){
			form1.ssid[i]=0x00;
		}
		ini+=sizeof("ssid=")-1;
		for(int i=0; ini[i]!='&';i++){
			if(strncmp(&ini[i],"%",1)==0){
				cono[0]=ini[i+1];
				cono[1]=ini[i+2];
				if(cono[0]>0x40&&cono[0]<0x47){
					cono[0]-= 0x37;
				}
				if(cono[1]>0x40&&cono[1]<0x47){
					cono[1]-=0x37;
				}
				aux = ((0x0f&cono[0])<<4)|(0x0f&cono[1]);
				form1.ssid[count]=aux;
				i+=2;
				count++;
			}else{
				form1.ssid[count]=ini[i];
				count++;
				}
		}
		if (count == 0) return false;
		printf("Usuario: %s\r\n",form1.ssid);
	}
	/*Extrayendo Password WIFI*/
    ini=strstr(p,"contrasena=");
	if(ini!=NULL){
		count=0;
		for(int i =0;i<20;i++){
			form1.password[i]=0x00;
		}
		ini+=sizeof("contrasena=")-1;
		for(int i=0; ini[i]!='&';i++){
			if(strncmp(&ini[i],"%",1)==0){
				cono[0]=ini[i+1];
				cono[1]=ini[i+2];
				if(cono[0]>0x40&&cono[0]<0x47){
					cono[0]-= 0x37;
				}
				if(cono[1]>0x40&&cono[1]<0x47){
					cono[1]-=0x37;
				}
				aux = ((0x0f&cono[0])<<4)|(0x0f&cono[1]);
				form1.password[count]=aux;
				i+=2;
				count++;
			}else{
				form1.password[count]=ini[i];
				count++;}
		}
		if (count == 0) return false;
		printf("Contrasena: %s\r\n",form1.password);
	}
	/*Extrayendo Mesh ID   */
    ini=strstr(p,"meshID=");
	if(ini!=NULL){
		count=0;
		ini+=sizeof("meshID=")-1;
		for(int i=0; ini[i]!='&';i+=2){
			if(ini[i]=='%'){
				i++;
			}else{
				if(ini[i]>0x60){
					ini[i]-= 0x57;
				}
				if(ini[i+1]>0x60){
					ini[i+1]-=0x57;
				}
				aux = ((0x0f&ini[i])<<4)|(0x0f&ini[i+1]);
				form1.mesh_id[count]=aux;
				count++;
			}
		}
		if (count == 0) return false;
		printf("meshID:"MACSTR"\r\n",MAC2STR(form1.mesh_id));
	}
	/*Extrayendo Mesh AP Password*/
    ini=strstr(p,"meshAPpass=");
	if(ini!=NULL){
		count=0;
		for(int i =0;i<20;i++){
			form1.meshappass[i]=0x00;
		}
		ini+=sizeof("meshAPpass=")-1;
		for(int i=0; ini[i]!='&';i++){
			if(strncmp(&ini[i],"%",1)==0){
				cono[0]=ini[i+1];
				cono[1]=ini[i+2];
				if(cono[0]>0x40&&cono[0]<0x47){
					cono[0]-= 0x37;
				}
				if(cono[1]>0x40&&cono[1]<0x47){
					cono[1]-=0x37;
				}
				aux = ((0x0f&cono[0])<<4)|(0x0f&cono[1]);
				form1.meshappass[count]=aux;
				i+=2;
				count++;
			}else{
				form1.meshappass[count]=ini[i];
				count++;}
		}
		if (count == 0) return false;
		printf("Mesh AP Password: %s\r\n",form1.meshappass);
	}


	/*Extrayendo Layers*/
    ini=strstr(p,"layers=");
	if(ini!=NULL){
		ini+=sizeof("layers=")-1;
		aux = ((*ini)-'0')*10+(*(ini+1)-'0');
		if(aux<25) {
			form1.max_layer=aux;
			printf("Max. Layer: %d\r\n",form1.max_layer);
		}else return false;
	}
	/*Extrayendo STA*/
	    ini=strstr(p,"estaciones=");
		if(ini!=NULL){
			ini+=sizeof("estaciones=")-1;
			aux = *(ini)-'0';
			if(aux<=9) {
				form1.max_sta=aux;
				printf("Max. sta: %d\r\n",form1.max_sta);
			}else return false;
		}
		/*Extrayendo PORT*/
		ini=strstr(p,"port=");

		if(ini!=NULL){
			int aux1;
			char puerto[5];
			ini+=sizeof("port=")-1;
			for(int i = 0; ini[i]!='&';i++){
				puerto[i]=ini[i];
			}
			aux1 = atoi(puerto);
			if(aux1<=65536) {
				form1.port=(uint16_t)aux1;
				printf("PORT: %d\r\n",form1.port);
			}else return false;
		}
		set_form_flash_init(form1);
		return true;
}

bool Llenar_form_modbus(char *p,struct form_home form){
	char *ini;
	int count;
	tipo_de_medidor tipo;

	/*Extrayendo Modo Output*/
	ini = strstr(p,"modoOutput=");
	if(ini!=NULL){
		ini +=sizeof("modoOutput=")-1;
		for(int i =0;ini[i]!='&';i++){
			form.tipo[i]=ini[i];
		}
		printf("Modo de Nodo: %s\r\n",form.tipo);
	}
	else{
		return false;
	}

	tipo = str2enum(form.tipo);

	switch(tipo){
		case(rs485):
		ini=strstr(p,"baud_rate=");
		if(ini!=NULL){
			ini+=sizeof("baud_rate=")-1;
			char baud[10];
			count = 0;
			int auxRate;
			for(int i = 0; ini[i]!='&';i++){
				baud[i]=ini[i];
				count++;
			}
			baud[count]=0;
			auxRate = atoi(baud);
			if(auxRate>=0){
				form.baud_rate =(uint32_t)auxRate;
				printf("Baud Rate:%d\r\n",form.baud_rate);
			}else{
				return false;
			}
		}

		break;

		case(pulsos):
			/*Extrayendo Factor de Conversion*/
				ini=strstr(p,"conversion=");
				if(ini!=NULL){
					ini+=sizeof("conversion=")-1;
					char conversion[5];
					count = 0;
					int cont;
					for(int i = 0; ini[i]!='&';i++){
						conversion[i]=ini[i];
						count++;
					}
					conversion[count]=0;
					cont = atoi(conversion);
					if(cont>=0){
						form.conversion =(uint16_t)cont;
						printf("Factor de conversion: %d imp/kWh\r\n",form.conversion);
					}else{
						return false;
					}
				}
				/*Extrayendo Medida Inicial*/
				ini=strstr(p,"energia=");
				if(ini!=NULL){
					ini+=sizeof("energia=")-1;
					char energy[]="";
					float backup_f;
					uint64_t backup=0;
					for(int i = 0; ini[i]!='&';i++){
						energy[i]=ini[i];
					}
					backup_f = atof(energy);
					printf("Medicion Energia:%.4fkWh\r\n",backup_f);
					backup = round(backup_f*form.conversion);
					if((backup>0)&(backup!=0)){
						form.energia = backup;
						printf("Medicion Pulsos:%"PRIu64"\r\n",form.energia);
					}
				}
				/*Extrayendo Slave IDl*/
				ini=strstr(p,"slaveid=");
				if(ini!=NULL){
					ini+=sizeof("slaveid=")-1;
					char slaveid[3];
					count = 0;
					int auxSlave;
					for(int i = 0; ini[i]!='&';i++){
						slaveid[i]=ini[i];
						count++;
					}
					slaveid[count]=0;
					auxSlave = atoi(slaveid);
					if(auxSlave>=0){
						form.slaveid =(uint8_t)auxSlave;
						printf("Slave ID:%d\r\n",form.slaveid);
					}else{
						return false;
					}
				}
				break;

		case(chino):
		ini=strstr(p,"baud_rate=");
				if(ini!=NULL){
					ini+=sizeof("baud_rate=")-1;
					char baud[10];
					count = 0;
					int auxRate;
					for(int i = 0; ini[i]!='&';i++){
						baud[i]=ini[i];
						count++;
					}
					baud[count]=0;
					auxRate = atoi(baud);
					if(auxRate>=0){
						form.baud_rate =(uint32_t)auxRate;
						printf("Baud Rate:%d\r\n",form.baud_rate);
					}else{
						return false;
					}
				}
				break;

		case(enlace):
		break;
	}
	set_form_flash_modbus(form);
	return true;
}

// Tarea RTOS para el servidor --------------------------------------------------//
void ServidorHTTP(){
	esp_wifi_restore();
	iniciar_wifi();
	initialise_mdns();
	ESP_ERROR_CHECK(init_fs());
    ESP_ERROR_CHECK(start_rest_server(CONFIG_EXAMPLE_WEB_MOUNT_POINT));
}
/*********************************************************************************/
