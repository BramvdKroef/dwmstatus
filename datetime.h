#include <time.h>

void datetime_update(char* status, size_t length, const char* format) {
  time_t now = time(0);
  size_t len;

  len = strftime(status, length + 2, format, localtime(&now));
  if (len != 0)
    status[len] = ' ';
}

int datetime_ctl(CtlOp op, int fd, CtlData *data) {
  switch(op) {
    
  case CTL_START:
    datetime_update(data->status, data->length, data->format);
    return timer_start(30);

  case CTL_CHECK:
    datetime_update(data->status, data->length, data->format);
    timer_check(fd);
    return 0;
  }
}


