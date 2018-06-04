//传递文件文件描述符的函数
#include"fun.h"
void send_fd(int fds,int fd)//主进程将描述符fd通过管道fds[1]发送个子进程,  sendmsg 发送内核控制信息!!
{
    struct msghdr msg;
	bzero(&msg,sizeof(msg));

	struct iovec iov[2];
	char buf1[10]="hello";
	char buf2[10]="world";
	iov[0].iov_base=buf1;
	iov[0].iov_len=5;
	iov[1].iov_base=buf2;
	iov[1].iov_len=5;
	msg.msg_iov=iov;
	msg.msg_iovlen=2;

	struct cmsghdr *cmsg;
	int len=CMSG_LEN(sizeof(int));
	cmsg=(struct cmsghdr *)calloc(1,len);
	cmsg->cmsg_len=len;
	cmsg->cmsg_level = SOL_SOCKET;
	cmsg->cmsg_type = SCM_RIGHTS;
	*(int*)CMSG_DATA(cmsg)=fd;
	msg.msg_control=cmsg;
	msg.msg_controllen=len;

	int ret;
	ret=sendmsg(fds,&msg,0);
	if(-1==ret)
	{
		perror("sendmsg");
		return;
	}
}
