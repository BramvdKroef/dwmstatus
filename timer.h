#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/timerfd.h>
#include <unistd.h>
#include <fcntl.h>

/**
 * Run a non-blocking timer as a file discriptor.
 */

int timer_start(int interval_sec) {
  int fd = -1;
  struct itimerspec timeout;

  /* create new timer */
  fd = timerfd_create(CLOCK_MONOTONIC, 0);
  if (fd <= 0) {
    printf("Failed to create timer\n");
    exit(EXIT_FAILURE);
  }

  /* set to non-blocking */
  if (fcntl(fd, F_SETFL, O_NONBLOCK)) {
    printf("Failed to set to non blocking mode\n");
    exit(EXIT_FAILURE);
  }

  /* set timeout */
  timeout.it_value.tv_sec = interval_sec;
  timeout.it_value.tv_nsec = 0;
  timeout.it_interval.tv_sec = interval_sec; 
  timeout.it_interval.tv_nsec = 0;

  if (timerfd_settime(fd, 0, &timeout, NULL)) {
    printf("Failed to set timer duration\n");
    exit(EXIT_FAILURE);
  }

  return fd;
}

void timer_check(int fd) {
  unsigned long long missed;
  read(fd, &missed, sizeof(missed));
}
