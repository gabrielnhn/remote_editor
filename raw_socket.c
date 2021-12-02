#include "network.h"
#include <fcntl.h>


int raw_socket_connection(char *device)
{
    int my_socket;
    struct ifreq ir;
    struct sockaddr_ll endereco;
    struct packet_mreq mr;

    my_socket = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));  	/*cria socket*/
    if (my_socket == -1)
    {
        printf("Erro no Socket\n");
        exit(-1);
    }

    memset(&ir, 0, sizeof(struct ifreq));  	/*dispositivo eth0*/
    memcpy(ir.ifr_name, device, strlen(device));
    if (ioctl(my_socket, SIOCGIFINDEX, &ir) == -1)
    {
        printf("Erro no ioctl\n");
        exit(-1);
    }


    memset(&endereco, 0, sizeof(endereco)); 	/*IP do dispositivo*/
    endereco.sll_family = AF_PACKET;
    endereco.sll_protocol = htons(ETH_P_ALL);
    endereco.sll_ifindex = ir.ifr_ifindex;
    if (bind(my_socket, (struct sockaddr *)&endereco, sizeof(endereco)) == -1)
    {
        printf("Erro no bind\n");
        exit(-1);
    }


    memset(&mr, 0, sizeof(mr));          /*Modo Promiscuo*/
    mr.mr_ifindex = ir.ifr_ifindex;
    mr.mr_type = PACKET_MR_PROMISC;
    if (setsockopt(my_socket, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mr, sizeof(mr)) == -1)
    {
        printf("Erro ao fazer setsockopt\n");
        exit(-1);
    }

    // Set timeout
    fcntl(my_socket, F_SETFL, O_NONBLOCK);


    return my_socket;
}
