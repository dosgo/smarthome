#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <arpa/inet.h>
#include <netinet/if_ether.h>
#include <net/if.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <errno.h>
struct arpMsg {
struct ethhdr ethhdr; /* Ethernet header */
u_short htype; /* hardware type (must be ARPHRD_ETHER) */
u_short ptype; /* protocol type (must be ETH_P_IP) */
u_char hlen; /* hardware address length (must be 6) */
u_char plen; /* protocol address length (must be 4) */
u_short operation; /* ARP opcode */
u_char sHaddr[6]; /* sender's hardware address */
u_char sInaddr[4]; /* sender's IP address */
u_char tHaddr[6]; /* target's hardware address */
u_char tInaddr[4]; /* target's IP address */
u_char pad[18]; /* pad for min. Ethernet payload (60 bytes) */
};
/* miscellaneous defines */
#define MAC_BCAST_ADDR (uint8_t *) "\xff\xff\xff\xff\xff\xff"
#define OPT_CODE 0
#define OPT_LEN 1
#define OPT_DATA 2
void print_mac(unsigned char * mac_addr)
{
    for (int i =0; i < 6; ++i)
    {
    printf("%02X", mac_addr[i]);
    if (i != 5) printf(":");
    }
    printf("\n");
}
void print_ip(unsigned char * ip_addr)
{
    printf("ip:");
    for (int i =0; i < 4; ++i)
    {
    printf("%d", ip_addr[i]);
    if (i != 3) printf(".");
    }
    printf("\r\n");
}

void get_local_addr(unsigned char* mac, u_int32_t &ip)
{
    struct ifconf interface_conf;
    struct ifreq ifreq1;
    int sock;
    struct sockaddr_in* psockaddr_in = NULL;
    if ( (sock = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
    {
        printf("Unable to create socket for geting the mac address");
        exit(1);
    }

    strcpy(ifreq1.ifr_name, "wlan0");
    if (ioctl(sock, SIOCGIFHWADDR, &ifreq1) < 0)
    {
       printf("Unable to get the mac address");
       exit(1);
    }
    memcpy(mac, ifreq1.ifr_hwaddr.sa_data, 6);
   if (ioctl(sock, SIOCGIFADDR, &ifreq1) < 0)
   {
      printf("Unable to get the ip address");
      exit(1);
    }
    psockaddr_in = (struct sockaddr_in*)&ifreq1.ifr_addr;
    ip = psockaddr_in->sin_addr.s_addr;
//print_ip((unsigned char*)ip);
}
int macfindip( char* dmacv)
{
    printf("go1\r\n");
    int timeout = 2;
    int optval = 1;
    int s; /* socket */
    int rv = 1; /* return value */
    struct sockaddr addr; /* for interface name */
    struct arpMsg arp;
    fd_set fdset;
    struct timeval tm;
    time_t prevTime;
    u_int32_t ip;
    struct in_addr my_ip;
    struct in_addr dst_ip;
    char buff[2000];

    unsigned char mac[6];
    unsigned char dmac[6];
printf("go1\r\n");
    char interface[] = "wlan0";

    //if (argc != 2)
   // {
     //   printf("Usage: get_ip_by_mac dst_mac\n");
        //printf("For example: get_ip_by_mac 00:0F:EA:40:0D:04\n");
     //   return 0;
   // }
    get_local_addr(mac, ip);
printf("go2\r\n");
    for (int i = 0; i < 6; ++i)
    {
        strncpy(buff, dmacv+3*i, 2);
        buff[3] = '\0';
        dmac[i] = strtol(buff, (char**)NULL, 16);
    }
    if ((s = socket (PF_PACKET, SOCK_PACKET, htons(ETH_P_RARP))) == -1)
    {
        printf("Could not open raw socket\n");
        return -1;
    }

    if (setsockopt(s, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(optval)) == -1)
    {
        printf("Could not setsocketopt on raw socket\n");
        close(s);
        return -1;
    }

    memset(&addr, 0, sizeof(addr));
    strcpy(addr.sa_data, interface);

    /* send rarp request */
    memset(&arp, 0, sizeof(arp));
    memcpy(arp.ethhdr.h_dest, MAC_BCAST_ADDR, 6); /* MAC DA */
    memcpy(arp.ethhdr.h_source, mac, 6); /* MAC SA */
    arp.ethhdr.h_proto = htons(ETH_P_RARP); /* protocol type (Ethernet) */
    arp.htype = htons(ARPHRD_ETHER); /* hardware type */
    arp.ptype = htons(ETH_P_IP); /* protocol type (ARP message) */
    arp.hlen = 6; /* hardware address length */
    arp.plen = 4; /* protocol address length */
    arp.operation = htons(3); /* RARP request code */
    *((u_int *) arp.sInaddr) = ip; /* source IP address */
    memcpy(arp.sHaddr, mac, 6); /* source hardware address */
    memcpy(arp.tHaddr, dmac, 6);
    if (sendto(s, &arp, sizeof(arp), 0, &addr, sizeof(addr)) < 0)
    {
        printf("Unabele to send arp request");
        return 0;
    }
    rv = 0;
    printf("go\r\n");
    /* wait arp reply, and check it */
    tm.tv_usec = 0;
    time(&prevTime);
    while (timeout > 0)
    {
          printf("go-1\r\n");
        FD_ZERO(&fdset);
        FD_SET(s, &fdset);
        tm.tv_sec = 1000*1000;
        if (select(s + 1, &fdset, NULL,  NULL, &tm) < 0)
        {
            printf("Error on ARPING request:");
            if (errno != EINTR) rv = 0;
        }
        else if (FD_ISSET(s, &fdset))
        {
            if (recv(s, &arp, sizeof(arp), 0) < 0 )
            {
                printf("Unable get valid rarp response");
                rv = 0;
            }
            if (arp.operation == htons(4) &&bcmp(arp.tHaddr, mac, 6) == 0 )
            {
                printf("Valid rarp reply receved for this address\n");
                //print_mac(arp.sHaddr);
                //print_ip(arp.sInaddr);
                rv = 0;
                break;
            }

            print_ip(arp.tInaddr);
        }
        timeout -= time(NULL) - prevTime;
        time(&prevTime);
    }
    close(s);
    return 0;
}
