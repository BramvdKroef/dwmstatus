#include <errno.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <X11/Xlib.h>

#include <signal.h>
#include <sys/signalfd.h>

#define min(x,y) (x < y ? x : y)
#define max(x,y) (x > y ? x : y)

typedef enum ctlop {CTL_START, CTL_CHECK} CtlOp;


typedef struct ctlData {
  const size_t length;
  const char* format;
  const int (*fnc) (CtlOp op, int fd, struct ctlData *data);
  char* status;
  void* arg;
} CtlData;

#include "timer.h"
#include "mailmonitor.h"
#include "ram.h"
#include "datetime.h"
#include "weather.h"
#include "net_if.h"

#include "config.h"

void updateStatus(Display* display, const char* status) {
  XStoreName(display, DefaultRootWindow(display), status);
  XSync(display, 0);
}


int main(int argc, char* argv[]) {
  char* status;

  Display* display;

  sigset_t mask;
  struct signalfd_siginfo siginfo;
  
  nfds_t nfds;
  struct pollfd fds[5];
  int poll_num;

  size_t status_len = (ctl_n - 1) * 3;

  size_t i;
  
  for (i = 0; i < ctl_n; i++) {
    status_len += ctl_p[i].length;
  }

  status = (char*)malloc(status_len);
  memset(status, ' ', status_len);

  ctl_p[0].status = status;

  for (i = 1; i < ctl_n; i++) {
    ctl_p[i].status = ctl_p[i - 1].status + ctl_p[i - 1].length + 3;
    *(ctl_p[i].status - 2) = '|';
  }
  
  if (( display = XOpenDisplay(0x0)) == NULL ) {
    fprintf(stderr, "Cannot open display!\n");
    exit(EXIT_FAILURE);
  }

  nfds = ctl_n + 1;

  for (i = 0; i < ctl_n; i++) {
    fds[i].fd = (*ctl_p[i].fnc)(CTL_START, 0, &ctl_p[i]);
    fds[i].events = POLLIN;
  }


  sigemptyset(&mask);
  sigaddset(&mask, SIGINT);
  sigaddset(&mask, SIGQUIT);

  if (sigprocmask(SIG_BLOCK, &mask, NULL) == -1) {
    perror("sigprocmask");
    exit(EXIT_FAILURE);
  }

  fds[ctl_n].fd = signalfd(-1, &mask, SFD_NONBLOCK);
  fds[ctl_n].events = POLLIN;

  updateStatus(display, status);

  while (1) {
    poll_num = poll(fds, nfds, -1);
    if (poll_num == -1) {
      if (errno == EINTR)
        continue;
      perror("poll");
      exit(EXIT_FAILURE);
    }

    if (poll_num > 0) {

      for (i = 0; i < ctl_n; i++) {
        if (fds[i].revents & POLLIN) {
          (*ctl_p[i].fnc)(CTL_CHECK, fds[i].fd, &ctl_p[i]);
        }
      }

      if (fds[ctl_n].revents & POLLIN) {
        if (read(fds[ctl_n].fd, &siginfo, sizeof(struct signalfd_siginfo))
            == sizeof(struct signalfd_siginfo)) {
          if (siginfo.ssi_signo == SIGINT) {
            printf("Got SIGINT\n");
          } else if (siginfo.ssi_signo == SIGQUIT) {
            printf("Got SIGQUIT\n");
          }
        }
        break;
      }

      updateStatus(display, status);
    }
  }

  printf("Closing\n");

  for (i = 0; i < nfds; i++) {
    close(fds[i].fd);
  }

  XCloseDisplay(display);
  free(status);
  exit(EXIT_SUCCESS);
}
