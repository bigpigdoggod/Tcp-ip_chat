#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <linux/tcp.h>
#include <sys/types.h>
#include <sys/signal.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/errno.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/time.h>

#define gettid() syscall(SYS_gettid)
#define	BUFSIZE		1024// 缓冲区大小
#define	port	8000	//端口号
struct {
	pthread_mutex_t	st_mutex;
	unsigned int	st_concount;
	unsigned int	st_contotal;
	unsigned long	st_contime;
	unsigned long	st_bytecount;
	int a_sockfd[100]; 	//统计信息以及客户端socket连接数组
} stats;

pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_t tid;

int sig_handler(int sig);
void* echo_function();	//聊天函数
int Socket();

int main(int argc, char *argv[])
{
    pthread_attr_t	ta;
    (void) pthread_attr_init(&ta);
	(void) pthread_attr_setdetachstate(&ta, PTHREAD_CREATE_DETACHED);
	(void) pthread_mutex_init(&stats.st_mutex, 0);
    bzero(stats.a_sockfd,sizeof(stats.a_sockfd));
    stats.st_concount=0;
    pthread_t pt[2];
    int i;
    //pthread_t tid;
    tid=pthread_self();
    for(i=0;i<2;i++)
    {
        int ret=pthread_create(&pt[i],&ta,(void *)echo_function,NULL);//使用线程处理连接
        if(ret!=0)
            printf("---create pthread%d fail---",i);
        else printf("---create pthread%d successfully---\n",pt[i]);
	sleep(0.5);
    }

    int s=Socket();
    struct sockaddr_in addr;
    socklen_t len;
    signal(10,sig_handler);
    while(1){
        int c=accept(s,(struct sockaddr*)&addr,&len);
        if(c<0)
            continue;
	else printf("-----Connect:%d-----\n",c);
	i=0;
        (void) pthread_mutex_lock(&stats.st_mutex);
	stats.a_sockfd[stats.st_concount]=c;
	stats.st_concount++;
	stats.st_contotal++;
        (void) pthread_cond_signal(&cond);
        (void) pthread_mutex_unlock(&stats.st_mutex);

    }
    pthread_attr_destory(&ta);

}
int Socket(){
    int s=socket(AF_INET,SOCK_STREAM,0);

    struct sockaddr_in addr;
    bzero(&addr,sizeof(addr));
    addr.sin_family=AF_INET;
    addr.sin_port=htons(port);
    addr.sin_addr.s_addr=htonl(INADDR_ANY);

    bind(s,(struct sockaddr*)&addr,sizeof(addr));
    listen(s,5);
    return s;
}


void* echo_function()
{
    while(1){
    char recvbuf[BUFSIZE];
    memset(recvbuf,0,sizeof(recvbuf));	//清空缓冲区
    int num=0;
    int i=0;
    time_t	start;
    (void) pthread_mutex_lock(&stats.st_mutex);
    while(stats.st_concount==0)
        pthread_cond_wait(&cond, &stats.st_mutex);	//等待唤醒
    start=time(0);
    int sockfd=stats.a_sockfd[--stats.st_concount];	//处理最新的连接
    stats.a_sockfd[stats.st_concount]=0;
    (void) pthread_mutex_unlock(&stats.st_mutex);

    while(num = recv(sockfd,recvbuf,BUFSIZE,0))	//接收字符
    {
	printf("get masseage:%s\n",recvbuf);
	pthread_t id;
	id=pthread_self();
	printf("deal by:%d\n",id);
        send(sockfd,recvbuf,strlen(recvbuf),0);	//发送回声

	(void) pthread_mutex_lock(&stats.st_mutex);
		stats.st_bytecount += strlen(recvbuf);
	(void) pthread_mutex_unlock(&stats.st_mutex);
	int ret;
	fd_set read_set;
	struct timeval t_o;
	FD_ZERO(&read_set);
	FD_SET(sockfd,&read_set);
	t_o.tv_sec = 30;/* 超时秒数*/
	ret = select(sockfd + 1,&read_set,NULL,NULL,&t_o);
	if(ret == 1)
	{
	}
	if((ret == 0)||(ret == -1))
	{
	 /* 这两种情况都可认为是链路关闭*/
	break;
	}
    }
    printf("%d:sockfd has colse\n",sockfd);
    (void) pthread_mutex_lock(&stats.st_mutex);
	stats.st_contime += time(0) - start;
    close(sockfd);
    (void) pthread_mutex_unlock(&stats.st_mutex);
    pthread_kill(tid,10);
printf("--------------------------\n",stats.st_concount);
(void) pthread_mutex_lock(&stats.st_mutex);
    printf("---concount:%d---\n",stats.st_concount);
    printf("---contotal:%d---\n",stats.st_contotal);
    printf("---contime:%lu---\n",stats.st_contime);
    printf("---bytecount:%d---\n",stats.st_bytecount);
(void) pthread_mutex_unlock(&stats.st_mutex);
printf("--------------------------\n",stats.st_concount);

}

}int sig_handler(int sig)
{
    usleep(2);
    (void) pthread_cond_signal(&cond);
}

