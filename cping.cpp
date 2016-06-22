#include "cping.h"
#include <stdio.h>
CPing::CPing()
{
    #if WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2,2),&wsaData);
    #endif
    m_socket = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);

    if( m_socket == INVALID_SOCKET )
    {
        #if WIN32
        WSACleanup();
        #endif
        return;
    }

}

CPing::~CPing()
{
    if( INVALID_SOCKET != m_socket )
    {
         #if WIN32
         closesocket(m_socket);
         #else
         close(m_socket);
         #endif
    }
    #if WIN32
    WSACleanup();
    #else

    #endif
}

void CPing::Ping(std::string strAddr)
{
    hostent *host;
    host = gethostbyname(strAddr.c_str());

    if( host == NULL )
    {
        printf("gethostbyname err\n");
        return;
    }

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(0);
    addr.sin_addr = *((struct in_addr *)(host->h_addr));

    char* icmp = new char[sizeof(ICMPHDR) + DATA_SIZE];
    #if WIN32
    ZeroMemory(icmp,sizeof(ICMPHDR) + DATA_SIZE);
    #else
    memset(icmp,0,sizeof(ICMPHDR) + DATA_SIZE);
    #endif

    PICMPHDR picmp = (PICMPHDR)icmp;
    int nSequence = 0;
    int nCount = 4;
    while ( nCount-- )
    {
        InitICMP(picmp, nSequence++);

        picmp->icmp_checksum = CheckSum((u_short*)picmp,sizeof(ICMPHDR) + DATA_SIZE); //校验值

        SendData((char*)picmp,sizeof(ICMPHDR) + DATA_SIZE,&addr);

        sockaddr_in addrRecv;
        char cBuf[RECV_SIZE] = {0};

        int recvLen = 0;
        RecvData(cBuf,RECV_SIZE,&addrRecv,recvLen);
        int nHeadLen = sizeof(IPHDR) + sizeof(ICMPHDR) + DATA_SIZE;
        if( recvLen < nHeadLen )
        {
            printf("tool few data~\n");
            continue;
        }

        IPHDR *ipHead = (IPHDR *)cBuf;
        PICMPHDR icmpRecv = (PICMPHDR) (cBuf + sizeof(IPHDR));

        if( icmpRecv->icmp_type != 0 )
        {
            printf("Icmp Type err~\n");
            continue;
        }
        #if WIN32
        if( icmpRecv->icmp_id != GetCurrentProcessId() )
        #else

        if( icmpRecv->icmp_id != getpid() )
        #endif
        {
            printf("Icmp ID err~\n");
            continue;
        }

        printf("recv from %s\n",inet_ntoa(addrRecv.sin_addr));
        printf("time: %d s\n",(int)(GetTickCount() - icmpRecv->icmp_timestamp));
        printf("TTL= %u\n",ipHead->ipTTL);
    }
    getchar();
}


bool CPing::PingScanf(std::string strAddr)
{
    hostent *host;
    host = gethostbyname(strAddr.c_str());
    if( host == NULL )
    {

        printf("gethostbyname err\n");
        return false;
    }

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(0);
    addr.sin_addr = *((struct in_addr *)(host->h_addr));

    char* icmp = new char[sizeof(ICMPHDR) + DATA_SIZE];
    #if WIN32
    ZeroMemory(icmp,sizeof(ICMPHDR) + DATA_SIZE);
    #else
    memset(icmp,0,sizeof(ICMPHDR) + DATA_SIZE);
    #endif
    PICMPHDR picmp = (PICMPHDR)icmp;
    int nSequence = 0;
    InitICMP(picmp, nSequence++);
   // printf("InitICMP\r\n");
    picmp->icmp_checksum = CheckSum((u_short*)picmp,sizeof(ICMPHDR) + DATA_SIZE); //校验值
    return SendData((char*)picmp,sizeof(ICMPHDR) + DATA_SIZE,&addr);
}

bool CPing::PingCheck(std::string strAddr)
{
    hostent *host;
    host = gethostbyname(strAddr.c_str());
    if( host == NULL )
    {
        printf("gethostbyname \n");
        return false;
    }

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(0);
    addr.sin_addr = *((struct in_addr *)(host->h_addr));

    char* icmp = new char[sizeof(ICMPHDR) + DATA_SIZE];
    #if WIN32
    ZeroMemory(icmp,sizeof(ICMPHDR) + DATA_SIZE);
    #else
    memset(icmp,0,sizeof(ICMPHDR) + DATA_SIZE);
    #endif
    PICMPHDR picmp = (PICMPHDR)icmp;
    int nSequence = 0;
    InitICMP(picmp, nSequence++);
    picmp->icmp_checksum = CheckSum((u_short*)picmp,sizeof(ICMPHDR) + DATA_SIZE); //校验值
    SendData((char*)picmp,sizeof(ICMPHDR) + DATA_SIZE,&addr);

    sockaddr_in addrRecv;
    char cBuf[RECV_SIZE] = {0};

    int recvLen = 0;
    RecvData(cBuf,RECV_SIZE,&addrRecv,recvLen);
    int nHeadLen = sizeof(IPHDR) + sizeof(ICMPHDR) + DATA_SIZE;
    if( recvLen < nHeadLen )
    {
        printf("tool few data~\n");
        return false;
    }
    PICMPHDR icmpRecv = (PICMPHDR) (cBuf + sizeof(IPHDR));

    if( icmpRecv->icmp_type != 0 )
    {
        printf("Icmp Type err~\n");
         return false;
    }
    #if WIN32
    if( icmpRecv->icmp_id != GetCurrentProcessId() )
    #else
    if( icmpRecv->icmp_id != getpid() )
    #endif
    {
        printf("Icmp ID err~\n");
         return false;
    }
    return true;

}

bool CPing::PingCheckV2(std::string strAddr)
{
   hostent *host;
    host = gethostbyname(strAddr.c_str());

    if( host == NULL )
    {
        printf("gethostbyname err\n");
        return false;
    }

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(0);
    addr.sin_addr = *((struct in_addr *)(host->h_addr));

    char* icmp = new char[sizeof(ICMPHDR) + DATA_SIZE];
    #if WIN32
    ZeroMemory(icmp,sizeof(ICMPHDR) + DATA_SIZE);
    #else
    memset(icmp,0,sizeof(ICMPHDR) + DATA_SIZE);
    #endif

    PICMPHDR picmp = (PICMPHDR)icmp;
    int nSequence = 0;
    int nCount = 4;
    int statistic=0;
    while ( nCount-- )
    {
        InitICMP(picmp, nSequence++);

        picmp->icmp_checksum = CheckSum((u_short*)picmp,sizeof(ICMPHDR) + DATA_SIZE); //校验值

        SendData((char*)picmp,sizeof(ICMPHDR) + DATA_SIZE,&addr);

        sockaddr_in addrRecv;
        char cBuf[RECV_SIZE] = {0};

        int recvLen = 0;
        RecvData(cBuf,RECV_SIZE,&addrRecv,recvLen);
        int nHeadLen = sizeof(IPHDR) + sizeof(ICMPHDR) + DATA_SIZE;
        if( recvLen < nHeadLen )
        {
            printf("tool few data~\n");
            continue;
        }

        PICMPHDR icmpRecv = (PICMPHDR) (cBuf + sizeof(IPHDR));

        if( icmpRecv->icmp_type != 0 )
        {
            printf("Icmp Type err~\n");
            continue;
        }
        #if WIN32
        if( icmpRecv->icmp_id != GetCurrentProcessId() )
        #else

        if( icmpRecv->icmp_id != getpid() )
        #endif
        {
            printf("Icmp ID err~\n");
            continue;
        }

       // printf("recv from %s\n",inet_ntoa(addrRecv.sin_addr));
       // printf("time: %u s\n",(GetTickCount() - icmpRecv->icmp_timestamp));
       // printf("TTL= %u\n",ipHead->ipTTL);
       statistic++;
    }
    return statistic>0?true:false;
}

bool CPing::PingCheckV3(std::string strAddr)
{
    int icmps,udpsock;
    struct sockaddr_in icmpaddr={0};
    struct sockaddr_in udpaddr={0};
    struct sockaddr_in from={0};
    int ret;
    //icmp
    icmps = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    //udp
    udpsock=socket(AF_INET,SOCK_DGRAM,0);
    if (icmps < 0||udpsock<0) {
        return false;
    }
    //icmp
    icmpaddr.sin_addr.s_addr = inet_addr("0.0.0.0");
    icmpaddr.sin_family = AF_INET;
    icmpaddr.sin_port = htons(0);
    //udp
    udpaddr.sin_family=AF_INET;
    udpaddr.sin_port=htons(32768 + 666);//32768 + 666
    udpaddr.sin_addr.s_addr=inet_addr(strAddr.c_str());//inet_addr("192.168.2.61");
    ret = bind(icmps, (const struct sockaddr *)&icmpaddr, sizeof(sockaddr_in));
    if (ret != 0) {
        return false;
    }
    #if WIN32
    DWORD nMode=1;
    ioctlsocket( icmps, FIONBIO,&nMode);
    #else
    fcntl(icmps,F_SETFL,O_NONBLOCK);
    #endif
    int flag=1;
    setsockopt(icmps, IPPROTO_IP, IP_HDRINCL, (char*)&flag, sizeof(flag));
    unsigned short  icmp_id;
    #if WIN32
    icmp_id= GetCurrentProcessId();
    #else
    icmp_id= getpid();
    #endif
    //发送4次
    ret=sendto(udpsock,(char *)&icmp_id,2,0,(struct sockaddr *)&udpaddr,sizeof(sockaddr));
    ret=sendto(udpsock,(char *)&icmp_id,2,0,(struct sockaddr *)&udpaddr,sizeof(sockaddr));
    ret=sendto(udpsock,(char *)&icmp_id,2,0,(struct sockaddr *)&udpaddr,sizeof(sockaddr));
    ret=sendto(udpsock,(char *)&icmp_id,2,0,(struct sockaddr *)&udpaddr,sizeof(sockaddr));
    if (ret < 0)  {
        return false;
    }
    char packet[512]={0};
    fd_set fds;
	struct timeval  wait;
	socklen_t fromlen = sizeof(sockaddr);
	FD_ZERO(&fds);
	FD_SET(icmps, &fds);
	wait.tv_sec =0;
	wait.tv_usec =100*1000;
    if (select(icmps + 1, &fds, NULL, NULL, &wait) > 0){
        ret = recvfrom(icmps, (char *)packet, 512, 0, (struct sockaddr *)&from, &fromlen);
        if(ret>0){
            short backicmp_id=0;
            memcpy(&backicmp_id,packet+sizeof(IPHDR)+sizeof(ICMPHDR)+sizeof(IPHDR)+4,2);
            if(icmp_id==backicmp_id){
                return true;
            }
        }
    }
    return false;
}

bool CPing::SendData(char* buf,int nBufLen,sockaddr_in* pAddr)
{
    if( pAddr == NULL )
    {
        //printf("Addr == NULL\n");
        return false;
    }

    #if WIN32
    int timeOut = 1000;
    #else
    struct timeval timeOut={1,0};//1s
    #endif
    int nRet = setsockopt(m_socket,SOL_SOCKET,SO_SNDTIMEO,(char*)&timeOut, sizeof(timeOut));

    if( nRet == -1 )
    {
        //printf("setsockopt SO_SNDTIMEO err\n");
        return false;
    }

    nRet = sendto(m_socket,buf,nBufLen,0,(sockaddr*)pAddr,sizeof(sockaddr));

    if( nRet == -1 )
    {
       return false;
    }

    return true;
}

bool CPing::RecvData(char* buf,int nBufLen,sockaddr_in* pRecvAddr,int &nRecvLen)
{
    if( INVALID_SOCKET == m_socket )
        return false;


    #if WIN32
    int nTimeOut = 1000;
    #else
    struct timeval nTimeOut={1,0};//1s
    #endif
    int nRet = setsockopt(m_socket,SOL_SOCKET,SO_RCVTIMEO ,(char*)&nTimeOut,sizeof(nTimeOut));

    if( -1 == nRet )
    {
        printf("setsockopt SO_RCVTIMEO err\n");
        return false;
    }

    int nAddrLen = sizeof(sockaddr);
    #if WIN32
    nRet = recvfrom(m_socket,buf,nBufLen,0,(sockaddr*)pRecvAddr,&nAddrLen);
    #else
    nRet = recvfrom(m_socket,buf,nBufLen,0,(sockaddr*)pRecvAddr,(socklen_t*)&nAddrLen);
    #endif
    if( -1 == nRet )
    {
        return false;
    }

    nRecvLen = nRet;
    return true;
}

void CPing::InitICMP(PICMPHDR icmpHDR,int nSequence)
{
    if( icmpHDR == NULL )
        return;
    icmpHDR->icmp_type = 8; //request
    icmpHDR->icmp_code = 0; //icmp request
    icmpHDR->icmp_sequence = nSequence;
    #if WIN32
    icmpHDR->icmp_id = (u_short)GetCurrentProcessId();
    #else
    icmpHDR->icmp_id = (u_short)getpid();
    #endif
    icmpHDR->icmp_timestamp = GetTickCount();
    icmpHDR->icmp_checksum = 0; //校验值
}

u_short CPing::CheckSum(u_short *pBuf,int nLen)
{
    USHORT cksum=0;
    while(nLen>1)
    {
        cksum+=*pBuf++;
        nLen-=sizeof(USHORT);
    }
    if(nLen)
        cksum+=*pBuf++;
    cksum=(cksum>>16)+(cksum&0xffff);
    cksum+=(cksum>>16);
    return (USHORT)(~cksum);
}


#if WIN32
#else
unsigned long GetTickCount()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}
#endif // WIN32
