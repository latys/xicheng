#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <pthread.h>
#include "sqlite3.h"
//#define PORT 3000
char* err_msg = NULL;
sqlite3* pDB = NULL;
static int nRow, nCol;
char netsendbuf[200]={'0'};
char IP[20];
char IP_port[10];
int flag=0;
char startbuf[]={0x7E};
char endbuf[]={0x7E};
int PORT;
int sockfd;
char buf[100];
 int r_size;
 int num=0;
int callback(void* param, int n_column, char** column_value, char** column_name)
{
    int i = 0;
    if(flag==1)//从数据库度IP
    {
    	for( i = 0 ; i < n_column; i ++ )
    	{
      		strcpy (IP,column_value[i]);
    	}
    }
    else if(flag==2)//从数据库度端口号
    {
    	for( i = 0 ; i < n_column; i ++ )
    	{
      		strcpy (IP_port,column_value[i]);
    	}
    }
   else//从数据库读取传感器数据
   {
   	//strcpy (netsendbuf,startbuf);
    	for( i = 0 ; i < n_column; i ++ )
    	{
      		strcat (netsendbuf,column_value[i]);
      		
    	}
    	strcat (netsendbuf,endbuf);
    }
    return 0;
}
void *thrd_write(void *arg)       //创建写线程
{
	 while(1)
 	{
       		sqlite3_exec(pDB, "select  data  from  iot where flag=1",callback, NULL, &err_msg);
       		if(strlen(netsendbuf)>10)
      	 	{
 			write(sockfd,netsendbuf,strlen(netsendbuf));
 		}
 		printf(netsendbuf);
 		memset(netsendbuf,0,200);
 		num=num+1;
 		if(num>=100)
 		{
 			 char cmdd[100]="update iot set flag=0"; 
 			 sqlite3_exec(pDB, cmdd, NULL, NULL, &err_msg);
 			 num=0;
 		}
 		sleep(5);
	 } 
	 pthread_exit(NULL);
}


int main(int argc, char *argv[])
{
	
 	struct sockaddr_in server_addr;
 	struct hostent *host;
 	start:
 	if(sqlite3_open("/usr/local/wangguan.db", &pDB)!=SQLITE_OK)
  	  {
                 printf("open database  falie");
                 exit(1);
 	   }
 	 flag=1;
 	 sqlite3_exec(pDB, "select  IP  from  desIP",callback, NULL, &err_msg);
 	 flag=2;
 	 sqlite3_exec(pDB, "select  port  from  desIP",callback, NULL, &err_msg);
 	 PORT=(IP_port[0]-0x30)*1000+(IP_port[1]-0x30)*100+(IP_port[2]-0x30)*10+(IP_port[3]-0x30);
 	 flag=0;
	 sockfd = socket(AF_INET,SOCK_STREAM,0);
	 if(sockfd==-1)
 	{
  			fprintf(stderr,"Socket error:%s\n",strerror(errno));
 	 		exit(1);
 	}

 	bzero(&server_addr,sizeof(server_addr));
 	server_addr.sin_family=AF_INET;
 	server_addr.sin_port=htons(PORT);
 	server_addr.sin_addr.s_addr=inet_addr(IP);
 	int con_flag,res2;
	pthread_t thread_write;
	pthread_t thread_read;
 	start1:
 	con_flag=connect(sockfd,(struct sockaddr *)(&server_addr),sizeof(struct sockaddr));   //连接服务端
	if(con_flag<0)
 	{
 		goto start1;
	}
	 else
	 {
		printf("Has connection Server %s\n",inet_ntoa(server_addr.sin_addr));
  		res2=pthread_create(&thread_write,NULL,thrd_write,NULL);   
  		if(res2<0)
  		{
  				printf("create pthread falied");
  		}
  		else
  				printf("create pthread sucess");
  		while(1)  //读服务端信息
 		{	
 		 	r_size=read(sockfd,buf,100);
   			if(r_size>0)
   			{	
   				 /*char cmd[100]="update desIP set IP='"; 
   				 char cmd1[100]="',port="; 
   				 strncpy(cmd,buf,5);
   				 strncpy(cmd,cmd1,13);*/
   				 printf("###############");
   				 printf("%s",buf);
   				 //sqlite3_exec(pDB, buf, NULL, NULL, &err_msg);
   			}
   			
			else
   			 {
  				
    				printf("Server has exit!\n");
				close(sockfd);
				goto start;
   			 } 
		}
	 }
 	close(sockfd);
 	close(pDB);
 	return 0;
}
