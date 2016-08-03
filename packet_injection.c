/*
Based on code from this link: http://hackoftheday.securitytube.net/2013/04/my-code-made-it-to-hollywood-movie.html
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <features.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <net/ethernet.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <arpa/inet.h> //IP_PROTOTCP
//#include <sys/time.h>

#include <string.h> /* memset */
#include <unistd.h> /* close */

#define PACKET_LENGTH	1024
#define DATA_SIZE 		100

#include "validator.c"
#include "eth_header.c"
#include "ip_header.c"
#include "tcp_header.c"

int CreateRawSocket(int protocol_to_sniff){
	int rawsock;

	if((rawsock = socket(PF_PACKET, SOCK_RAW, htons(protocol_to_sniff)))== -1)	{
		perror("Error creating raw socket: ");
		exit(-1);
	}

	return rawsock;
}

int BindRawSocketToInterface(char *device, int rawsock, int protocol){
	
	struct sockaddr_ll sll;
	struct ifreq ifr;

	//memset(&sll, 0, sizeof(struct sockaddr_ll));
	//memset(&ifr, 0, sizeof(struct ifreq));

	bzero(&sll, sizeof(sll));
	bzero(&ifr, sizeof(ifr));
	
	/* First Get the Interface Index  */
	strncpy((char *)ifr.ifr_name, device, IFNAMSIZ);
	if((ioctl(rawsock, SIOCGIFINDEX, &ifr)) == -1)	{
		printf("Error getting Interface index !\n");
		exit(-1);
	}

	/* Bind our raw socket to this interface */
	sll.sll_family = AF_PACKET;
	sll.sll_ifindex = ifr.ifr_ifindex;
	sll.sll_protocol = htons(protocol); 


	if((bind(rawsock, (struct sockaddr *)&sll, sizeof(sll)))== -1)	{
		perror("Error binding raw socket to interface\n");
		exit(-1);
	}

	return 1;
	
}


int SendRawPacket(int rawsock, unsigned char *pkt, int pkt_len){
	int sent= 0;

	/* A simple write on the socket ..thats all it takes ! */
	if((sent = write(rawsock, pkt, pkt_len)) != pkt_len){ 
		return 0;
	}

	return 1;
}

/* argv[1] is the device e.g. eth0
   argv[2] is the number of packets to send
*/ 
int main(int argc, char **argv){
	if (argc < 1){
		perror("Arguments?\n");
		return 1;
	}

	int raw;
	unsigned char* packet;
	struct ethhdr *ethernet_header;
	struct iphdr *ip_header;
	struct tcphdr  *tcp_header;
	unsigned char *data;	
	int num_of_pkts;
	int pkt_len;
	
	/* Create the raw socket */
	raw = CreateRawSocket(ETH_P_ALL);

	/* Bind raw socket to interface */
	BindRawSocketToInterface(argv[1], raw, ETH_P_ALL);
	//num_of_pkts = atoi(argv[2]);

	ethernet_header = CreateEthernetHeader();
	ip_header = CreateIPHeader();
	tcp_header = CreateTcpHeader();
	data = CreateData(DATA_SIZE);

	/* Create PseudoHeader and compute TCP Checksum  */
	CreatePseudoHeaderAndComputeTcpChecksum(tcp_header, ip_header, data);

	//pkt_len = sizeof(struct ethhdr) + sizeof(struct iphdr);
	pkt_len = sizeof(struct ethhdr) + ntohs(ip_header->tot_len);

	packet = (unsigned char *) malloc(pkt_len);
	memcpy(packet, ethernet_header, sizeof(struct ethhdr));
	memcpy((packet + sizeof(struct ethhdr)), ip_header, ip_header->ihl*4);
	memcpy((packet + sizeof(struct ethhdr) + ip_header->ihl*4),tcp_header, tcp_header->doff*4);
	memcpy((packet + sizeof(struct ethhdr) + ip_header->ihl*4 + tcp_header->doff*4), data, DATA_SIZE);

	//while((num_of_pkts--)>0){
		if(!SendRawPacket(raw, packet, pkt_len))
			perror("Error sending packet");
		else
			printf("Packet sent successfully\n");
	//}

	/*free(ethernet_header);
	free(ip_header);
	free(tcp_header);
	free(data);
	free(packet);*/

	close(raw);

	return 0;
}

