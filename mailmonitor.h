#include <sys/inotify.h>
#include <stdio.h>
#include <errno.h>
#include <dirent.h>
#include <string.h>

/**
 * Monitor for Maildir mailboxes.
 *
 * All it does is track the number of files inside a given directory.
 */

struct mailmonitor_data {
  const char* mailbox;
  size_t emails;
};

int mailmonitor_email_init(const char* mailbox) {
  int n = 0;
  DIR* dir = NULL;
  struct dirent* rf = NULL;
  size_t len;

  if ((dir = opendir(mailbox)) == NULL) {
    return -1;
  } else {
    /*count number of file*/
    while ((rf = readdir(dir)) != NULL) {
      if (strcmp(rf->d_name, ".") != 0 &&
          strcmp(rf->d_name, "..") != 0) {
        n++;
      }
    }

    closedir(dir);
  }
  return n;
}

void mailmonitor_update(char* status, size_t length, const char* format, size_t emails) {
  size_t len;
  
  len = snprintf(status, length + 1, format, emails);
  status[min(len, length)] = ' ';
}

int mailmonitor_start(const char* inbox, char* status, size_t length,
                      const char* format, size_t* emails) {
  int fd;
  if ((fd = inotify_init1(IN_NONBLOCK)) == -1) {
    perror("inotify_init1");
    exit(EXIT_FAILURE);
  }

  if ((inotify_add_watch(fd, inbox,
                         IN_CREATE | IN_DELETE | IN_MOVED_TO |  IN_CLOSE_WRITE |
                         IN_MOVED_FROM ))
      == -1) {
    perror("inotify_add_watch");
    exit(EXIT_FAILURE);
  }
  *emails = mailmonitor_email_init(inbox);
  mailmonitor_update(status, length, format, *emails);  
  return fd;
}


void mailmonitor_check(int fd, char* status, size_t length,
                       const char* format, size_t *emails) {
  char buf[4096]
    __attribute__ ((aligned(__alignof__(struct inotify_event))));
  const struct inotify_event *event;
  int i;
  ssize_t len;
  char *ptr;

  while ((len = read(fd, buf, sizeof buf)) != 0) {
    /* Read some events. */
    
    if (len == -1) {
      if (errno == EAGAIN)
        break;
      else {
        perror("read");
        exit(EXIT_FAILURE);
      }
    }

    for (ptr = buf; ptr < buf + len;
         ptr += sizeof(struct inotify_event) + event->len) {

      event = (const struct inotify_event *) ptr;
      
      if (event->mask & IN_CREATE ||
          event->mask & IN_MOVED_TO ||
          event->mask & IN_CLOSE_WRITE )
        (*emails)++;
      if (event->mask & IN_DELETE ||
          event->mask & IN_MOVED_FROM )
        (*emails)--;
    }
  }
  mailmonitor_update(status, length, format, *emails);
}

int mailmonitor_ctl(CtlOp op, int fd, CtlData* data) {
  struct mailmonitor_data *maildata = data->arg;
  switch(op) {

  case CTL_START:
    return mailmonitor_start(maildata->mailbox,
                             data->status,
                             data->length,
                             data->format,
                             &maildata->emails);

  case CTL_CHECK:
    mailmonitor_check(fd, data->status,
                      data->length,
                      data->format,
                      &maildata->emails);
    return 0;
  }
}


