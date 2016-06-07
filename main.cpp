#include <iostream>
#include "cping.h"
#include "mytime.h"
#include "dd.h"
#include "freearp.h"
#include <stdio.h>
#include <time.h>
#include <list>
#if WIN32
extern "C"{
#define STATIC_GETOPT 1
#include "getopt.h"

}
#include <windows.h>
#include <Iphlpapi.h>
#include <WinSock2.h>
#else
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
#endif
using namespace std;
int checktime=60;
char VER[28]="v1.7-(2016/6/7)";
int FindIP(char *mac,char *ip);
char backhomecmd[1024]="cmd.exe";//返回家
char gohomecmd[1024]="cmd.exe";//离开家
char mac[30]={0};
char btmac[30]={0};//蓝牙mac
//char ip[30]={0};
//-config[BackHomeCmd:"",GoHomeCmd:cmd.exe,Mac:xxx,IP:""]
int lastinfo=-1;
int reloadarp=0;//是否强制刷新arp表
int ble=0;//蓝牙设备类型，默认0普通设备，1le设备
bool CheckMac(char *mac);
bool CheckBtMac(char *btmac);
bool CheckBtMacLe(char *btmac);
bool CheckBtMacLeV2(char *btmac);
bool CheckMacV2(char *mac);
int getlocalip(list<string>*iplist);
int CheckArpIp(char *DestIP);
int GetIPType(const char * ipAddress);
void tolower(char *str);
int main(int argc, char *argv[])
{
    printf("smarthome %s\r\n",VER);
    struct option long_options[] = {
    { "mac", 1, NULL, 'm'},
    { "gcmd", 1, NULL, 'g' },
    { "bcmd", 1, NULL, 'b' },
    { "bmac", 1, NULL, 't' },
    { "ble", 1, NULL, 'l' },
    { "reloadarp", 1, NULL, 'r' },
    {0, 0, 0, 0}//必须保留，不然不存在会崩溃
    };
    int c;
    while((c = getopt_long_only(argc, argv, "m:i:b:g:", long_options, NULL)) != -1)
    {
        switch (c)
        {
            case 'm':
                memset(mac,0,30);
                memcpy(mac,optarg,strlen(optarg));
                break;
            case 'b':
                memset(backhomecmd,0,1024);
                memcpy(backhomecmd,optarg,strlen(optarg));
                break;
            case 'g':
                memset(gohomecmd,0,1024);
                memcpy(gohomecmd,optarg,strlen(optarg));
                break;
             case 't':
                memset(btmac,0,30);
                memcpy(btmac,optarg,strlen(optarg));
                break;
             case 'r':
                sscanf(optarg,"%d",&reloadarp);
             case 'l':
                sscanf(optarg,"%d",&ble);
            default:
                printf("use  -mac  -bcmd -gcmd [-reloadarp]  or -bmac  -bcmd -gcmd \r\n");
        }
    }

    if(strlen(btmac)==0&&strlen(mac)==0){
        printf("use  -mac  -bcmd -gcmd  [-reloadarp] or -bmac  -bcmd -gcmd\r\n");
        return -1;
    }
    while(true){
        int info=0;
        if(strlen(btmac)>0)
        {
            if(ble==1){
                 info=(int)CheckBtMacLe(btmac);
            }
            else if(ble==2){
                info=(int)CheckBtMacLeV2(btmac);
            }
            else
            {
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
                 popen(backhomecmd, "r");

            }else{
                //离开wifi
                printf("gohome\r\n");
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
     tolower(btmac);
     //读取名字
     sprintf(btcmd,"hcitool name %s",btmac);
     FILE  *stream=popen(btcmd, "r");
     char   buf[1024]={0};
     fread( buf, sizeof(char), sizeof(buf),  stream);  //将刚刚FILE* stream的数据流读取到buf中
     pclose(stream);
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
     tolower(btmac);
     //连接ble
     sprintf(btcmd,"hcitool lecc %s",btmac);
     FILE  *stream=popen(btcmd, "r");
     char   buf[1024]={0};
     fread( buf, sizeof(char), sizeof(buf),  stream);  //将刚刚FILE* stream的数据流读取到buf中
     char bthandle[10]={0};
      if(sscanf(buf," %[0-9]",bthandle)!=-1)
      {
          //断开ble
          sprintf(btcmd,"hcitool ledc %s",bthandle);
          popen(btcmd, "r");
          pclose(stream);
          return true;
      }
     pclose(stream);
     return false;
}
/*检测蓝牙是否在附近  手环另一种方法*/
bool CheckBtMacLeV2(char *btmac){
     char btcmd[255]={0};
     char mac[30]={0};
     char btname[30]={0};
     tolower(btmac);
     //连接ble
     sprintf(btcmd,"hcitool lescan");
     FILE  *stream=popen(btcmd, "r");
     char   buf[1024]={0};

       time_t t;
    int starttime;
    starttime = time(&t);
    int cutime=0;
     while(1){
        fgets(buf,1024,stream);  //将刚刚FILE* stream的数据流读取到buf中
        memset(mac,0,30);
        memset(btname,0,30);
        if(sscanf(buf,"%s %s",mac,btname)!=-1)
        {
            tolower(mac);
            if(strncmp(btmac,mac,strlen(btmac))==0){
                pclose(stream);
                return true;
                return 0;
            }
        }
        cutime= time(&t);
        //ms已过结束进程
        if(starttime+10<cutime)
        {
            #if WIN32

            #else
            int pid=getPidByName((char*)"hcitool");
            kill(pid,SIGKILL );
            #endif
        }
     }
     pclose(stream);
     return false;
}

/*检测mac地址是否在内网 依赖ping响应*/
bool CheckMac(char *mac){
    CPing ping;
    char destip[30]={0};
    tolower(mac);
    if(FindIP(destip,mac)!=0||reloadarp==1){
        list<string>iplist;
        getlocalip(&iplist);
        list<string>::iterator it;
        char ip[32]={0};
        for(it = iplist.begin();it!=iplist.end();it++){
            memset(ip,0,32);
            memcpy(ip,(*it).c_str(),strlen((*it).c_str()));
           if(GetIPType(ip)>0){
              char prefix_ip[30]={0};
              char *prefix_pos=strrchr(ip,'.');
              if(prefix_pos!=NULL){
                memcpy(prefix_ip,ip,prefix_pos-ip);//截取强最
                for(int i=1;i<255;i++){
                    sprintf(ip,"%s.%d",prefix_ip,i);
                    //检测是否在arp表
                    if(CheckArpIp(ip)!=0){
                        ping.PingScanf(ip);
                    }
                }
              }

           }else{
               printf("ipeerr:%s\r\n",ip);
           }

        }
        //清空
        iplist.clear();
     }

    sleeps(1*1000);//1ms
    //memset(ip,0,30);
    if(FindIP(destip,mac)==0){
        printf("find ip:%s\r\n",destip);
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
    tolower(DestMac);
    if(result)
    {
        return -1;
    }
    int i = 0;
    IN_ADDR ip;

    char ipstr[30]={0};
    char mac[30]={0};
    for(i=0; i < ipNetTable->dwNumEntries; i++)
    {
        ip.S_un.S_addr = ipNetTable->table[i].dwAddr;
        memset(ipstr,0,30);
        memset(mac,0,30);
        sprintf(ipstr,"%s",inet_ntoa(ip));
        sprintf(mac,"%2x:%2x:%2x:%2x:%2x:%2x",ipNetTable->table[i].bPhysAddr[0],ipNetTable->table[i].bPhysAddr[1],ipNetTable->table[i].bPhysAddr[2],ipNetTable->table[i].bPhysAddr[3],ipNetTable->table[i].bPhysAddr[4],ipNetTable->table[i].bPhysAddr[5]);
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
    for(i=0; i < ipNetTable->dwNumEntries; i++)
    {
        ip.S_un.S_addr = ipNetTable->table[i].dwAddr;
        memset(ipstr,0,30);
        sprintf(ipstr,"%s",inet_ntoa(ip));
        if(strncmp(DestIP,ipstr,strlen(ipstr))==0){
            return 0;
        }
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
    char hwtype[30]={0};
    char Flags[30]={0};
    char Mask[30]={0};
    char Device[30]={0};
    tolower(DestMac);
    int i=0;
    while(fgets(buf,sizeof(buf),fp)!=NULL){
        memset(mac,0,30);
        memset(ip,0,30);
        if(i>0){
            sscanf(buf,"%s %s %s %s %s %s",ip,hwtype,Flags,mac,Mask,Device);
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
    char hwtype[30]={0};
    char Flags[30]={0};
    char Mask[30]={0};
    char Device[30]={0};
    int i=0;
    while(fgets(buf,sizeof(buf),fp)!=NULL){
        memset(ip,0,30);
        memset(mac,0,30);
        if(i>0){
            sscanf(buf,"%s %s %s %s %s %s",ip,hwtype,Flags,mac,Mask,Device);
            if(strncmp(ip,DestIP,strlen(ip))==0){
                return 0;
            }
        }
        i++;
    }
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

#endif // WIN32
void tolower(char *str)
{
    int i=0;
    for(i = 0; i <(int) sizeof(str); i++)
    {
           str[i] = tolower(str[i]);
    }
}



