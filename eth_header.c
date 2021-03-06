#define SRC_ETHER_ADDR  "xx:aa:aa:aa:aa:aa"
#define DST_ETHER_ADDR	"xx:xx:xx:aa:aa:aa"

struct ethhdr* CreateEthernetHeader(){
	struct ethhdr *ethernet_header;
	ethernet_header = (struct ethhdr *)malloc(sizeof(struct ethhdr));

	int aton_src = (intptr_t)ether_aton(SRC_ETHER_ADDR);
	int aton_dst = (intptr_t)ether_aton(DST_ETHER_ADDR);
	memcpy(&ethernet_header->h_source, &aton_src, 6);
	memcpy(&ethernet_header->h_dest, &aton_dst, 6);

	ethernet_header->h_proto = htons(ETHERTYPE_IP);

	return ((struct ethhdr*)ethernet_header);
}