#include <iostream>
#include "cping.h"
#include "mytime.h"
#include "freearp.h"
extern "C"{
#include "args.h"
}
#include <stdio.h>
#include <time.h>
#include <list>
using namespace std;
#if WIN32
#include<ws2tcpip.h>
#include <windows.h>
#include <Iphlpapi.h>
#include <WinSock2.h>
#else
#include <arpa/inet.h>
/*net bios*/
#define NCB_POST void CALLBACK
typedef struct _NCB {
  unsigned char  ncb_command;
  unsigned char   ncb_retcode;
  unsigned char   ncb_lsn;
  unsigned char   ncb_num;
  unsigned char * ncb_buffer;
  short int   ncb_length;
  unsigned char   ncb_callname[16];
  unsigned char   ncb_name[16];
  unsigned char   ncb_rto;
  unsigned char   ncb_sto;
  void *ncb_post;
  unsigned char   ncb_lana_num;
  unsigned char   ncb_cmd_cplt;
  unsigned char   ncb_reserve[10];
  void * ncb_event;
} NCB, *PNCB;
#include <net/if.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdio.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <linux/sockios.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <signal.h>
int getPidByName(char* task_name);
int getPidBySid(int sid,list<int>*pidlist);

#endif
int checktime=55;
char VER[28]="v1.93-(2016/6/25)";
int FindIP(char *mac,char *ip);
char backhomecmd[1024]="cmd.exe";//返回家
char gohomecmd[1024]="cmd.exe";//离开家
char mac[30]={0};
char btmac[30]={0};//蓝牙mac

char ip[30]={0};
//-config[BackHomeCmd:"",GoHomeCmd:cmd.exe,Mac:xxx,IP:""]
int lastinfo=-1;
int reloadarp=0;//是否强制刷新arp表
int ble=0;//蓝牙设备类型，默认0普通设备，1le设备
bool CheckMac(char *mac);
bool CheckBtMac(char *btmac);
bool CheckBtMacLe(char *btmac);
bool CheckBtMacLeV2(char *btmac);
int getlocalip(list<string>*iplist);
int CheckArpIp(char *DestIP);
int GetIPType(const char * ipAddress);
void strtolower(char *str);
int UdpScan(char *ip);
int PingScan(char *scanip);
int DnsScan(char *scanip);
int GetArpTable();
int DnsGetName(char *ip,char *name);
int NetbiosGetName(char *ip,char *name);
int ScanType=0;
void write_log_file(char* filename, char* buffer);
int main(int argc, char *argv[])
{
    printf("smarthome %s\r\n",VER);
    fflush(stdout);
     char arg[255]={0};
     if(getArgValue(argc, argv,(char*) "-mac",arg)==0){
        sprintf(mac,"%s",arg); // "mac"
     }
     if(getArgValue(argc, argv,(char*) "-gcmd",arg)==0){
        sprintf(gohomecmd,"%s",arg); // "gohomecmd"
     }
     if(getArgValue(argc, argv,(char*) "-bcmd",arg)==0){
        sprintf(backhomecmd,"%s",arg); // "backhomecmd"
     }
     if(getArgValue(argc, argv,(char*) "-ble",arg)==0){
         sscanf(arg,"%d",&ble); // "ble"
     }
     if(getArgValue(argc, argv,(char*) "-reloadarp",arg)==0){
        sscanf(arg,"%d",&reloadarp); // "reloadarp"
     }
     if(getArgValue(argc, argv,(char*) "-bmac",arg)==0){
        sprintf(btmac,"%s",arg); // "btmac"
     }
     if(getArgValue(argc, argv,(char*) "-ip",arg)==0){
        sprintf(ip,"%s",arg); // "btmac"
     }
     if(getArgValue(argc, argv,(char*) "-scantype",arg)==0){
        sscanf(arg,"%d",&ScanType);  // "btmac"
     }

    if(strlen(btmac)==0&&strlen(mac)==0){
        printf("use  -mac  -bcmd -gcmd  [-reloadarp] or -bmac  -bcmd -gcmd\r\n");
        return -1;
    }

    while(true){
             CPing ping;

       //  int xx=ping.PingCheckV3("192.168.8.135");
          int xx1=ping.PingCheckV3("192.168.8.135");
        printf(" xx1:%d\r\n",xx1);
             sleeps(10*1000);//ms
        continue;
        int info=0;
        if(strlen(btmac)>0)
        {
            if(ble==1){
                    printf("ble1\r\n");
                 info=(int)CheckBtMacLe(btmac);
            }
            else if(ble==2){
                printf("ble2\r\n");
                info=(int)CheckBtMacLeV2(btmac);
                 printf("ble22\r\n");
            }
            else
            {
                  printf("noble:%d\r\n",ble);
                info=(int)CheckBtMac(btmac);

            }
        }
        else
        {
            info=(int)CheckMac(mac);
        }

        if(info!=lastinfo||lastinfo==-1){
            //进入wifi
            if(info==1){
                 printf("backhome\r\n");
                 write_log_file("log.log","backhome\r\n");
                 popen(backhomecmd, "r");

            }else{
                //离开wifi
                printf("gohome\r\n");
                write_log_file("log.log","gohome\r\n");
                 popen(gohomecmd, "r");
            }
            lastinfo=info;
        }
        sleeps(checktime*1000);//ms
    }
    return 0;
}

/*检测蓝牙是否在附近 手机*/
bool CheckBtMac(char *btmac){
     char btcmd[255]={0};
     strtolower(btmac);
     //读取名字
     sprintf(btcmd,"hcitool name %s",btmac);
      printf("btcmd:%s\r\n",btcmd);
     FILE  *stream=popen(btcmd, "r");
     char   buf[1024]={0};
     fgets( buf, 1024,   stream);  //将刚刚FILE* stream的数据流读取到buf中
     pclose(stream);
     printf("buf:%s\r\n",buf);
     //有返回有返回
     if(strlen(buf)>0)
     {
         return true;
     }
     return false;
}

/*检测蓝牙是否在附近  手环*/
bool CheckBtMacLe(char *btmac){
     char btcmd[255]={0};
     char   buf[1024]={0};
     strtolower(btmac);
     FILE  *stream1=popen("hciconfig hci0 down", "r");
     fgets( buf, 1024,stream1);
     pclose(stream1);
     sleeps(1000*2);
     FILE  *stream2=popen("hciconfig hci0 up", "r");
     fgets( buf, 1024,   stream2);
     pclose(stream2);
     sleeps(1000*3);
     //连接ble
     sprintf(btcmd,"hcitool lecc %s",btmac);
     printf("btcmd:%s\r\n",btcmd);
     FILE  *stream=popen(btcmd, "r");
     memset(buf,0,1024);
     fgets( buf, 1024,   stream);
     printf("buf:%s\r\n",buf);
     char bthandle[10]={0};
      if(sscanf(buf," %[0-9]",bthandle)!=-1)
      {
          //断开ble
          pclose(stream);
          return true;
      }
     //重启否则会卡死
     pclose(stream);
     return false;
}
/*检测蓝牙是否在附近  手环另一种方法*/
bool CheckBtMacLeV2(char *btmac){
      printf("CheckBtMacLeV2\r\n");
     char btcmd[255]={0};
     strtolower(btmac);
     //连接ble
     sprintf(btcmd,"hcitool leinfo %s",btmac);
     FILE  *stream=popen(btcmd, "r");
     char   buf[1024]={0};


    fgets(buf,1024,stream);  //将刚刚FILE* stream的数据流读取到buf中
    pclose(stream);
    printf("buf:%s\r\n",buf);
     //有返回有返回
     if(strlen(buf)>0)
     {
         return true;
     }
     return false;
}

/*检测mac地址是否在内网 依赖ping响应*/
bool CheckMac(char *mac){
    CPing ping;
    char destip[30]={0};
    strtolower(mac);
    if(FindIP(destip,mac)!=0||reloadarp==1){
        if(GetIPType(ip)>0){
                //扫描方法
                if(ScanType==0){
                    PingScan(ip);
                }else{
                    UdpScan(ip);
                }
        }
        else
        {
            list<string>iplist;
            getlocalip(&iplist);
            list<string>::iterator it;
            char ip[32]={0};
            for(it = iplist.begin();it!=iplist.end();it++){
                memset(ip,0,32);
                memcpy(ip,(*it).c_str(),strlen((*it).c_str()));
               if(GetIPType(ip)>0){
                    //扫描方法
                    if(ScanType==0){
                    PingScan(ip);
                    }else{
                         UdpScan(ip);
                    }

               }else{
                   printf("ipeerr:%s\r\n",ip);
               }

            }
            //清空
            iplist.clear();
        }
     }

    sleeps(1*1000);//1ms
    //memset(ip,0,30);
    if(FindIP(destip,mac)==0){
        printf("find ip:%s\r\n",destip);
        write_log_file("log.log","find ip:");
        write_log_file("log.log",destip);
        write_log_file("log.log","\r\n");
        #if WIN32
        return ping.PingCheckV2(destip);
        #else
        uid_t uid = getuid();
        //root权限
        if(uid==0){
                return ping.PingCheckV3(destip);
        }else{
            return ping.PingCheckV2(destip);
        }
        #endif
    }
    return false;
}


void write_log_file(char* filename, char* buffer)
{
    if (filename != NULL && buffer != NULL)
    {

        // 写日志
        {
            FILE *fp;
            fp = fopen(filename, "at+");
            if (fp != NULL)
            {
                char now[32];
                memset(now, 0, sizeof(now));
                time_t rawtime;
                struct tm* timeinfo;
                time(&rawtime);
                timeinfo = localtime(&rawtime);
                sprintf(now, "%04d-%02d-%02d %02d:%02d:%02d",
                (timeinfo->tm_year+1900), timeinfo->tm_mon, timeinfo->tm_mday,
                timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);

                fwrite(now, strlen(now)+1, 1, fp);
                fwrite(buffer, strlen(buffer), 1, fp);
                fclose(fp);
                fp = NULL;
            }
        }
    }
}

/*读取mac查询IP*/
#if WIN32
int FindIP(char *DestIP,char *DestMac)
{
    MIB_IPNETTABLE *ipNetTable = NULL;
    ULONG size = 0;
    DWORD result = 0;
    result = GetIpNetTable(ipNetTable, &size, TRUE);
    ipNetTable = (MIB_IPNETTABLE *)malloc(size);
    result = GetIpNetTable(ipNetTable, &size, TRUE);
    strtolower(DestMac);
    if(result)
    {
        return -1;
    }
    int i = 0;
    IN_ADDR ip;

    char ipstr[30]={0};
    char mac[30]={0};
    for(i=0; i < ( int )ipNetTable->dwNumEntries; i++)
    {
        ip.S_un.S_addr = ipNetTable->table[i].dwAddr;
        memset(ipstr,0,30);
        memset(mac,0,30);
        sprintf(ipstr,"%s",inet_ntoa(ip));
        sprintf(mac,"%02x:%02x:%02x:%02x:%02x:%02x",ipNetTable->table[i].bPhysAddr[0],ipNetTable->table[i].bPhysAddr[1],ipNetTable->table[i].bPhysAddr[2],ipNetTable->table[i].bPhysAddr[3],ipNetTable->table[i].bPhysAddr[4],ipNetTable->table[i].bPhysAddr[5]);
        strtolower(mac);
        if(strncmp(mac,DestMac,17)==0){
            memcpy(DestIP,ipstr,strlen(ipstr));
            return 0;
        }
    }
    return -1;
}
int CheckArpIp(char *DestIP){
    MIB_IPNETTABLE *ipNetTable = NULL;
    ULONG size = 0;
    DWORD result = 0;
    result = GetIpNetTable(ipNetTable, &size, TRUE);
    ipNetTable = (MIB_IPNETTABLE *)malloc(size);
    result = GetIpNetTable(ipNetTable, &size, TRUE);
    if(result)
    {
        return -1;
    }
    int i = 0;
    IN_ADDR ip;
    char ipstr[30]={0};
    for(i=0; i < (int)ipNetTable->dwNumEntries; i++)
    {
        ip.S_un.S_addr = ipNetTable->table[i].dwAddr;
        memset(ipstr,0,30);
        sprintf(ipstr,"%s",inet_ntoa(ip));
        if(strncmp(DestIP,ipstr,strlen(DestIP))==0){
            return 0;
        }
    }
    return -1;
}

int GetArpTable(){
     MIB_IPNETTABLE *ipNetTable = NULL;
    ULONG size = 0;
    DWORD result = 0;
    result = GetIpNetTable(ipNetTable, &size, TRUE);
    ipNetTable = (MIB_IPNETTABLE *)malloc(size);
    result = GetIpNetTable(ipNetTable, &size, TRUE);
    if(result)
    {
        return -1;
    }
    int i = 0;
    IN_ADDR ip;
    char ipstr[30]={0};
    char mac[30]={0};
    char name[30]={0};
    char zero6[6]={0};
    for(i=0; i < ( int )ipNetTable->dwNumEntries; i++)
    {
        //过滤为零的
        if(memcmp(ipNetTable->table[i].bPhysAddr,zero6,6)==0){
            continue;
        }
        ip.S_un.S_addr = ipNetTable->table[i].dwAddr;
        memset(ipstr,0,30);
        memset(mac,0,30);
        memset(name,0,30);
        sprintf(ipstr,"%s",inet_ntoa(ip));
        sprintf(mac,"%02x:%02x:%02x:%02x:%02x:%02x",ipNetTable->table[i].bPhysAddr[0],ipNetTable->table[i].bPhysAddr[1],ipNetTable->table[i].bPhysAddr[2],ipNetTable->table[i].bPhysAddr[3],ipNetTable->table[i].bPhysAddr[4],ipNetTable->table[i].bPhysAddr[5]);
        strtolower(mac);
        if(DnsGetName(ipstr,name)==-1)
            {
                NetbiosGetName(ipstr,name);
            }
        printf("ip:%s--%s--%s\r\n",ipstr,mac,name);
        fflush(stdout);

    }
    return -1;
}

#else
int FindIP(char *DestIP,char *DestMac){
    FILE *fp = fopen("/proc/net/arp","r");
    if(fp==NULL)
    {
       return -1;
    }
    char buf[1024]={0};
    char ip[30]={0};
    char mac[30]={0};
    strtolower(DestMac);
    int i=0;
    while(fgets(buf,sizeof(buf),fp)!=NULL){
        memset(mac,0,30);
        memset(ip,0,30);
        if(i>0){
            sscanf(buf,"%s %*s %*s %s %*s %*s",ip,mac);
            strtolower(mac);
            if(strncmp(mac,DestMac,17)==0){
                memcpy(DestIP,ip,strlen(ip));
                return 0;
            }
        }
        i++;
    }
    return -1;
}
int CheckArpIp(char *DestIP){
    FILE *fp = fopen("/proc/net/arp","r");
    if(fp==NULL)
    {
       return -1;
    }
    char buf[1024]={0};
    char ip[30]={0};
    char mac[30]={0};

    int i=0;
    while(fgets(buf,sizeof(buf),fp)!=NULL){
        memset(ip,0,30);
        memset(mac,0,30);
        if(i>0){
            sscanf(buf,"%s %*s %*s %s %*s %*s",ip,mac);
            if(strncmp(ip,DestIP,strlen(DestIP))==0){
                return 0;
            }
        }
        i++;
    }
    return -1;
}

int GetArpTable(){
    FILE *fp = fopen("/proc/net/arp","r");
    if(fp==NULL)
    {
       return -1;
    }
    char buf[1024]={0};
    char ip[30]={0};
    char mac[30]={0};
    int i=0;
    char name[30]={0};
    char *zeromac=(char*)"00:00:00:00:00:00";
    while(fgets(buf,sizeof(buf),fp)!=NULL){
        memset(ip,0,30);
        memset(mac,0,30);
        memset(name,0,30);
        if(i>0){
            sscanf(buf,"%s %*s %*s %s %*s %*s",ip,mac);
            strtolower(mac);
               //过滤为零的
            if(strncmp(mac,zeromac,17)==0){
                continue;
            }
            if(DnsGetName(ip,name)==-1)
            {
                NetbiosGetName(ip,name);
            }
            printf("ip:%s--%s--%s\r\n",ip,mac,name);
            fflush(stdout);
        }
        i++;
    }
    printf("deive num:%d\r\n",i);
    return -1;
}
#endif






int GetIPType(const char * ipAddress)
{
    int ipAddressList=0;
    int ipAddressList1=0;
    int ipAddressList2=0;
    int ipAddressList3=0;
    if( sscanf(ipAddress,"%d.%d.%d.%d",&ipAddressList,&ipAddressList1,&ipAddressList2,&ipAddressList3)==4){
        //局域网IP
        if (ipAddressList == 10||(ipAddressList == 172 && ipAddressList1 >= 16 && ipAddressList1 <= 31)|| (ipAddressList == 192 && ipAddressList1 == 168))
        {
            return 1;
        }
    }
    return 0;
}



#if WIN32
int getlocalip(list<string>*iplist){
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2,2),&wsaData);
    char hname[128]={0};
    struct hostent *hostinfo;
    int i;
    char ip[32]={0};
    gethostname(hname, sizeof(hname));
    if((hostinfo = gethostbyname(hname)) != NULL)
    {
        for(i = 0; hostinfo->h_addr_list[i]; i++) {
            memset(ip,0,32);
            sprintf(ip,"%s", inet_ntoa(*(struct in_addr*)(hostinfo->h_addr_list[i])));
            (*iplist).insert((*iplist).begin(),string(ip));
        }
    }
    return 0;
}
#else
int getlocalip(list<string>*iplist)
{
       int s;
       struct ifconf conf;
       struct ifreq *ifr;
       char buff[BUFSIZ];
       int num;
       int i;

       s = socket(PF_INET, SOCK_DGRAM, 0);
       conf.ifc_len = BUFSIZ;
       conf.ifc_buf = buff;

       ioctl(s, SIOCGIFCONF, &conf);
       num = conf.ifc_len / sizeof(struct ifreq);
       ifr = conf.ifc_req;
       char ip[32]={0};
       for(i=0;i < num;i++)
       {
               memset(ip,0,32);
               struct sockaddr_in *sin = (struct sockaddr_in *)(&ifr->ifr_addr);

               ioctl(s, SIOCGIFFLAGS, ifr);
               if(((ifr->ifr_flags & IFF_LOOPBACK) == 0) && (ifr->ifr_flags & IFF_UP))
               {
                       sprintf(ip,"%s",inet_ntoa(sin->sin_addr));
                       //printf("ip:%s\r\n",ip);
                       (*iplist).insert((*iplist).begin(),string(ip));
               }
               ifr++;
       }
       return 0;
}

int getPidByName(char* task_name)
 {
     DIR *dir;
     struct dirent *ptr;
     FILE *fp;
     char filepath[50];//大小随意，能装下cmdline文件的路径即可
     char cur_task_name[50];//大小随意，能装下要识别的命令行文本即可
     char buf[512];
     dir = opendir("/proc"); //打开路径
     int pid=0;
     if (NULL != dir)
     {
         while ((ptr = readdir(dir)) != NULL) //循环读取路径下的每一个文件/文件夹
         {
             if (DT_DIR != ptr->d_type)
                continue;
             if(sscanf(ptr->d_name,"%d",&pid)>0)
             {
                 sprintf(filepath, "/proc/%s/status", ptr->d_name);//生成要读取的文件的路径
                 fp = fopen(filepath, "r");//打开文件
                 if (NULL != fp)
                 {
                     if( fgets(buf, 512-1, fp)== NULL ){
                        fclose(fp);
                        continue;
                    }
                    sscanf(buf, "%*s %s", cur_task_name);
                    //如果文件内容满足要求则打印路径的名字 即进程的PID
                    if(!strcmp(task_name, cur_task_name))
                    {
                        printf("PID:%s\n",ptr->d_name);
                        break;
                     }
                     fclose(fp);
                 }
             }
         }
         closedir(dir);//关闭路径
     }
     return pid;
}

int getPidBySid(int Sid,list<int>*pidlist)
 {
     DIR *dir;
     struct dirent *ptr;
     FILE *fp;
     char filepath[50];//大小随意，能装下cmdline文件的路径即可
     int tmpsid;//大
     char buf[512];
     dir = opendir("/proc"); //打开路径
     int pid=0;
     if (NULL != dir)
     {
         while ((ptr = readdir(dir)) != NULL) //循环读取路径下的每一个文件/文件夹
         {
             if (DT_DIR != ptr->d_type)
                continue;
             if(sscanf(ptr->d_name,"%d",&pid)>0)
             {
                 sprintf(filepath, "/proc/%s/stat", ptr->d_name);//生成要读取的文件的路径
                 fp = fopen(filepath, "r");//打开文件
                 if (NULL != fp)
                 {
                     if( fgets(buf, 512-1, fp)== NULL ){
                        fclose(fp);
                        continue;
                    }
                    sscanf(buf, "%*s %*s %*s %*s %*s %d .+", &tmpsid);
                    //如果文件内容满足要求则打印路径的名字 即进程的PID
                    if(tmpsid==Sid&&pid!=Sid)
                    {
                        //printf("xxPID:%s\n",ptr->d_name);
                          (*pidlist).insert((*pidlist).begin(),pid);
                       // break;
                     }
                     fclose(fp);
                 }
             }
         }
         closedir(dir);//关闭路径
     }
     return 0;
}
#endif // WIN32
void strtolower(char *str)
{
    int i=0;
    for(i = 0; i <(int)strlen(str); i++)
    {
        str[i] = toupper(str[i]);//大写//tolower
    }
}
int NetbiosScan(char *destip){
   char *prefix_pos=strrchr(destip,'.');
   char prefix_ip[30]={0};
   if(prefix_pos!=NULL){
        memcpy(prefix_ip,destip,prefix_pos-destip);//截取强最
        sprintf(destip,"%s.%d",prefix_ip,255);
   }

   #if WIN32
   WSADATA wsaData;
   WSAStartup(MAKEWORD(2,2),&wsaData);
  #endif
  int sockfd;
  struct sockaddr_in adr_srvr;
  struct sockaddr_in adr_inet;


  adr_srvr.sin_family=AF_INET;
  adr_srvr.sin_port=htons(137);
  adr_srvr.sin_addr.s_addr =inet_addr(destip);

  char buf[50]={(char)0x82,0x28,0x00,0x00,0x00,0x01,0x00,0x00,
             0x00,0x00,0x00,0x00,0x20,0x43,0x4b,0x41,
             0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,
             0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,
             0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,
             0x41,0x41,0x41,0x41,0x41,0x00,0x00,0x21,0x00,0x1};

   adr_inet.sin_family=AF_INET;
   adr_inet.sin_port=htons(45534+rand()%100);
   adr_inet.sin_addr.s_addr=htonl(INADDR_ANY);


  memset(&(adr_srvr.sin_zero),0,8);
  memset(&(adr_inet.sin_zero),0,8);
  int z=0;
  sockfd=socket(AF_INET,SOCK_DGRAM,0);
  if(sockfd==-1){
    printf("socket error!\r\n");
  }
  int optval=true;
  setsockopt(sockfd,SOL_SOCKET,SO_BROADCAST,(char *)&optval,sizeof(optval));
  z=bind(sockfd,(struct sockaddr *)&adr_inet,sizeof(adr_inet));
   if(z==-1){
        printf("bind error!\r\n");
   }
   printf("scaning %s ...\r\n",destip);
   int sendlen=sendto(sockfd,buf,50,0,(struct sockaddr *)&adr_srvr,sizeof(adr_srvr));
   if(sendlen<1){
     printf("sendto error!\r\n");
   }
   sendto(sockfd,buf,50,0,(struct sockaddr *)&adr_srvr,sizeof(adr_srvr));
   #if WIN32
   closesocket(sockfd);
   #else
   close(sockfd);
   #endif
   return 0;
}

int UdpScan(char *destip){
   char *prefix_pos=strrchr(destip,'.');
   char prefix_ip[30]={0};
   #if WIN32
   WSADATA wsaData;
   WSAStartup(MAKEWORD(2,2),&wsaData);
   #endif
   int sockfd;
   struct sockaddr_in adr_srvr;
   struct sockaddr_in adr_inet;
   char buf[1]={0x1};

   adr_inet.sin_family=AF_INET;
   adr_inet.sin_port=htons(45534+rand()%100);
   adr_inet.sin_addr.s_addr=htonl(INADDR_ANY);


  memset(&(adr_inet.sin_zero),0,8);
  int z=0;
  sockfd=socket(AF_INET,SOCK_DGRAM,0);
  if(sockfd==-1){
    printf("socket error!\r\n");
  }
  int optval=true;
  setsockopt(sockfd,SOL_SOCKET,SO_BROADCAST,(char *)&optval,sizeof(optval));
  z=bind(sockfd,(struct sockaddr *)&adr_inet,sizeof(adr_inet));
   if(z==-1){
        printf("bind error!\r\n");
   }

    if(prefix_pos!=NULL){
        memcpy(prefix_ip,destip,prefix_pos-destip);//截取强最
         for(int i=1;i<255;i++){
            sprintf(destip,"%s.%d",prefix_ip,i);
            printf("scaning %s ...\r\n",destip);
            memset(&adr_srvr,0,sizeof(adr_srvr));
            adr_srvr.sin_family=AF_INET;
            adr_srvr.sin_port=htons(137);
            adr_srvr.sin_addr.s_addr =inet_addr(destip);
            memset(&(adr_srvr.sin_zero),0,8);
            sendto(sockfd,buf,1,0,(struct sockaddr *)&adr_srvr,sizeof(adr_srvr));
         }
   }
   #if WIN32
   closesocket(sockfd);
   #else
   close(sockfd);
   #endif
   return 0;
}

int DnsGetName(char *ip,char *name){
    struct  hostent  *lpHostEnt=NULL;
    struct in_addr ina = { 0 };
    #if WIN32
    ina.S_un.S_addr = inet_addr(ip); //获取本地主机信息
    lpHostEnt = gethostbyaddr((char*)&ina.S_un.S_addr, 4, AF_INET);
    #else
    if(!inet_aton(ip,&ina))
    {
        return -1;
    }
    lpHostEnt = gethostbyaddr((char*)&ina,4,AF_INET);
    #endif
    if(lpHostEnt!=NULL){
      sprintf(name,"%s",lpHostEnt->h_name);
      return 0;
    }
    return -1;
}
int NetbiosGetName(char *ip,char *name){
    srand((unsigned) time(NULL));
    #if WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2,2),&wsaData);
    #endif
    int sockfd;
    struct sockaddr_in adr_srvr;
      struct sockaddr_in adr_inet;
      struct sockaddr_in adr_clnt;
      #if WIN32
      int   len=sizeof(adr_clnt);
      #else
      socklen_t   len=sizeof(adr_clnt);
      #endif

      adr_srvr.sin_family=AF_INET;
      adr_srvr.sin_port=htons(137);
      adr_srvr.sin_addr.s_addr =inet_addr(ip);

      char buf[50]={(char)0x82,0x28,0x00,0x00,0x00,0x01,0x00,0x00,
             0x00,0x00,0x00,0x00,0x20,0x43,0x4b,0x41,
             0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,
             0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,
             0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,
             0x41,0x41,0x41,0x41,0x41,0x00,0x00,0x21,0x00,0x1};
       char recvbuf[1024]={0};
       adr_inet.sin_family=AF_INET;
       adr_inet.sin_port=htons(45534+rand()%100);
       adr_inet.sin_addr.s_addr=htonl(INADDR_ANY);


      memset(&(adr_srvr.sin_zero),0,8);
      memset(&(adr_inet.sin_zero),0,8);
      int z=0;
      sockfd=socket(AF_INET,SOCK_DGRAM,0);
      if(sockfd==-1){
        printf("socket error!");
        return -1;
      }
      int optval=true;
      setsockopt(sockfd,SOL_SOCKET,SO_BROADCAST,(char *)&optval,sizeof(optval));
      z=bind(sockfd,(struct sockaddr *)&adr_inet,sizeof(adr_inet));
      if(z==-1){
        return -1;
      }
    sendto(sockfd,buf,50,0,(struct sockaddr *)&adr_srvr,sizeof(adr_srvr));
    sendto(sockfd,buf,50,0,(struct sockaddr *)&adr_srvr,sizeof(adr_srvr));
    struct timeval  wait;
    fd_set fds;
	FD_ZERO(&fds);
	FD_SET(sockfd, &fds);
	wait.tv_sec =0;
	wait.tv_usec =1000*1000;
    if (select(sockfd + 1, &fds, NULL, NULL, &wait) > 0){
         recvfrom(sockfd,recvbuf,1024,0,(struct sockaddr *)&adr_clnt,&len);
         PNCB ncb = (PNCB) (recvbuf+31 );
         sprintf(name,"%s",ncb->ncb_name);
    }

     #if WIN32
      closesocket(sockfd);
      #else
      close(sockfd);
      #endif
    return 0;
}




int DnsScan(char *scanip){
    char prefix_ip[30]={0};
    char *prefix_pos=strrchr(scanip,'.');
    char tempip[30]={0};
    struct in_addr ina = { 0 };
    if(prefix_pos!=NULL){
            memcpy(prefix_ip,scanip,prefix_pos-scanip);//截取强最
            for(int i=1;i<255;i++){
                sprintf(tempip,"%s.%d",prefix_ip,i);

                printf("scan %s\r\n",tempip);
                fflush(stdout);
                //检测是否在arp表
                #if WIN32
                ina.S_un.S_addr = inet_addr(tempip); //获取本地主机信息
                gethostbyaddr((char*)&ina.S_un.S_addr, 4, AF_INET);
                #else
                inet_aton(tempip,&ina);
                gethostbyaddr((char*)&ina,4,AF_INET);
                #endif
           }
  }
  return 0;
}

int PingScan(char *scanip){
  CPing ping;
  char tempip[30]={0};
  char prefix_ip[30]={0};
  char *prefix_pos=strrchr(scanip,'.');
  if(prefix_pos!=NULL){
    memcpy(prefix_ip,scanip,prefix_pos-scanip);//截取强最
    for(int i=1;i<255;i++){
        sprintf(tempip,"%s.%d",prefix_ip,i);
        printf("scan %s\r\n",tempip);
        fflush(stdout);
        ping.PingScanf(tempip);
    }
  }
  return 0;
}
