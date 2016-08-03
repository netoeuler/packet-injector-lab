#define SRC_IP	"192.168.0.10"
#define DST_IP	"192.168.0.11"

struct iphdr* CreateIPHeader(){
	struct iphdr *ip_header;

	ip_header = (struct iphdr *)malloc(sizeof(struct iphdr));

	ip_header->version = 4;
	ip_header->ihl = (sizeof(struct iphdr))/4 ;
	ip_header->tos = 0;
	ip_header->tot_len = htons(sizeof(struct iphdr) + sizeof(struct tcphdr) + DATA_SIZE);
	ip_header->id = htons(111);
	ip_header->frag_off = 0;
	ip_header->ttl = 111;
	ip_header->protocol = IPPROTO_TCP;
	ip_header->check = 0;
	ip_header->saddr = inet_addr(SRC_IP);
	ip_header->daddr = inet_addr(DST_IP);

	// Calculate the IP checksum now : The IP Checksum is only over the IP header
	ip_header->check = ComputeChecksum((unsigned char *)ip_header, ip_header->ihl*4);

	return ((struct iphdr*)ip_header);
}