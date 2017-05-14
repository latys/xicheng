#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "sqlite3.h"
#include <pthread.h>
#include "soapH.h"
#include "SynchronizedAnaDataSoapBinding.nsmap"
#define MYPORT   8000//端口号

#define BACKLOG 20     // 监听队列长度为1000

#define BUF_SIZE 8000
char temp[25];
int fd_A[BACKLOG];    // accepted connection fd
int conn_amount=0;    // 当前连接数量
int PL=10;
char IPADDR[20][25];
int  equipPort[20]={0};
char EquipPort[20][6];
char webserviceIP[20]={0};
int  webservicePort=0;
char* err_msg = NULL;
sqlite3* pDB = NULL;
static int nRow, nCol;
char netsendbuf[50]={'0'};
char  cmdchaxun[]={0x01,0x03,0x00,0x00,0x00,0x04,0x44,0x09};
char IP_ID[50][1]={0x00};
int isSaveData=0;
char CRC16[10][2]={0x44,0x09,0x44,0x3A,0x45,0xEB,0x44,0x5C,0x45,0x8D,0x45,0xBE,0x44,0x6F,0x44,0x90,0x45,0x41,0x45,0x72};


int GetPLcallback(void* param, int n_column, char** column_value, char** column_name)
{
   if(n_column>0)
     PL=atoi(column_value[0]);
}
int GetWebServiceAddrcallback(void* param, int n_column, char** column_value, char** column_name)
{
   if(n_column==2)
     {
      strcpy(webserviceIP,column_value[0]);
     webservicePort=atoi(column_value[1]);
     }
}
void *thrd_write(void *arg)       //创建个写线程
{
        int ii=0,jj=0x01;
        char buf[]={0x01,0x03,0x00,0x00,0x00,0x04,0x44,0x09};
	 while(1)
 	{
                 
		for (ii = 0; ii<conn_amount; ii++) 
		{
		       if(fd_A[ii]!=0)	
		       send(fd_A[ii], buf,sizeof(buf),0);
		       /* if(IP_ID[ii][0]!=0x00)
		        {
		        	buf[0]=IP_ID[ii][0];
		        	buf[6]=CRC16[IP_ID[ii][0]-1][0];
				buf[7]=CRC16[IP_ID[ii][0]-1][1];
				write(fd_A[ii], buf,sizeof(buf));
				sleep(10);
		        }
		       else
		       {
				for(jj=0x01;jj<0x0A;jj++)
				{
					buf[0]=jj;
					buf[6]=CRC16[jj-1][0];
					buf[7]=CRC16[jj-1][1];
					write(fd_A[ii], buf,sizeof(buf));
					sleep(10);
				}
                      }	*/		
		}
                sqlite3_exec(pDB,"select pl from desIP",GetPLcallback, NULL, &err_msg);
                
 		sleep(PL);
	 } 
	 pthread_exit(NULL);
}

int SaveDatecallback(void* param, int n_column, char** column_value, char** column_name)
{
    isSaveData=1;
    printf("isSave\r\n");
    return 0;
}

int callback(void* param, int n_column, char** column_value, char** column_name)
{
    int i = 0;
    for( i = 0 ; i < n_column; i ++ )
    {
      printf(column_value[i]);
      strcpy (netsendbuf,column_value[i]);
    }
    return 0;
}
void showclient()
{
    int i;
    printf("client amount: %d\n", conn_amount);
    for (i = 0; i < BACKLOG; i++) {
	if(fd_A[i]!=0)
        printf("[%d]:%d  ", i, fd_A[i]);
    }
    printf("\n\n");
}

int main(void)
{
	
    pthread_t thread_write;
    struct soap *soap=soap_new();
    char webserver[200]="http://192.168.1.155:8083/xcwlwWebService/services/SynchronizedAnaData";
    char cmdd[100]="select IP,Port from webserviceAddr"; 
    int sock_fd, new_fd;  // listen on sock_fd, new connection on new_fd
    struct sockaddr_in server_addr;    //服务器网络地址结构体
    struct sockaddr_in client_addr; // 客户端网络地址结构体
    socklen_t len;
    char buf[BUF_SIZE],number[10];
    int i,j,k,re,se,yes=1;
    long int retval;
    if(sqlite3_open("/usr/local/wangguan.db", &pDB)!=SQLITE_OK)
  	  {
                printf("open database  falie");
                 exit(1);
 	   }
     sqlite3_exec(pDB, cmdd, GetWebServiceAddrcallback, NULL, &err_msg);

     sprintf(webserver,"http://%s:%d/xcwlwWebService/services/SynchronizedAnaData",webserviceIP,webservicePort);
     printf("addr:%s\r\n",webserver);
	if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
		{
			perror("socket");
			exit(1);
		}

	if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) 
		{
			perror("setsockopt");
			exit(1);
		}
    
	bzero(&server_addr,sizeof(server_addr));
	server_addr.sin_family = AF_INET;         // host byte order
	server_addr.sin_port = htons(MYPORT);     // short, network byte order
	server_addr.sin_addr.s_addr = INADDR_ANY; // automatically fill with my IP	
	memset(server_addr.sin_zero, '\0', sizeof(server_addr.sin_zero));
	if (bind(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) 
		{
        		perror("bind");
			exit(1);
		}

	if (listen(sock_fd, BACKLOG) == -1) 
		{
			perror("listen");
			exit(1);
		}

	printf("listen port %d\n", MYPORT);

	fd_set fdsr;
	int maxsock;
	struct timeval tv;

	conn_amount = 0;
	maxsock = sock_fd;
	pthread_create(&thread_write,NULL,thrd_write,NULL);   
	
	
	while(1)
	{// initialize file descriptor set
		len = sizeof(struct sockaddr);
		
	    	if((new_fd=accept(sock_fd,(struct sockaddr *)&client_addr,&len))<=0)
			{
		    		perror("accept");
		    		continue;
			}
		else if((conn_amount)<BACKLOG)
		    {		
		    		fd_A[conn_amount]=new_fd;
				strcpy(IPADDR[conn_amount],inet_ntoa(client_addr.sin_addr));
                                equipPort[conn_amount]=ntohs(client_addr.sin_port);
                                sprintf(EquipPort[conn_amount],"%d",equipPort[conn_amount]);
				printf("NEW connection from client[%d] %s,port %d,socket %d\n",

				conn_amount,inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port),new_fd);
				conn_amount++;
				showclient();
		    }
		else 
		    {
			printf("max connections arrive, exit\n");
			send(new_fd, "bye", 4, 0);
			close(new_fd);
			break;
		}
	    while(1)
		{

			FD_ZERO(&fdsr);//初始化
			FD_SET(0, &fdsr);
			FD_SET(sock_fd,&fdsr);
			for (i = 0; i < BACKLOG; i++) 
				{
					if (fd_A[i] != 0) 
						{
							FD_SET(fd_A[i], &fdsr);//将sock_fd和数组fd_A中不为0的描述符放入select将检查的集合fdsr
						}
				}
			//FD_SET(new_fd,&fdsr);
			if(new_fd>maxsock)
				maxsock = new_fd;
			// timeout setting
			tv.tv_sec = 30;
			tv.tv_usec = 0;

			

			// add active connection to fd set


			retval = select(maxsock + 1, &fdsr, NULL, NULL, &tv);//selcet()函数用来探测各套接字的异常
							//如果在文件描述符集中有连接请求或发送请求，会作相应处理，
							//从而成功的解决了单线程情况下阻塞进程的情况，实现多用户连接与通信
			//printf("retval=%ld\n",retval);
			if (retval < 0) 
				{
					perror("select");
					exit(1);
				} 
			else if (retval == 0) 
				{
					printf("waiting\n");
					continue;
				}

		for (i = 0; i<conn_amount; i++) 
		{
		if(FD_ISSET(fd_A[i], &fdsr))//连接的socket上有消息到来则接收对方发过来的消息并显示
			{	
				bzero(buf,sizeof(buf)+1);
				re=recv(fd_A[i],buf,sizeof(buf),0);
				if (re<= 0) 
				{	// 关闭客户端
					printf("client[%d] close\n", i);
					close(fd_A[i]);
					IP_ID[i][0]=0x00;
					FD_CLR(fd_A[i], &fdsr);
					fd_A[i] = 0;
					conn_amount--;
					for(j=i;j<conn_amount;j++)
						fd_A[j]=fd_A[j+1];
					showclient();
					//continue;
				} 
				else 
				{
					// 接收消息
					if (re < BUF_SIZE)
						memset(&buf[re], '\0', 1);

                                         printf("%s\r\n",buf);
                                        //更新数据库 

                                        if(memcmp(buf,"insert",6)==0||memcmp(buf,"update",6)==0||memcmp(buf,"delete",6)==0)
                                        {
                                           // sqlite3_exec(pDB,buf,NULL,NULL,&err_msg);
                                        }

                                        else  //接收数据
                                        {
                                        int j=0;
                                        char hexBuf[100]={0};
                                        if(re<13)
                                              continue;
                                        int nData=0;
                                        char chData[10];
                                        nData|=buf[re-8]<<4;
                                        nData|=buf[re-7];
                                        sprintf(chData,"%d",nData);
                                       /* for( j=0;j<re;j++)
                                        {
                                           printf("%x",buf[j]);
                                           char temp[3]={0};
                                           sprintf(temp,"%02X",buf[j]);
                                           strcat(hexBuf,temp);
                                        }
                                        printf("\r\n");*/

					printf("Recieve messages from client[%d]:\n%s\n", i, buf);
					IP_ID[i][0]=buf[0];
                                        char cmd[300];
                                        sqlite3_stmt  *stmt = NULL;
                                        sprintf(cmd,"select * from jierurenzheng where equipIP='%s' and jrstate='1' and jhstate='1'",IPADDR[i]);
                                        printf(cmd);
                                        printf("\r\n");

                                        sqlite3_exec(pDB, cmd, SaveDatecallback, NULL, &err_msg);
                                       /* int rc=sqlite3_prepare_v2(pDB , cmd , strlen(cmd) , &stmt , NULL);
                                        if(rc != SQLITE_OK)
					    {
						if(stmt)
						{
						    sqlite3_finalize(stmt);
						}
						sqlite3_close(pDB);
						return -1;
					    }
					    int nColumn = sqlite3_column_count(stmt);
                                            
					
					    { 
						rc = sqlite3_step(stmt);
						if(rc == SQLITE_ROW)
						{
                                                   isSaveDate=1;
                                                   printf("saveDate:1");
		                                   break;
						}
					      }
					      sqlite3_finalize(stmt);*/
                                              if(isSaveData==1)
{
                                              char equipId[5];
                                              sprintf(equipId,"%d",buf[i]);
                                              int res=0;
                                              if(soap_call_ns1__synchronizedAnaData(soap,webserver,NULL,"1",equipId,chData,IPADDR[i],EquipPort[i],&res)==SOAP_OK)
                                              printf("call webservice success\r\n");
                                              else
                                                  printf("call webservice fail\r\n");
                                              sprintf(cmd,"update iot set data='~%s',flag=1 where ID=%d",hexBuf,buf[1]);
	   					printf(cmd);
	   					sqlite3_exec(pDB, cmd, NULL, NULL, &err_msg);
                                                isSaveData=0;
}

                                              
					/*char cmd[100]="update iot set data='~"; 
   				 	char cmd1[100]="',flag=1  where  ID=11"; 
   				 	cmd1[20]=buf[1]/10+0x30;
   				 	cmd1[21]=buf[1]%10+0x30;
   					strcat(cmd,hexBuf);
   					strcat(cmd,cmd1);*/

					showclient();
                                        }
				}
			}

		}

		// 检测是否有新的连接请求
		if (FD_ISSET(sock_fd, &fdsr)) 
			{
				new_fd = accept(sock_fd, (struct sockaddr *)&client_addr, &len);
				if (new_fd <= 0) 
					{
						perror("accept");
						continue;
					}

				// 加入队列
				if (conn_amount < BACKLOG) 
					{	for(i = 0;i < BACKLOG;i++)
							{
								if(fd_A[i] == 0)
									{
										fd_A[i] = new_fd;										
										break;
									}

							}
						conn_amount++;
						printf("New connection from client[%d] %s:%d\n", conn_amount,inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
						if (new_fd > maxsock)
							maxsock = new_fd;
						showclient();
					}
				else 
					{
						conn_amount=0;
						printf("max connection arrive, exit\n");
						send(new_fd, "bye", 4, 0);
						close(new_fd);
						break;
					}
			}


		}
	// close other connections
	for (i = 0; i < BACKLOG; i++) 
		{
			if (fd_A[i] != 0) 
				{
					close(fd_A[i]);
				}
		}
	}

	close(sock_fd);
	exit(0);
}

