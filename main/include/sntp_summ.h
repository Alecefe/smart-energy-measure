#define TIME_FORMAT "%04i-%02i-%02i %02i:%02i:%02i -0400"
char *time_string(time_t *now, struct tm *t);
void print_time(time_t now, struct tm t);
void simple_ntp();
