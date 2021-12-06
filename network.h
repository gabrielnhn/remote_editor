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

#define MAX_SEND_TRIES 2
#define MAX_RECEIVE_TRIES 30
//microsec
#define TIME_BETWEEN_SEND_TRIES 10000 //* 100 //* 3
#define TIME_BETWEEN_RECV_TRIES 10000

int raw_socket_connection(char *device);

#endif