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
#include <arpa/inet.h> //IPPROTO_TCP

#include <string.h> /* memset */
#include <unistd.h> /* close */

#define PACKET_LENGTH	1024

#define SRC_ETHER_ADDR  "aa:aa:aa:aa:aa:aa"
#define DST_ETHER_ADDR	"bb:bb:bb:bb:bb:bb"

#define SRC_IP	"192.168.0.10"
#define DST_IP	"192.168.0.11"


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

	memset(&sll, 0, sizeof(struct sockaddr_ll));
	memset(&ifr, 0, sizeof(struct ifreq));
	
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

/* Ripped from Richard Stevans Book */
unsigned short ComputeIpChecksum(unsigned char *header, int len){
	long sum = 0;  /* assume 32 bit long, 16 bit short */
 	unsigned short *ip_header = (unsigned short *)header;

     while(len > 1){
         //sum += *((unsigned short*) ip_header)++;
     	sum += *((unsigned short*) ip_header);
     	ip_header++;
         if(sum & 0x80000000)   /* if high order bit set, fold */
         	sum = (sum & 0xFFFF) + (sum >> 16);
         len -= 2;
     }

     if(len)       /* take care of left over byte */
         sum += (unsigned short) *(unsigned char *)ip_header;
      
     while(sum>>16)
         sum = (sum & 0xFFFF) + (sum >> 16);

    return ~sum;
}

unsigned char* createIPHeader(){
	struct iphdr *ip_header;

	ip_header = (struct iphdr *)malloc(sizeof(struct iphdr));

	ip_header->version = 4;
	ip_header->ihl = (sizeof(struct iphdr))/4 ;
	ip_header->tos = 0;
	ip_header->tot_len = htons(sizeof(struct iphdr));
	ip_header->id = htons(111);
	ip_header->frag_off = 0;
	ip_header->ttl = 111;
	ip_header->protocol = IPPROTO_TCP;
	//ip_header->check = 0;
	ip_header->saddr = inet_addr(SRC_IP);
	ip_header->daddr = inet_addr(DST_IP);

	// Calculate the IP checksum now : The IP Checksum is only over the IP header
	ip_header->check = ComputeIpChecksum((unsigned char *)ip_header, ip_header->ihl*4);

	return ((unsigned char *)ip_header);
}

unsigned char* createEthernetHeader(){
	struct ethhdr *ethernet_header;
	ethernet_header = (struct ethhdr *)malloc(sizeof(struct ethhdr));

	int aton_src = (intptr_t)ether_aton(SRC_ETHER_ADDR);
	int aton_dst = (intptr_t)ether_aton(DST_ETHER_ADDR);
	memcpy(&ethernet_header->h_source, &aton_src, 6);
	memcpy(&ethernet_header->h_dest, &aton_dst, 6);

	ethernet_header->h_proto = htons(ETHERTYPE_IP);

	return ((unsigned char*)ethernet_header);
}

/* argv[1] is the device e.g. eth0
   argv[2] is the number of packets to send
*/ 
int main(int argc, char **argv){
	int raw;
	unsigned char *ethernet_header;
	unsigned char *ip_header;
	unsigned char* packet;	
	int num_of_pkts;
	int pkt_len;
	
	/* Create the raw socket */
	raw = CreateRawSocket(ETH_P_ALL);

	/* Bind raw socket to interface */
	BindRawSocketToInterface(argv[1], raw, ETH_P_ALL);
	num_of_pkts = atoi(argv[2]);

	ethernet_header = createEthernetHeader();
	ip_header = createIPHeader();	

	//pkt_len = PACKET_LENGTH;
	pkt_len = sizeof(struct ethhdr) + sizeof(struct iphdr);

	packet = (unsigned char *) malloc(pkt_len);
	memcpy(packet, ethernet_header, sizeof(struct ethhdr));
	memcpy((packet + sizeof(struct ethhdr)), ip_header, sizeof(struct iphdr));

	while((num_of_pkts--)>0){
		if(!SendRawPacket(raw, packet, pkt_len))
			perror("Error sending packet");
		else
			printf("Packet sent successfully\n");
	}

	close(raw);

	return 0;
}

