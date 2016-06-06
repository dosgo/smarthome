#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/if_packet.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/ethernet.h>
#include <netinet/ether.h>
#include <net/if.h>
#include <netinet/ip.h>

#define    FAILURE   -1
#define    SUCCESS    0

unsigned char src_ip[4] = { 192, 168, 9, 118 };    //Ҫ��������IP��ַ
unsigned char src_mac[6] = {0x00, 0x0c, 0x29, 0x4b, 0x6c, 0x13};    //Ҫ����������MAC��ַ
unsigned char dst_ip[4] = { 192, 168, 9, 118 };    //Ŀ��IP��ַ
unsigned char dst_mac[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };    //ARP�㲥��ַ

int send_arp(int sockfd, struct sockaddr_ll *peer_addr);
int recv_arp(int sockfd, struct sockaddr_ll *peer_addr);

//ARP��װ��
typedef struct _tagARP_PACKET{
    struct ether_header  eh;
    struct ether_arp arp;
}ARP_PACKET_OBJ, *ARP_PACKET_HANDLE;

int freearp(char *mac)
{
    int sockfd;
    int rtval = -1;
    struct sockaddr_ll peer_addr;

     char buff[2000];
     memset(src_mac,0,6);
    for (int i = 0; i < 6; ++i)
    {
        strncpy(buff, mac+3*i, 2);
        buff[3] = '\0';
        src_mac[i] = strtol(buff, (char**)NULL, 16);
    }
    //����socket
    sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ARP));
    if (sockfd < 0)
    {
        fprintf(stderr, "socket error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    memset(&peer_addr, 0, sizeof(peer_addr));
        peer_addr.sll_family = AF_PACKET;
        struct ifreq req;
    bzero(&req, sizeof(struct ifreq));
        strcpy(req.ifr_name, "wlan0");
        if(ioctl(sockfd, SIOCGIFINDEX, &req) != 0)
        perror("ioctl()");
        peer_addr.sll_ifindex = req.ifr_ifindex;
        peer_addr.sll_protocol = htons(ETH_P_ARP);
    //peer_addr.sll_family = AF_PACKET;
    while (1)
    {
        rtval = send_arp(sockfd, &peer_addr);
        if (FAILURE == rtval)
        {
            fprintf(stderr, "Send arp socket failed: %s\n", strerror(errno));
        }
        rtval = recv_arp(sockfd, &peer_addr);
        if (rtval == SUCCESS)
        {
            printf ("Get packet from peer and IP conflicts!\n");
        }
        else if (rtval == FAILURE)
        {
            fprintf(stderr, "Recv arp IP not conflicts: %s\n", strerror(errno));
        }
        else
        {
            fprintf(stderr, "Recv arp socket failed: %s\n", strerror(errno));
        }
        //sleep(1);
    }
    return 0;
}
//////////////////////////////////////////////////////////////////////////
// ������: send_arp
// ���� : ���ARP���ݰ����Ĳ����ͳ�ȥ��
// ����:
//    [in] sockfd -- ������socket������;
//    [in] peer_addr -- �Զ˵�IP��Ϣ
// ����ֵ:
//    �ɹ�: SUCCESS, ʧ��: FAILURE;
// ˵��:
//////////////////////////////////////////////////////////////////////////
int send_arp(int sockfd, struct sockaddr_ll *peer_addr)
{
    int rtval;
    ARP_PACKET_OBJ frame;
    memset(&frame, 0x00, sizeof(ARP_PACKET_OBJ));

    //�����̫��ͷ��
        memcpy(frame.eh.ether_dhost, dst_mac, 6);    //Ŀ��MAC��ַ
        memcpy(frame.eh.ether_shost, src_mac, 6);    //ԴMAC��ַ
        frame.eh.ether_type = htons(ETH_P_ARP);      //Э��

    //���ARP����ͷ��
        frame.arp.ea_hdr.ar_hrd = htons(ARPHRD_ETHER);    //Ӳ������
        frame.arp.ea_hdr.ar_pro = htons(ETHERTYPE_IP);    //Э������ ETHERTYPE_IP | ETH_P_IP
        frame.arp.ea_hdr.ar_hln = 6;                //Ӳ����ַ����
        frame.arp.ea_hdr.ar_pln = 4;                //Э���ַ����
        frame.arp.ea_hdr.ar_op = htons(ARPOP_REQUEST);    //ARP�������
        memcpy(frame.arp.arp_sha, src_mac, 6);    //ԴMAC��ַ
        memcpy(frame.arp.arp_spa, src_ip, 4);     //ԴIP��ַ
        memcpy(frame.arp.arp_tha, dst_mac, 6);    //Ŀ��MAC��ַ
        memcpy(frame.arp.arp_tpa, dst_ip, 4);     //Ŀ��IP��ַ

        rtval = sendto(sockfd, &frame, sizeof(ARP_PACKET_OBJ), 0,
        (struct sockaddr*)peer_addr, sizeof(struct sockaddr_ll));
    if (rtval < 0)
    {
        return FAILURE;
    }
    return SUCCESS;
}

//////////////////////////////////////////////////////////////////////////
// ������: recv_arp
// ���� : ����ARP�ظ����ݱ��Ĳ��ж��ǲ��Ƕ����ARP�Ļظ���
// ����:
//    [in] sockfd -- ������socket������;
//    [in] peer_addr -- �Զ˵�IP��Ϣ
// ����ֵ:
//    �ɹ�: SUCCESS, ʧ��: FAILURE;
// ˵��:
//    ���Ƕ����arp����Ļظ��򷵻�:SUCCESS.
//////////////////////////////////////////////////////////////////////////
int recv_arp(int sockfd, struct sockaddr_ll *peer_addr)
{
    int rtval;
    ARP_PACKET_OBJ frame;

    memset(&frame, 0, sizeof(ARP_PACKET_OBJ));
    rtval = recvfrom(sockfd, &frame, sizeof(frame), 0,
        NULL, NULL);
    //�ж��Ƿ���յ����ݲ����Ƿ�Ϊ��Ӧ��
    if (htons(ARPOP_REPLY) == frame.arp.ea_hdr.ar_op && rtval > 0)
    {
        //�ж�Դ��ַ�Ƿ�Ϊ��ͻ��IP��ַ
        if (memcmp(frame.arp.arp_spa, src_ip, 4) == 0)
        {
            fprintf(stdout, "IP address is common~\n");
            return SUCCESS;
        }
    }
    if (rtval < 0)
    {
        return FAILURE;
    }
    return FAILURE;
}
