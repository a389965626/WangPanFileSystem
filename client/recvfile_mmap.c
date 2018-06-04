//客户端用mmap接受文件
//开两次火车即可
//第一次可能会接受文件名，以及文件名大小，第二次直接一次性接受全部文件
#include"fun.h"
int recvfile_mmap(int sfd,char *FILENAME,int shift,off_t size)
{
	int ret;
	char buf[1000]="0";
	int len;
	if(shift==0)  //表明本地没有该文件，则得先接受文件名
	{
		recv(sfd,&len,sizeof(int),0);//接受火车头，文件名的大小
		recv(sfd,FILENAME,len,0);//接受文件名
	}
	//否则表明本地有该文件，直接偏移，再接受文件
	int fd=open(FILENAME,O_RDWR|O_CREAT,0666);//创建或打开文件,
	if(fd==-1)
	{
		perror("open");
		return -1;
	}
	//打开文件后，对文件进行扩容，以便进行mmap
	ftruncate(fd,size);//改变文件大小并映射到内存中用于mmap
	//映射到内存
	char *p=mmap(NULL,size,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
	//映射到内存后开始进行偏移，因为可能本地已经有部分该文件了，实现断点下载
	if((char*)-1==p)
	{
		perror("mmap");
		return -1;
	}
	printf("mmap success\n");

	//映射成功,将内存映射地址偏移shift个位置并开始接收数据
	while(1)
	{
		recv(sfd,&len,sizeof(int),0);//先接受大小
		if(len>0)
		{
			int total=0;
			while(total<len)
			{
				ret=recv(sfd,p+shift+total,len-total,0);//客户端从p+shift位置开始接受
				total=total+ret;
			}
		}
		else
		{
			break;
		}
	}
	//发送完成后，再解除映射，让文件内容写回磁盘
	ret=munmap(p,size);//解除映射
	if(ret==-1)
	{
		perror("munmap");
		return -1;
	}
	return 0;//传送成功
}
