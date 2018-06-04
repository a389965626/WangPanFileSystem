//接受文件描述符
#include"fun.h"
void recv_fd(int fds,int *fd)  //接收fd时必须用指针，因为接收到的进程并不知道fd的值是多少，fd发过来是3，但接受到的并不是3
{
    struct msghdr msg;
	bzero(&msg,sizeof(msg));

	struct iovec iov[2];
	char buf1[10]="0";
	char buf2[10]="0";
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
	msg.msg_control=cmsg;
	msg.msg_controllen=len;

	int ret;
	ret=recvmsg(fds,&msg,0);
	if(-1==ret)
	{
		perror("recvmsg");
		return;
	}
	*fd=*(int*)CMSG_DATA(cmsg);
}
