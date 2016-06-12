#include <iostream>
#include "cping.h"
#include "mytime.h"
#include "dd.h"
#include "freearp.h"
extern "C"{
#include "args.h"
}
#include <stdio.h>
#include <time.h>
#include <list>
using namespace std;
#if WIN32
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
int getPidBySid(int sid,list<int>*pidlist);
#endif
int checktime=20;
char VER[28]="v1.82-(2016/6/12)";
int FindIP(char *mac,char *ip);
char backhomecmd[1024]="cmd.exe";//���ؼ�
char gohomecmd[1024]="cmd.exe";//�뿪��
char mac[30]={0};
char btmac[30]={0};//����mac
//char ip[30]={0};
//-config[BackHomeCmd:"",GoHomeCmd:cmd.exe,Mac:xxx,IP:""]
int lastinfo=-1;
int reloadarp=0;//�Ƿ�ǿ��ˢ��arp��
int ble=0;//�����豸���ͣ�Ĭ��0��ͨ�豸��1le�豸
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

    list<int>pidlist;
    getPidBySid(828,&pidlist);
    CPing ping;
    int xx=ping.PingCheckV3("192.168.43.134");
    printf("xx:%d\r\n",xx);
    if(strlen(btmac)==0&&strlen(mac)==0){
        printf("use  -mac  -bcmd -gcmd  [-reloadarp] or -bmac  -bcmd -gcmd\r\n");
        return -1;
    }
    while(true){
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
            //����wifi
            if(info==1){
                 printf("backhome\r\n");
                 popen(backhomecmd, "r");

            }else{
                //�뿪wifi
                printf("gohome\r\n");
                 popen(gohomecmd, "r");
            }
            lastinfo=info;
        }
        sleeps(checktime*1000);//ms
    }
    return 0;
}

/*��������Ƿ��ڸ��� �ֻ�*/
bool CheckBtMac(char *btmac){
     char btcmd[255]={0};
     tolower(btmac);
     //��ȡ����
     sprintf(btcmd,"hcitool name %s",btmac);
      printf("btcmd:%s\r\n",btcmd);
     FILE  *stream=popen(btcmd, "r");
     char   buf[1024]={0};
     fgets( buf, 1024,   stream);  //���ո�FILE* stream����������ȡ��buf��
     pclose(stream);
     printf("buf:%s\r\n",buf);
     //�з����з���
     if(strlen(buf)>0)
     {
         return true;
     }
     return false;
}

/*��������Ƿ��ڸ���  �ֻ�*/
bool CheckBtMacLe(char *btmac){
     char btcmd[255]={0};
     char   buf[1024]={0};
     tolower(btmac);

     FILE  *stream1=popen("hciconfig hci0 down", "r");
     fgets( buf, 1024,   stream1);
      sleeps(2);
     FILE  *stream2=popen("hciconfig hci0 up", "r");
     fgets( buf, 1024,   stream2);
     sleeps(2);
     //����ble
     sprintf(btcmd,"hcitool lecc %s",btmac);
     printf("btcmd:%s\r\n",btcmd);
     FILE  *stream=popen(btcmd, "r");
     memset(buf,0,1024);
   //  fread( buf, sizeof(char),1024,  stream);  //���ո�FILE* stream����������ȡ��buf��
      fgets( buf, 1024,   stream);
      printf("buf:%s\r\n",buf);
      char bthandle[10]={0};
      if(sscanf(buf," %[0-9]",bthandle)!=-1)
      {
          //�Ͽ�ble
          pclose(stream);
          return true;
      }
     //��������Ῠ��
     pclose(stream);
     return false;
}
/*��������Ƿ��ڸ���  �ֻ���һ�ַ���*/
bool CheckBtMacLeV2(char *btmac){
      printf("CheckBtMacLeV2\r\n");
     char btcmd[255]={0};
     char mac[30]={0};
     char btname[30]={0};
     tolower(btmac);
     //����ble
     sprintf(btcmd,"hcitool lescan");
     FILE  *stream=popen(btcmd, "r");
     char   buf[1024]={0};

           // sleep(10);

                #if WIN32

                #else
                int pid=getPidByName((char*)"hcitool");
                kill(pid,SIGTERM);//SIGTERM SIGKILL
                #endif
                fgets(buf,1024,stream);  //���ո�FILE* stream����������ȡ��buf��
                memset(mac,0,30);
                memset(btname,0,30);
                printf("buf:%s\r\n",buf);
                if(sscanf(buf,"%s %s",mac,btname)!=-1)
                {
                    printf("scanbtmac:%s\r\n",mac);
                    tolower(mac);
                    if(strncmp(btmac,mac,strlen(btmac))==0){
                        pclose(stream);
                        return true;
                    }
                }

     pclose(stream);
     return false;
}

/*���mac��ַ�Ƿ������� ����ping��Ӧ*/
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
                memcpy(prefix_ip,ip,prefix_pos-ip);//��ȡǿ��
                for(int i=1;i<255;i++){
                    sprintf(ip,"%s.%d",prefix_ip,i);
                    //����Ƿ���arp��
                    if(CheckArpIp(ip)!=0){
                        ping.PingScanf(ip);
                    }
                }
              }

           }else{
               printf("ipeerr:%s\r\n",ip);
           }

        }
        //���
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
        //rootȨ��
        if(uid==0){
                return ping.PingCheckV3(destip);
        }else{
            return ping.PingCheckV2(destip);
        }
        #endif
    }
    return false;
}


/*��ȡmac��ѯIP*/
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
        //������IP
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
     char filepath[50];//��С���⣬��װ��cmdline�ļ���·������
     char cur_task_name[50];//��С���⣬��װ��Ҫʶ����������ı�����
     char buf[512];
     dir = opendir("/proc"); //��·��
     int pid=0;
     if (NULL != dir)
     {
         while ((ptr = readdir(dir)) != NULL) //ѭ����ȡ·���µ�ÿһ���ļ�/�ļ���
         {
             if (DT_DIR != ptr->d_type)
                continue;
             if(sscanf(ptr->d_name,"%d",&pid)>0)
             {
                 sprintf(filepath, "/proc/%s/status", ptr->d_name);//����Ҫ��ȡ���ļ���·��
                 fp = fopen(filepath, "r");//���ļ�
                 if (NULL != fp)
                 {
                     if( fgets(buf, 512-1, fp)== NULL ){
                        fclose(fp);
                        continue;
                    }
                    sscanf(buf, "%*s %s", cur_task_name);
                    //����ļ���������Ҫ�����ӡ·�������� �����̵�PID
                    if(!strcmp(task_name, cur_task_name))
                    {
                        printf("PID:%s\n",ptr->d_name);
                        break;
                     }
                     fclose(fp);
                 }
             }
         }
         closedir(dir);//�ر�·��
     }
     return pid;
}

int getPidBySid(int Sid,list<int>*pidlist)
 {
     DIR *dir;
     struct dirent *ptr;
     FILE *fp;
     char filepath[50];//��С���⣬��װ��cmdline�ļ���·������
     int tmpsid;//��
     char buf[512];
     dir = opendir("/proc"); //��·��
     int pid=0;
     if (NULL != dir)
     {
         while ((ptr = readdir(dir)) != NULL) //ѭ����ȡ·���µ�ÿһ���ļ�/�ļ���
         {
             if (DT_DIR != ptr->d_type)
                continue;
             if(sscanf(ptr->d_name,"%d",&pid)>0)
             {
                 sprintf(filepath, "/proc/%s/stat", ptr->d_name);//����Ҫ��ȡ���ļ���·��
                 fp = fopen(filepath, "r");//���ļ�
                 if (NULL != fp)
                 {
                     if( fgets(buf, 512-1, fp)== NULL ){
                        fclose(fp);
                        continue;
                    }
                    sscanf(buf, "%*s %*s %*s %*s %*s %d .+", &tmpsid);
                    //����ļ���������Ҫ�����ӡ·�������� �����̵�PID
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
         closedir(dir);//�ر�·��
     }
     return 0;
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




