#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <netinet/tcp.h>
#include "sqlite3.h"
#define MAXLINE 100;


char* err_msg = NULL;
sqlite3* pDB = NULL;
static int nRow, nCol;
char netsendbuf[200]={'0'};
char IP[20];
char IP_port[10];
char PL[5];
int flag=0;
char startbuf[]={0x7E};
char endbuf[]={0x7E};
int PORT;
int sockfd;
char buf[100];
int r_size;
int num;
int keepalive=1;
int keepidle=60;
int keepintvl=10;
int keepcnt=3;
int *clientfdp;
int isconnect=0;
pthread_t tid1,tid2;

struct sockaddr_in serveraddr;

void *threadsend(void *vargp);
void *threadrecv(void *vargp);

int same_str(char * str1,char * str2,int len)            //查找相同字符
{
   int i=0;
   int pos;
   int j;
  
   
     for( j=0;j<sizeof(str1);j++)
    {
      if(str1[j]==str2[0])
       {pos=j;
       break;
       }
    }
    for( j=pos;i<len;j++)
    {
       if(str1[j]!=str2[i])
         return 0;
       i++;
     }
  
   return 1;

}

int connect_callback(void* param, int n_column, char** column_value, char** column_name)
{
    int i = 0;
    //从数据库度IP
    {
    	for( i = 0 ; i < n_column; i ++ )
    	{
      		strcpy (IP,column_value[0]);
                PORT=atoi(column_value[1]);

	        *clientfdp = socket(AF_INET,SOCK_STREAM,0);
                serveraddr.sin_port = htons(PORT);
	        serveraddr.sin_addr.s_addr = inet_addr(IP);
                printf("IP:%s,Port:%d\r\n",IP,PORT); 
	        if(connect(*clientfdp,(struct sockaddr *)&serveraddr,sizeof(serveraddr)) < 0)
	        {
                 isconnect=0;
                 
       		 
	        }
               else
               {
                printf("connect ok\r\n");            
               //j pthread_create(&tid1,NULL,threadsend,clientfdp);
		
               // pthread_join(tid1,NULL);
               }

    	}
    }
     return 0;
}

int getpl_callback(void* param, int n_column, char** column_value, char** column_name)
{
   int i=0;
      //从数据库读频率
   {
    for( i = 0 ; i < n_column; i ++ )
    	{
      		strcpy (PL,column_value[i]);
    	}

   }
 return 0;
}

int getdata_callback(void* param, int n_column, char** column_value, char** column_name)
{
int i=0;
   //从数据库读取传感器数据
   {
   	//strcpy (netsendbuf,startbuf);
    	for( i = 0 ; i < n_column; i ++ )
    	{
      		strcat (netsendbuf,column_value[i]);
      		
    	}
	printf("netsendbuf:%s\r\n",netsendbuf);
    	strcat (netsendbuf,endbuf);
    }
    return 0;
}
int main()
{

	
	clientfdp = (int *)malloc(sizeof(int));
        start:
	*clientfdp = socket(AF_INET,SOCK_STREAM,0);
         if(sqlite3_open("/usr/local/wangguan.db", &pDB)!=SQLITE_OK)
  	  {
                 printf("open database  falie");
                 exit(1);
 	   }
 
 	
 	 flag=0;
	
	struct hostent *hp;
	bzero((char *)&serveraddr,sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(PORT);
	serveraddr.sin_addr.s_addr = inet_addr(IP);
        
	while(1)
	{
         
 	   sqlite3_exec(pDB, "select  SERVERIP,DATAPORT  from  Zhuanfa_shebei",connect_callback, NULL, &err_msg);
   
sqlite3_stmt  *stmt = NULL;
 
    rc = sqlite3_prepare_v2(pdb , strsql , strlen(strsql) , &stmt , NULL);
    if(rc != SQLITE_OK)
    {
        if(stmt)
        {
            sqlite3_finalize(stmt);
        }
        sqlite3_close(pdb);
        return -1;
    }
    int nColumn = sqlite3_column_count(stmt);
    int vtype , i;
    do{ 
        rc = sqlite3_step(stmt);
        if(rc == SQLITE_ROW)
        {
             
            for(i = 0 ; i < nColumn ; i++ )
            {
             
                vtype = sqlite3_column_type(stmt , i);
                if(vtype == SQLITE_INTEGER)
                {
                    printf("%s : %d \n" , sqlite3_column_name(stmt , i) , sqlite3_column_int(stmt , i));
                }
                else if(vtype == SQLITE_TEXT)
                {
                    printf("%s : %s \n" , sqlite3_column_name(stmt , i) , sqlite3_column_text(stmt , i));
                }
                else if(vtype == SQLITE_NULL)
                {
                    printf("no values\n");
                }
            }
            printf("\n****************\n");
             
        }
        else if(rc == SQLITE_DONE)
        {
            printf("Select finish\n");
            break;
        }
        else
        {
            printf("Select faile\n");
            sqlite3_finalize(stmt);
            break;
             
        }
     
         
    }while(1);
    sqlite3_finalize(stmt);      
                flag=3;                //查频率
                sqlite3_exec(pDB,"select pl from desIP",getpl_callback, NULL, &err_msg);
                int pl=atoi(PL);
                flag=0;  
                //printf("%d",pl);
 	        sleep(pl);
        
               
		pthread_create(&tid1,NULL,threadsend,clientfdp);
		//pthread_create(&tid2,NULL,threadrecv,clientfdp);
                pthread_join(tid1,NULL);
                //pthread_join(tid2,NULL);
               
              //  if(recv(*clientfdp,buf,0,0)<0)
              //   printf("disconnect\n");
              //  connect(*clientfdp,(struct sockaddr *)&serveraddr,sizeof(serveraddr));
	}

	return EXIT_SUCCESS;
}




void *threadsend(void * vargp)
{
	//pthread_t tid2;
	int connfd = *((int *)vargp);

	int idata;
	char temp[100];
        
      
        
	//while(1)
	{      
		flag=4;
                //printf("send.............");
                sqlite3_exec(pDB, "select  data  from  iot where flag=1",getdata_callback, NULL, &err_msg);
       		if(strlen(netsendbuf)>5)
      	 	{
 			send(connfd,netsendbuf,strlen(netsendbuf),0);
 		}
 		//printf(netsendbuf);
 		memset(netsendbuf,0,200);
                flag=0;
                
                {
                close(connfd);
                pthread_exit(NULL);
                }

	}


	printf("client send\n");
	return NULL;
}


void *threadrecv(void *vargp)
{
	char temp[100]={'0'};
	int connfd = *((int *)vargp);
        printf("recv...........");
	while(1)
	{
		int idata = 0;
		idata = recv(connfd,temp,100,0);
		if(idata > 0){
                 sqlite3_exec(pDB, temp, NULL, NULL, &err_msg);
		printf("server :\n%s\n",temp);
                 if(same_str(temp,"update desIP set IP",19)==1)
                    {
                close(connfd);
                printf("disconnect\n");
                isconnect=0;
                pthread_exit(NULL);
               }
                
                   
                } 
                if(idata<=0)
                {
                close(connfd);
                printf("disconnect\n");
                isconnect=0;
                pthread_exit(NULL);
               }
                
	}



	return NULL;
}
