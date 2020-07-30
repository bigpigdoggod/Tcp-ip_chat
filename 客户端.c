#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>
#include <signal.h>

#define BUFSIZE	1024	//缓冲区大小
int main(int argc,char const * argv[])
{
    int sockfd,c;
    char sendline[BUFSIZE];
    struct hostent * h;
    struct sockaddr_in channel;
    if (argc != 2)
	{
		perror("usage  <IP Adrress>");	//缺少参数
        		exit(1);
	}
	h = gethostbyname(argv[1]);	//解析域名
	if (!h)
	{
        		perror("gethostbyname error");	//解析域名错误
		exit(1);
	}
	sockfd = socket(AF_INET,SOCK_STREAM,0);	//客户端socket
	if(sockfd <0)
	{
        	perror("create socket failed");
        	exit(1);
	}
	memset(&channel, 0, sizeof(channel));	//给通道1清空
    channel.sin_family = AF_INET;			//IPV4
    channel.sin_port = htons(8000);			//端口
    channel.sin_addr = *((struct in_addr *) h->h_addr);		//IP地址
    c= connect(sockfd, (struct sockaddr *)&channel, sizeof(channel));	//connect函数，建立连接1
    if(c<0)
    {
        perror("connect to server error");
        exit(1);
    }
    printf("input the something you want:");
	bzero(sendline,sizeof(sendline));
	scanf("%s",&sendline);//printf("%s",sendline);
	int test;
	test=send(sockfd,sendline,sizeof(sendline),0);
	while(1)
	{
		int check;
		char recvline[BUFSIZE];
		memset(recvline,0,sizeof(recvline));
		check = recv(sockfd,recvline,BUFSIZE,0);

		if (check > 0 )
		{
			printf("get massage form server:%s\n",recvline); //print file
			break;
		}
		else
		{
			printf(" wrong'\n");
			break;
		}
	}
	close(c);
	printf("client quit.\n");
	return 0;
}
