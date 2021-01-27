#include <stdint.h>

// ESTRUCTURAS DE DATOS PARA FORMULARIOS
typedef struct {
  char ssid[30];
  char password[30];
} form_locwifi;

typedef struct {
  char tipo[7];
  uint64_t energia;
  uint8_t slaveid;
  uint16_t conversion;
  uint32_t baud_rate;
} form_modbus;

typedef struct {
  char user[21];
  char password[21];
} form_login;

typedef struct {
  uint8_t mesh_id[6];
  char meshappass[20];
  uint8_t max_layer;
  uint8_t max_sta;
  uint16_t port;
} form_mesh;

typedef struct {
  uint8_t advance;
  char uri[30];
  char ip[20];
  uint16_t port;
  char pubtopic[20];
  uint8_t type;
  char user[20];
  char password[20];
  uint8_t app_layer;
} form_mqtt;

typedef enum { rs485 = 0, pulsos, chino, enlace } tipo_de_medidor;

typedef union {
  uint64_t tot;
  struct {
    uint32_t l32;
    uint32_t h32;
  } u32;
  struct {
    uint16_t ll16;
    uint16_t l16;
    uint16_t h16;
    uint16_t hh16;
  } u16;
  struct {
    uint8_t llll8;
    uint8_t lll8;
    uint8_t ll8;
    uint8_t l8;
    uint8_t h8;
    uint8_t hh8;
    uint8_t hhh8;
    uint8_t hhhh8;
  } u8;
} energytype_t;

typedef struct {
  tipo_de_medidor val;
  const char *str;
} conversion_t;
