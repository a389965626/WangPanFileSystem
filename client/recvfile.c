//客户端接受服务器端发送过来的文件
#include"fun.h"
int recvfile(int sfd,char *FILENAME,int shift)
{
	int ret;
	char buf[1000]="0";
	int len;
	//printf("FILENAME=%s\n",FILENAME);
	if(shift==0)  //表明本地没有该文件，则得先接受文件名
	{
		recv(sfd,&len,sizeof(int),0);//接受火车头，文件名的大小
		recv(sfd,FILENAME,len,0);//接受文件名
	}
	//否则表明本地有该文件，直接偏移，再接受文件
	//printf("FILENAME=%s\n",FILENAME);
	int fd=open(FILENAME,O_RDWR|O_CREAT,0666);//创建或打开文件,
	if(fd==-1)
	{
		perror("open");
		return -1;
	}
	//之后再偏移
	lseek(fd,(off_t)shift,SEEK_SET);
	while(1)  //开始循环接收文件
	{
		recv(sfd,&len,sizeof(int),0);
		if(len>0)
		{
			int total=0;
			memset(buf,0,sizeof(buf));//清空
			char *p=buf;
			while(total<len)
			{
				ret=recv(sfd,p+total,len-total,0);
				total=ret+total;
			}
			write(fd,buf,len);//最后再写到本地文件
		}
		else
		{
			printf("gets file success\n");
			break;
		}
	}
	return 0;//下载文件完成，返回0;
}
