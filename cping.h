/*

Time :2013-9-10 22:40
Title: CPing.h    (实现ping)
Author： Ryan Zhang

*/


#pragma once

#include <WinSock2.h>
#include <string>
#define DATA_SIZE 32
#define RECV_SIZE 1024

typedef struct tag_icmphdr      //icmpͷ
{
    unsigned char   icmp_type;
    unsigned char   icmp_code;
    unsigned short  icmp_checksum;
    unsigned short  icmp_id;
    unsigned short  icmp_sequence;
    unsigned long   icmp_timestamp;
} ICMPHDR, *PICMPHDR;

typedef struct tag_iphdr        //ipͷ
{
    UCHAR   iphVerLen;
    UCHAR   ipTOS;
    USHORT  ipLength;
    USHORT  ipID;
    USHORT  ipFlags;
    UCHAR   ipTTL;
    UCHAR   ipProtacol;
    USHORT  ipChecksum;
    ULONG   ipSource;
    ULONG   ipDestination;
} IPHDR;

class CPing
{
public:
    CPing();
    ~CPing();
    void Ping(std::string strAddr);
    bool PingScanf(std::string strAddr);
    bool PingCheck(std::string strAddr);
    bool SendData(char* buf,int nBufLen,sockaddr_in* pAddr);
    bool RecvData(char* buf,int nBufLen,sockaddr_in* pRecvAddr,int &nRecvLen);
    void InitICMP(PICMPHDR icmpHDR,int nSequence);
    u_short CheckSum(u_short *pBuf,int nLen);
private:
    SOCKET m_socket;

};
