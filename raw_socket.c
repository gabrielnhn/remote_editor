#include "network.h"


int RawSocketConnection(char *device, struct sockaddr_ll* address)
// Adapted.
{
  int my_socket;
  struct ifreq ir;
  struct packet_mreq mr;

  my_socket = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));  	/*cria socket*/
  if (my_socket == -1) {
    printf("Socket error\n");
    exit(-1);
  }

  memset(&ir, 0, sizeof(struct ifreq));  	/*dispositivo eth0*/
  memcpy(ir.ifr_name, device, strlen(device));
  if (ioctl(my_socket, SIOCGIFINDEX, &ir) == -1) {
    printf("ioctl error\n");
    exit(-1);
  }
	

  memset(address, 0, sizeof(*address)); 	/*IP do dispositivo*/
  address->sll_family = AF_PACKET;
  address->sll_protocol = htons(ETH_P_ALL);
  address->sll_ifindex = ir.ifr_ifindex;
  if (bind(my_socket, (struct sockaddr *)address, sizeof(*address)) == -1) {
    printf("bind error\n");
    exit(-1);
  }


  memset(&mr, 0, sizeof(mr));          /*Modo Promiscuo*/
  mr.mr_ifindex = ir.ifr_ifindex;
  mr.mr_type = PACKET_MR_PROMISC;
  if (setsockopt(my_socket, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mr, sizeof(mr)) == -1)	{
    printf("setsockopt error\n");
    exit(-1);
  }

  return my_socket;
}
