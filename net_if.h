
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <linux/if_link.h>

/**
 * Monitors network traffic on an interface.
 */

struct net_if_data {
  const char* netif;
  __u32 tx;
  __u32 rx;
};

void net_if_update(char* status, size_t length, const char* format,
                   struct net_if_data* data){
  
  struct ifaddrs *ifaddr, *ifa;
  __u32 tx_d = 0, rx_d = 0;
  size_t len;

  if (getifaddrs(&ifaddr) == -1) {
    perror("getifaddrs");
    exit(EXIT_FAILURE);
  }

  /* Walk through linked list, maintaining head pointer so we
     can free list later */

  for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
    if (ifa->ifa_addr == NULL || strcmp(ifa->ifa_name, data->netif))
      continue;

    if (ifa->ifa_addr->sa_family == AF_PACKET &&
        ifa->ifa_data != NULL) {
      struct rtnl_link_stats *stats = ifa->ifa_data;

      tx_d = (data->tx != 0) ? stats->tx_bytes - data->tx : 0;
      rx_d = (data->rx != 0) ? stats->rx_bytes - data->rx : 0;
      data->tx = stats->tx_bytes;
      data->rx = stats->rx_bytes;
    }
  }

  freeifaddrs(ifaddr);

  len = snprintf(status, length + 1, format, tx_d, rx_d);
  status[min(len, length)] = ' ';
}


int net_if_ctl(CtlOp op, int fd, CtlData *data) {
  switch(op) {
    
  case CTL_START:
    net_if_update(data->status, data->length, data->format, data->arg);
    return timer_start(5);

  case CTL_CHECK:
    net_if_update(data->status, data->length, data->format, data->arg);
    timer_check(fd);
    return 0;
  }
}
