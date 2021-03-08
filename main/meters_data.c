#include "meters_data.h"
#include <stdlib.h>

#define NELEMS(x) (sizeof(x) / sizeof((x)[0]))

meter_info meters[] = {{"000201", "10", "2021-01-19 14:27:00.096898 -0400"},
                       {"000202", "20", "2021-01-19 14:27:00.096898 -0400"},
                       {"000203", "30", "2021-01-19 14:27:00.096898 -0400"},
                       {"0012421", "55", "201021031 -213 4214 -0400"}};
size_t meters_len = NELEMS(meters);
