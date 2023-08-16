#include<stdio.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netinet/ether.h>
#include<sys/ioctl.h>
#include<linux/if.h>
#include<linux/ip.h>
#include<linux/tcp.h>
#include<string.h>
#include<netpacket/packet.h>
#include<unistd.h>
#include<arpa/inet.h>

int main(int argc,char** argv) {
    struct ifreq ifr;
    struct sockaddr_ll addr;
    struct packet_mreq mreq;
    struct ethhdr *eth;
    struct iphdr *iph;
    struct tcphdr *tcp;
    int sock = socket(AF_PACKET,SOCK_RAW,htons(ETH_P_ALL));
    if(sock < 0) {
        perror("socket create Error...");
        return -1;
    }

    if(argc < 2) {
        printf("arg setting network interface..\n");
        return -1;
    }

    memset(&ifr,0,sizeof(ifr));
    memcpy(&ifr.ifr_ifrn.ifrn_name,argv[1],IFNAMSIZ);
    if(ioctl(sock, SIOCGIFINDEX, &ifr) != 0) {
        perror("ioctl request ...");
        return -1;
    }

    memset(&mreq,0,sizeof(mreq));
    mreq.mr_ifindex = ifr.ifr_ifru.ifru_ivalue;
    mreq.mr_type = PACKET_MR_PROMISC;
    if(setsockopt(sock,SOL_PACKET,PACKET_ADD_MEMBERSHIP,(char*)&mreq,sizeof(mreq)) != 0) {
        perror("setsockopt error....");
        return -1;
    }


    memset(&addr,0,sizeof(addr));
    addr.sll_family = AF_PACKET;
    addr.sll_ifindex = ifr.ifr_ifru.ifru_ivalue;
    addr.sll_protocol = htons(ETH_P_ALL);

    bind(sock,(struct sockaddr*)&addr, sizeof(addr));

    char buf[2048];
    char ipaddr1[16]; //XXX.XXX.XXX.XXX
    char ipaddr2[16]; 

    int defaultsize = sizeof(struct ethhdr) + sizeof(struct iphdr) + sizeof(struct tcphdr);
    while(1) {
        int size = read(sock,buf,2048);
        if(defaultsize > size) {
            memset(buf,0,sizeof(buf));
            continue;
        }

        eth = (struct ethhdr*)buf;
        if(htons(eth->h_proto) == ETH_P_IP) {
        } else {
            memset(buf,0,sizeof(buf));
            continue;
        }

        iph = (struct iphdr*)(buf + sizeof(struct ethhdr));
        if(iph->protocol == IPPROTO_TCP) {
        } else {
            memset(buf,0,sizeof(buf));
            continue;
        }
        memcpy(ipaddr1,inet_ntoa(*(struct in_addr*)&iph->saddr),16);
        memcpy(ipaddr2,inet_ntoa(*(struct in_addr*)&iph->daddr),16);

        tcp = (struct tcphdr*)(buf + sizeof(struct ethhdr) + sizeof(struct iphdr));
        if(htons(tcp->dest) == 443 || htons(tcp->source) == 443) {
        } else if(htons(tcp->dest) == 9998 || htons(tcp->source) == 9998) {
        } else {
            memset(buf,0,sizeof(buf));
            memset(ipaddr1,0,sizeof(ipaddr1));
            memset(ipaddr2,0,sizeof(ipaddr2));
            continue;
        }
        printf("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
        printf("%s : %d // %s : %d\n",ipaddr1,htons(tcp->source),ipaddr2,htons(tcp->dest));
        char* data = (char*)(buf + sizeof(struct ethhdr) + sizeof(struct iphdr) + sizeof(struct tcphdr));
        printf("this is data ->  %s\n",data);
        printf("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");

        memset(buf,0,sizeof(buf));
        memset(ipaddr1,0,sizeof(ipaddr1));
        memset(ipaddr2,0,sizeof(ipaddr2));
    }
    return 0;
}