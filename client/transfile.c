//客户端发送文件,
#include"fun.h"
int transfile(int sfd,char *FILENAME,char *md5)  //FILENAME是文件的真正名字，因为在客户端这边，文件是以真正的文件名存储的
{
	train t;
	t.len=strlen(md5);//先发火车头
	strcpy(t.buf,md5);
	send(sfd,&t,4+t.len,0);//发火车头

	int ret;
	int fd;
	int total;
	char *p;

	fd=open(FILENAME,O_RDONLY);//打开客户端本地的真实文件并上传
	if(fd==-1)
	{
		perror("open");
		return -1;
	}
	while((t.len=read(fd,t.buf,sizeof(t.buf)))>0)
	{
		send(sfd,&t.len,sizeof(int),0);//先发大小
		total=0;
		p=t.buf;
		while(total<t.len)
		{
			ret=send(sfd,p+total,t.len-total,0);
			total=total+ret;
		}
	}
	send(sfd,&t,4+t.len,0);
	printf("puts file success\n");
	return 0;
}
