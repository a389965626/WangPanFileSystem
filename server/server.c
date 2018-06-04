//服务器端的父进程
#include"fun.h"

void signal_handle(int signalnum)
{
	printf("signal %d is coming\n",signalnum);
	return;
}

int main(int argc,char **argv)
{
	//第一：创建多个子进程
	if(argc!=4)
	{
		printf("error args:./server ip port cnum\n");
		return -1;
	}
	int cnum=atoi(argv[3]);//子进程数cnum;
	pdata *p=(pdata *)calloc(cnum,sizeof(pdata));//结构体数组保存子进程信息，用于与子进程通信
	make_child(p,cnum);//创建cnum个子进程函数

	char buf[1000]="0";//接受信息的buf
	int ret;

	//注册SIGPIPE信号,防止客户端退出
	signal(SIGPIPE,signal_handle);

	//第二：socket,bind,listen,accept,
	//socket
	int sfd=socket(AF_INET,SOCK_STREAM,0);
	if(sfd==-1)
	{
		 perror("socket");
		 return -1;
	}
	//设置socket的属性，让端口port在程序结束后又可以立即使用
	int reuse=1;
	ret=setsockopt(sfd,SOL_SOCKET,SO_REUSEADDR,&reuse,sizeof(int));
	if(-1==ret)
	{
		perror("setsockopt");
		return -1;
	}
	struct sockaddr_in ser;
	memset(&ser,0,sizeof(struct sockaddr_in));//先清空结构体再写，切记
	ser.sin_family=AF_INET;
	ser.sin_port=htons(atoi(argv[2]));
	ser.sin_addr.s_addr=inet_addr(argv[1]);

	struct sockaddr_in client;//接受客户端信息的结构体,accept时获得
	memset(&client,0,sizeof(struct sockaddr_in));

	//bind
	ret=bind(sfd,(struct sockaddr*)&ser,sizeof(struct sockaddr));
	if(ret==-1)
	{
		perror("bind");
		return -1;
	}

	//epoll,设置监听集合
	int epfd=epoll_create(1);//创建一个epoll的句柄,将监听的描述符集合加入epfd
	struct epoll_event event,*evs;//evs集合是epoll返回的已经监控到的描述符集合，从0开始填，第一个出发的填在evs[0]
	evs=(struct epoll_event*)calloc(cnum+1,sizeof(struct epoll_event));//cnum不知道到底多少(命令行参数),故只能申请动态数组
	event.events=EPOLLIN;
	event.data.fd=sfd;
	ret=epoll_ctl(epfd,EPOLL_CTL_ADD,sfd,&event);//sfd加入监听集合
	if(ret==-1)
	{
		perror("epoll_ctl");
		return -1;
	}
	int i;
	for(i=0;i<cnum;i++)
	{
		 event.events=EPOLLIN;
		 event.data.fd=p[i].fd;
		 ret=epoll_ctl(epfd,EPOLL_CTL_ADD,p[i].fd,&event);
		 if(ret==-1)
		 {
			 perror("epoll_ctl");
			 return -1;
		 }
	}

	//一切准备就绪,父进程开始监听在 ip  port，接受客户端的请求
	ret=listen(sfd,cnum);
	if(ret==-1)
	{
		perror("listen");
		close(epfd);
		close(sfd);
		return -1;
	}
	
	//服务器开始循环服务
	int ready_fd;
	int new_fd;
	int len;
	int j;
	while(1)
	{
		//epoll_wait
		ready_fd=epoll_wait(epfd,evs,cnum+1,-1);//-1表示永久等待，最多监听cnum+1个描述符，将就绪描述符(ready_fd个)放入evs数组中
		if(ready_fd>0)
		{
			for(i=0;i<ready_fd;i++)//循环检测哪个描述符就绪了
			{
				if(sfd==evs[i].data.fd)//有客户端的请求到来
				{
					len=sizeof(struct sockaddr);
					new_fd=accept(sfd,(struct sockaddr*)&client,&len);//client,len均是传入传出参数
					if(new_fd==-1)//accept出错，即客户端没能够成功connect，则
					{
						perror("accept");
						return -1;  //出错退出程序
					}
					//得到new_fd之后是登录模块(与数据库相连),，查找空闲子进程，让子进程来进行登录模块，否则父进程一次只能接受一个客户端的登录请求
					//new_fd发送给空闲的子进程
					for(j=0;j<cnum;j++)
					{
						if(p[j].busy==0) //空闲
						{
							send_fd(p[j].fd,new_fd);//发送描述符给子进程
							p[j].busy=1;
							close(new_fd);//父进程断开与客户端的连接，让子进程与其进行交流
							printf("child process %d is handling the business of client whose ip=%s,port=%d\n",p[j].pid,inet_ntoa(client.sin_addr),ntohs(client.sin_port));
							break;//找到空闲子进程之后，则不再查找
						}
					}
					if(j==cnum)  //如果全部子进程均在忙碌，则无法为客户处理事务，让客户等待
					{
						char *ss="host computer is busy,wait for a few minutes and please try connect again later";
						ret=send(new_fd,ss,strlen(ss),0);//给客户端发送忙碌消息
						if(ret<0)//读端退出，写端收到SIGPIPE信号,处理该信号，send 会返回负值
						{
							printf("client computer exited\n");
						}
						close(new_fd);
					}
				}
				for(j=0;j<cnum;j++)
				{
					if(p[j].fd==evs[i].data.fd)  //收到子进程管道的通信,表明子进程处理结束。
					{
						int flag;
						ret=read(p[j].fd,&flag,sizeof(int));//读到flag中(flag是什么不重要)(重要的是接收到子进程信息来的行为)
						if(ret==0)//如果写端退出
						{
							printf("child process %d exited abnormaly\n",p[j].pid);
							continue;
						}
						p[j].busy=0;//标为不忙碌
						printf("child process %d has finished the work and is not busy\n",p[j].pid);
					}
				}
			}
		}
	}
}
