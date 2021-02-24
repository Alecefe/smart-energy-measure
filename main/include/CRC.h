#include <stdint.h>

typedef struct {
  uint16_t b0 : 1;
  uint16_t b1 : 1;
  uint16_t b2 : 1;
  uint16_t b3 : 1;
  uint16_t b4 : 1;
  uint16_t b5 : 1;
  uint16_t b6 : 1;
  uint16_t b7 : 1;
  uint16_t b8 : 1;
  uint16_t b9 : 1;
  uint16_t b10 : 1;
  uint16_t b11 : 1;
  uint16_t b12 : 1;
  uint16_t b13 : 1;
  uint16_t b14 : 1;
  uint16_t b15 : 1;
} WORD_BITS;

typedef union {
  uint16_t Val;
  WORD_BITS bits;
  struct {
    uint8_t LB;
    uint8_t HB;
  } byte;

} INT_VAL;

uint16_t CRC16(const unsigned char *nData, uint16_t wLength);
