#include"fun.h"

//客户端,客户端与主机之间，不仅满足下载文件，接受文件，还满足:从客户端给主机传送命令等条件

void signal_handle(int signalnum)
{
	printf("signal %d is coming\n",signalnum);
	return;
}

int main(int argc,char **argv)
{
	if(argc!=3)
	{
		printf("error args:./client ip port\n");
		return -1;
	}
lable1:
	signal(SIGPIPE,signal_handle);//注册IGPIPE信号，防止读端退出

	//socket
	int sfd=socket(AF_INET,SOCK_STREAM,0);
	if(sfd==-1)
	{
		perror("socket");
		return -1;
	}
	struct sockaddr_in ser;
	memset(&ser,0,sizeof(struct sockaddr_in));
	ser.sin_family=AF_INET;
	ser.sin_addr.s_addr=inet_addr(argv[1]);
	ser.sin_port=htons(atoi(argv[2]));

	//connect;
	int ret;
	ret=connect(sfd,(struct sockaddr*)&ser,sizeof(struct sockaddr));
	if(ret==-1)
	{
		perror("connect");
		return -1;
	}

	//登录模块
	//connect 之后，应该登录，登陆成功才能与客户端进行通话;
    ret=client_register(sfd);//客户端登录模块
	if(ret==-1)//登录或注册失败,则返回登录界面继续进行
	{
		//system("clear");
		printf("请重新登录或注册");
		goto lable1;
		//close(sfd);
		//return 0;//失败则结束程序
	}

	char buf[1000]="0";

	//cd pwd ls  mkdir remove gets  puts
	fd_set rdset;
	while(1)//登陆或注册成功,客户端循环与服务器通信
	{
		FD_ZERO(&rdset);
		FD_SET(0,&rdset);//监听标准输入
		FD_SET(sfd,&rdset);
		ret=select(sfd+1,&rdset,NULL,NULL,NULL);
		if(ret>0)
		{
			if(FD_ISSET(sfd,&rdset))//服务器端有消息过来
			{
				//printf("message from server\n");
				memset(buf,0,sizeof(buf));//清空缓冲区
				ret=recv(sfd,buf,sizeof(buf)-1,0);//类似read
				if(ret==0)//返回0，证明要么服务端断开，要么服务端主动close了连接，此时客户端应该退出
				{
					printf("byebye\n");
					close(sfd);//关闭连接
					exit(0);
				}
				else
				{
					system("clear");
					printf("%s\n",buf);
				}
			}
			//cd pwd ls mkdir remove gets  puts
			if(FD_ISSET(0,&rdset))
			{
				//printf("message from stdin\n");
				memset(buf,0,sizeof(buf));
				ret=read(0,buf,sizeof(buf)-1);//读过来的包括换行符
				//printf("buf:%sbuflen=%d\n",buf,(int)strlen(buf)-1);
				if(ret<=0)//标准输入来了ctrl+D等退出的信号，会返回0
				{
					printf("byebye\n");
					close(sfd);
					exit(0);
				}
				else
				{
					//gets
					if(buf[0]=='g'&&buf[1]=='e'&&buf[2]=='t'&&buf[3]=='s'&&buf[4]==' ')  //如果是gets命令，则为了进行断点下载，应该先检查本地当前目录下是否有该文件,open打开失败则表明没有，open打开成功则表明有该文件，打开不成功则表明没有该文件
					{
						int l=strlen(buf)-1;
						int recv_flag;//是否能够接受文件的标志
						int i;
						int j=0;
						int shift;//文件偏移量
						char order[128]="0";
						for(i=5;i<l;i++)//得到具体文件名
						{
							order[j++]=buf[i];
						}
						int fd=open(order,O_RDWR); //确定本地是否有该文件，有的话则证明之前从服务器端下载过那个文件，只是没有全部下载完成，则直接进行偏移到文件末尾,进行断点下载,

						if(fd==-1)   //=-1则表明本地没有该文件，发送命令，请求下载文件,如果获得下载许可(服务器有文件存在),则接受下载许可，并开始循环接受文件(由于本地本目录下是不会有重名的，因此只需判断文件名即可)
						{
							shift=0;
						}
						else//本地有该文件
						{
							//获得文件大小
							struct stat filebuf;
							stat(order,&filebuf);
							shift=(int)filebuf.st_size;//得到文件大小即为要偏移的值
							close(fd);//关闭文件
						}
						ret=send(sfd,buf,strlen(buf)-1,0);//发送命令gets filename
						if(ret<=0)
						{
							printf("byebye\n");
							close(sfd);
							exit(0);
						}

						ret=recv(sfd,&recv_flag,sizeof(int),0);//接受标志
						if(ret==0)
						{
							printf("byebye\n");
							close(sfd);
							exit(0);
						}
						if(recv_flag==0)
						{
							printf("gets fail\n");//下载失败
						}
						else
						{
							printf("start loading\n");
							send(sfd,&shift,sizeof(int),0);//发送偏移量
							//printf("shift=%d\n",shift);
							off_t size;
							//接受下载的文件大小
							recv(sfd,&size,sizeof(off_t),0);
							if(size-shift<=100*1024*1024)//<=100MB时按照以前的方法进行传送
							{
					    		recvfile(sfd,order,shift);
								printf("gets success\n");
							}
							//否则表明传送的文件是大于100MB的，客户端也应该用mmap来进行接受
							else
							{
								recvfile_mmap(sfd,order,shift,size);
								printf("gets success\n");
							}
						}
					}
					//puts filename
					else if(buf[0]=='p'&&buf[1]=='u'&&buf[2]=='t'&&buf[3]=='s'&&buf[4]==' ')
					{
						int l=strlen(buf)-1;
						int send_flag;
						int i;
						int j=0;
						int shift;
						char order[128]="0";
						char md5[64]="0";
						for(i=5;i<l;i++)//得到文件名,用于取得md5码并发送
						{
							order[j++]=buf[i];
						}
						//printf("filename=%s\n",order);
						//获得对应本地文件的md5码
						ret=Compute_file_md5(order,md5);
						if(ret==-1)
						{
							printf("获得md5错误，本地无此文件\n");
						}
						else if(ret==0)//md5成功,本地有文件且得到md5，则先发命令，再发md5过去让服务器验证
						{
							//printf("md5:%s\n", md5);
							send(sfd,buf,strlen(buf)-1,0);//发送命令puts filename
							//将md5码发送到服务器端，服务器检查数据库是否有相应的md5信息，有则只插入数据库，不传文件，实现秒传，否则没有的话则纪要插入数据库，又要上传文件
							recv(sfd,&send_flag,sizeof(int),0);
							send(sfd,md5,strlen(md5),0);
							recv(sfd,&send_flag,sizeof(int),0);//接受标志
							//printf("send_flag=%d\n",send_flag);
							if(send_flag==0)  //不用发送
							{
								printf("start upload\n");
								printf("秒传成功\n");
							}
							//否则表明服务器端没有对应的文件，此时应该考试实际发送文件，但发送文件分为两种情况。第一是文件大小<100M，此时可以直接按原先的方法传送和接受文件，否则，得用到mmap来进行传送文件。
							else
							{
								printf("start upload\n");

								struct stat l;//用来得到文件大小
								stat(order,&l);
								send(sfd,&l.st_size,sizeof(off_t),0);//先发送文件大小

								if(l.st_size<=100*1024*1024)    //当文件大小<=100MB时，可以直接按照以前的发送方式，不用mmap
								{
						    		transfile(sfd,order,md5);
							    	printf("transmission succeed\n");
								}
								else        //否则表明文件>100MB,应该使用mmap
								{
									transfile_mmap(sfd,order,md5,l.st_size);
									printf("transmission succeed\n");
								}
							}
						}
					}
					else
					{
				     	ret=send(sfd,buf,strlen(buf)-1,0);
			     		//printf("send success\n");
				     	if(ret<=0)
				    	{
				     		printf("byebye\n");
					    	close(sfd);
						    exit(0);
					    }
			    	}
				}
			}
		}
	}
	return 0;
}
