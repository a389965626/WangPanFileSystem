//服务器端接受文件，先接受文件名(md5码)并创建，再接受文件大小
#include"fun.h"
int recvfile(int new_fd)
{
	int ret;
	char buf[1000]="0";
	int len;
	recv(new_fd,&len,sizeof(int),0);//接受火车头，文件名的大小(即md5码的长度)
	recv(new_fd,buf,len,0);//接受文件名
	char file_route[128]="\0";
	sprintf(file_route,"%s%s","/home/a389965626/Baiduwangpan_root_directory/",buf);//得到真正物理路径
	int fd=open(file_route,O_RDWR|O_CREAT,0666);//创建文件
	if(fd==-1)
	{
		perror("open");
		return -1;//失败返回-1
	}
	while(1)  //循环接受文件
	{
		recv(new_fd,&len,sizeof(int),0);
		if(len>0)
		{
			int total=0;
			memset(buf,0,sizeof(buf));//清空
			char *p=buf;
			while(total<len)
			{
				ret=recv(new_fd,p+total,len-total,0);//在传送的过程中很有可能客户端断开了
				if(ret==0)
				{
					return -1;
				}
				total=ret+total;
			}
			write(fd,buf,len);//最后再写到磁盘(大目录下)
		}
		else
		{
			//printf("finish transmission\n");
			break;
		}
	}
	return 0;//接受成功
}
