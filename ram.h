
#include <sys/sysinfo.h>

/**
 * Monitors RAM usage.
 */

void ram_update(char* status, size_t length, const char* format) {
  struct sysinfo s;
  size_t len;
  
  sysinfo(&s);
  len = snprintf(status, length + 1, format,
                 100 - ((double)s.freeram / (double)s.totalram) * 100);
  status[min(len, length)] = ' ';

}

int ram_ctl(CtlOp op, int fd, CtlData *data) {
  switch(op) {
    
  case CTL_START:
    ram_update(data->status, data->length, data->format);
    return timer_start(5);

  case CTL_CHECK:
    ram_update(data->status, data->length, data->format);
    timer_check(fd);
    return 0;
  }
}
