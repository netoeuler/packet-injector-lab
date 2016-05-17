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

#define PACKET_LENGTH	1024


int CreateRawSocket(int protocol_to_sniff)
{
	int rawsock;

	if((rawsock = socket(PF_PACKET, SOCK_RAW, htons(protocol_to_sniff)))== -1)
	{
		perror("Error creating raw socket: ");
		exit(-1);
	}

	return rawsock;
}

int BindRawSocketToInterface(char *device, int rawsock, int protocol)
{
	
	struct sockaddr_ll sll;
	struct ifreq ifr;

	bzero(&sll, sizeof(sll));
	bzero(&ifr, sizeof(ifr));
	
	/* First Get the Interface Index  */


	strncpy((char *)ifr.ifr_name, device, IFNAMSIZ);
	if((ioctl(rawsock, SIOCGIFINDEX, &ifr)) == -1)
	{
		printf("Error getting Interface index !\n");
		exit(-1);
	}

	/* Bind our raw socket to this interface */

	sll.sll_family = AF_PACKET;
	sll.sll_ifindex = ifr.ifr_ifindex;
	sll.sll_protocol = htons(protocol); 


	if((bind(rawsock, (struct sockaddr *)&sll, sizeof(sll)))== -1)
	{
		perror("Error binding raw socket to interface\n");
		exit(-1);
	}

	return 1;
	
}


int SendRawPacket(int rawsock, unsigned char *pkt, int pkt_len)
{
	int sent= 0;

	/* A simple write on the socket ..thats all it takes ! */

	if((sent = write(rawsock, pkt, pkt_len)) != pkt_len)
	{
		return 0;
	}

	return 1;
}

unsigned char* createIPHeader(){

}

unsigned char* createEthernetHeader(){
	struct ethhdr *ethernet_header;
	ethernet_header = (struct ethhdr *)malloc(sizeof(struct ethhdr));

	memcpy(ethernet_header->h_source, (void *)ether_aton("aa:aa:aa:aa:aa:aa"), 6);
	memcpy(ethernet_header->h_dest, (void *)ether_aton("bb:bb:bb:bb:bb:bb"), 6);
	ethernet_header->h_proto = htons(ETHERTYPE_IP);

	printf("hdr: %s\n",((unsigned char*)ethernet_header));

	return ((unsigned char*)ethernet_header);
}

unsigned char* createEthernetHeader2(){
	unsigned char* packet = malloc(PACKET_LENGTH);
	memset(packet, 'A', PACKET_LENGTH);
	int i;
	for (i=0; i<6; i++) packet[i] = 'B';
	for (i=0; i<6; i++) packet[i+6] = 'C';
	for (i=0; i<2; i++) packet[i+12] = 'D';	

	return packet;
}


/* argv[1] is the device e.g. eth0
   argv[2] is the number of packets to send
*/
 
main(int argc, char **argv)
{

	int raw;
	//unsigned char packet[PACKET_LENGTH];
	unsigned char* packet;
	int num_of_pkts;
	int pkt_len;
	
	/* Create the raw socket */
	raw = CreateRawSocket(ETH_P_ALL);

	/* Bind raw socket to interface */
	BindRawSocketToInterface(argv[1], raw, ETH_P_ALL);

	packet = createEthernetHeader2();
	pkt_len = PACKET_LENGTH;
	//pkt_len = sizeof(struct ethhdr);

	num_of_pkts = atoi(argv[2]);

	while((num_of_pkts--)>0)
	{

		if(!SendRawPacket(raw, packet, pkt_len))
			perror("Error sending packet");
		else
			printf("Packet sent successfully\n");
	}

	close(raw);

	return 0;
}

