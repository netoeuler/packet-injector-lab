#define SRC_IP	"192.168.0.10"
#define DST_IP	"192.168.0.11"

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

/* Ripped from Richard Stevans Book */
unsigned short ComputeChecksum(unsigned char *data, int len)
{
         long sum = 0;  /* assume 32 bit long, 16 bit short */
	 unsigned short *temp = (unsigned short *)data;

         while(len > 1){
             sum += *temp++;
             if(sum & 0x80000000)   /* if high order bit set, fold */
               sum = (sum & 0xFFFF) + (sum >> 16);
             len -= 2;
         }

         if(len)       /* take care of left over byte */
             sum += (unsigned short) *((unsigned char *)temp);
          
         while(sum>>16)
             sum = (sum & 0xFFFF) + (sum >> 16);

        return ~sum;
}

//unsigned char* createIPHeader(){
struct iphdr* CreateIPHeader(){
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
	ip_header->check = 0;
	ip_header->saddr = inet_addr(SRC_IP);
	ip_header->daddr = inet_addr(DST_IP);

	// Calculate the IP checksum now : The IP Checksum is only over the IP header
	//ip_header->check = ComputeChecksum((unsigned char *)ip_header, ip_header->ihl*4);

	//return ((unsigned char *)ip_header);
	return ((struct iphdr*)ip_header);
}