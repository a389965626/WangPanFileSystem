//服务器端用mmap的方式来接受文件
//接受两趟火车，第一次接受文件名md5,第二次接受文件内容并放到mmap映射区
#include"fun.h"
int recvfile_mmap(int new_fd,off_t size)
{
	int ret;
	char buf[1000]="0";
	int len;
	recv(new_fd,&len,sizeof(int),0);//接受火车头，文件名的大小(即md5码的长度)
	recv(new_fd,buf,len,0);//接受文件名

	char file_route[128]="\0";
	sprintf(file_route,"%s%s","/home/a389965626/Baiduwangpan_root_directory/",buf);//得到真正物理路径

	int fd=open(file_route,O_RDWR|O_CREAT,0666);//创建文件
	ftruncate(fd,size);//改变文件大小并映射到内存中用于mmap

	char *p=mmap(NULL,size,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
	if((char*)-1==p)
	{
		perror("mmap");
		return -1;
	}
	printf("mmap success\n");

	//成功映射到内存后，则开始进行文件传输
	while(1)
	{
    	recv(new_fd,&len,sizeof(int),0);//先接受大小
		if(len>0)
		{
			int total=0;
			while(total<len)
			{
				ret=recv(new_fd,p+total,len-total,0);
				if(ret==0)
				{
					return -1;
				}
				total=total+ret;
			}
		}
		else
		{
			break;
		}
	}
	//发送完成后，再解除映射，让文件内容写回磁盘.
	ret=munmap(p,size);//解除映射
	if(ret==-1)
	{
		perror("munmap");
		return -1;
	}
	return 0;//传送成功
}
