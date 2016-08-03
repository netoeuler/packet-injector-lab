#define SRC_PORT	80
#define DST_PORT	100

typedef struct PseudoHeader{
	unsigned long int source_ip;
	unsigned long int dest_ip;
	unsigned char reserved;
	unsigned char protocol;
	unsigned short int tcp_length;
}PseudoHeader;

/*The TCP Checksum is calculated over the PseudoHeader + TCP header +Data*/
void CreatePseudoHeaderAndComputeTcpChecksum(struct tcphdr *tcp_header, struct iphdr *ip_header, unsigned char *data){
	/* Find the size of the TCP Header + Data */
	int segment_len = ntohs(ip_header->tot_len) - ip_header->ihl*4; 

	/* Total length over which TCP checksum will be computed */
	int header_len = sizeof(PseudoHeader) + segment_len;

	/* Allocate the memory */
	unsigned char *hdr = (unsigned char *)malloc(header_len);

	/* Fill in the pseudo header first */	
	PseudoHeader *pseudo_header = (PseudoHeader *)hdr;

	pseudo_header->source_ip = ip_header->saddr;
	pseudo_header->dest_ip = ip_header->daddr;
	pseudo_header->reserved = 0;
	pseudo_header->protocol = ip_header->protocol;
	pseudo_header->tcp_length = htons(segment_len);
	
	/* Now copy TCP */
	memcpy((hdr + sizeof(PseudoHeader)), (void *)tcp_header, tcp_header->doff*4);

	/* Now copy the Data */
	memcpy((hdr + sizeof(PseudoHeader) + tcp_header->doff*4), data, DATA_SIZE);

	/* Calculate the Checksum */
	tcp_header->check = ComputeChecksum(hdr, header_len);

	/* Free the PseudoHeader */
	//free(hdr);

	return ;
}

struct tcphdr *CreateTcpHeader(){
	struct tcphdr *tcp_header;

	/* Check /usr/include/linux/tcp.h for header definition */
	tcp_header = (struct tcphdr *)malloc(sizeof(struct tcphdr));
	
	tcp_header->source = htons(SRC_PORT);
	tcp_header->dest = htons(DST_PORT);
	tcp_header->seq = htonl(1);
	tcp_header->ack_seq = htonl(1);
	tcp_header->res1 = 0;
	tcp_header->doff = (sizeof(struct tcphdr))/4;
	tcp_header->syn = 1;
	tcp_header->window = htons(100);
	tcp_header->check = 0; /* Will calculate the checksum with pseudo-header later */
	tcp_header->urg_ptr = 0;

	return (tcp_header);
}

unsigned char *CreateData(int len){
	unsigned char *data = (unsigned char *)malloc(len);  
	int counter;	
	#if 0
	struct timeval tv;
	struct timezone tz;	
	
	/* get time of the day */
	gettimeofday(&tv, &tz);

	/* seed the random number generator */
	srand(tv.tv_sec);
	
	/* Add random data for now */
	for(counter = 0  ; counter < len; counter++)
		data[counter] = 255.0 *rand()/(RAND_MAX +1.0);	
	#endif

	for(counter = 0 ; counter < len ; counter++) data[counter] = 'k';

	return data;
}