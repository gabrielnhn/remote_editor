#ifndef __NETWORK__
#define __NETWORK__

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/ethernet.h>
#include <netinet/in.h>
#include <linux/if_packet.h>
#include <linux/if.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <iso646.h>
#include <sys/time.h>

#define MAX_TRIES 20
//microsec
#define TIME_BETWEEN_TRIES 1000000

int raw_socket_connection(char *device);

#endif