#include <iostream>
#include "cping.h"
#include "mytime.h"
#include <stdio.h>
#include <time.h>
#if WIN32
extern "C"{
#define STATIC_GETOPT 1
#include "getopt.h"

}
#include <windows.h>
#include <Iphlpapi.h>
#else
#include <stdlib.h>
#include <getopt.h>
#endif
using namespace std;
int checktime=60;
char VER[28]="v1.5-(2016/6/5)";
int FindIP(char *mac,char *ip);
char backhomecmd[1024]="cmd.exe";//返回家
char gohomecmd[1024]="cmd.exe";//离开家
char mac[30]={0};
char btmac[30]={0};//蓝牙mac
char ip[30]={0};
//-config[BackHomeCmd:"",GoHomeCmd:cmd.exe,Mac:xxx,IP:""]
int lastinfo=-1;
bool CheckMac(char *ip,char *mac);
bool CheckBtMac(char *btmac);
bool CheckMacV2(char *mac);
int main(int argc, char *argv[])
{
    printf("smarthome %s\r\n",VER);
    struct option long_options[] = {
    { "mac", 1, NULL, 'm'},
    { "ip", 1, NULL, 'i'},
    { "gcmd", 1, NULL, 'g' },
    { "bcmd", 1, NULL, 'b' },
    { "bmac", 1, NULL, 't' },
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
            case 'i':
                memset(ip,0,30);
                memcpy(ip,optarg,strlen(optarg));
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
            default:
                printf("use  -mac  -ip -bcmd -gcmd  or -bmac  -bcmd -gcmd\r\n");
        }
    }

    // printf("argc mac:%s ip :%s bcmd:%s gcmd:%s \r\n",mac,ip,backhomecmd,gohomecmd);

    while(true){
        int info=0;
        if(strlen(btmac)>0)
        {
            info=(int)CheckBtMac(btmac);
        }
        else
        {
           info=(int)CheckMac(ip,mac);
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
     //连接ble
     sprintf(btcmd,"hcitool lescan");
     FILE  *stream=popen(btcmd, "r");
     char   buf[1024]={0};
     while(1){
        fgets(buf,1024,stream);  //将刚刚FILE* stream的数据流读取到buf中
        if(sscanf(buf,"")!=-1)
        {
              pclose(stream);
              return true;
        }
     }
     pclose(stream);
     return false;
}

/*检测mac地址是否在内网 依赖ping响应*/
bool CheckMac(char *ip,char *mac){
     CPing ping;
     char prefix_ip[30]={0};
     char *prefix_pos=strrchr(ip,'.');
     if(prefix_pos!=NULL){
        memcpy(prefix_ip,ip,prefix_pos-ip);//截取强最
     }else{
         printf("ip error exit. \r\n");
         exit(0);
     }
     for(int i=1;i<255;i++){
        sprintf(ip,"%s.%d",prefix_ip,i);
        ping.PingScanf(ip);
     }
    sleeps(1*1000);//1ms
    char destip[30]={0};
    //memset(ip,0,30);
    if(FindIP(destip,mac)==0){
        printf("find ip:%s\r\n",destip);
        return ping.PingCheckV2(destip);
    }
    return false;
}

/*检测mac地址是否在内网 使用udp*/
bool CheckMacV2(char *mac){

     CPing ping;
     char prefix_ip[30]={0};
     char *prefix_pos=strrchr(ip,'.');
     if(prefix_pos!=NULL){
        memcpy(prefix_ip,ip,prefix_pos-ip);//截取强最
     }else{
         printf("ip error exit. \r\n");
         exit(0);
     }
     for(int i=1;i<255;i++){
        sprintf(ip,"%s.%d",prefix_ip,i);
        ping.PingScanf(ip);
     }
    sleeps(1*1000);//1ms
    char destip[30]={0};
    //memset(ip,0,30);
    if(FindIP(destip,mac)==0){
        printf("find ip:%s\r\n",destip);
        return ping.PingCheckV2(destip);
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
#else
int FindIP(char *DestIP,char *DestMac){
    FILE *fp = fopen("/proc/net/arp","r");
    //FILE *fp = fopen("D:\\c\\shome\\xx.txt","r");
    int i=0;
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

    while(fgets(buf,sizeof(buf),fp)!=NULL){
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
#endif
