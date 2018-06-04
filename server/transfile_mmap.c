//服务器端用mmap的方式进行文件发送
//开两趟火车，第一趟发送文件名(lseek=0)，第二趟真正一次性地将mmap映射到内存的文件发送到客户端
#include"fun.h"
int transfile_mmap(int new_fd,char *FILENAME,char *md5,off_t size,int shift)
{
	train t;//定义火车
	if(shift==0)//如果偏移的的字节=0，表明客户端没有该文件，则此时按照以前的 方法，先发文件名,这是开的第一趟火车
	{
		t.len=strlen(FILENAME);//先发火车头
		strcpy(t.buf,FILENAME);
		send(new_fd,&t,4+t.len,0);
	}
	int fd;
	int total;
	int ret;
	char *p;
	char file_route[128]="\0";
	sprintf(file_route,"%s%s","/home/a389965626/Baiduwangpan_root_directory/",md5);
	fd=open(file_route,O_RDONLY);//打开文件,得到文件描述符
	if(fd==-1)
	{
		perror("open");
		return -1;
	}
	//服务器端成功打开文件，将文件映射到内存再发送(以只读的方式映射)
	p=mmap(NULL,size,PROT_READ,MAP_SHARED,fd,0);
	if((char*)-1==p)
	{
		perror("mmap");
		return -1;
	}
	printf("mmap success\n");

	//成功将文件映射到内存，然后在内存起始地址偏移后开始发送文件
	t.len=(int)size-shift;
	send(new_fd,&t.len,sizeof(int),0);//先发大小
	total=0;
	while(total<t.len)
	{
		ret=send(new_fd,p+shift+total,t.len-total,0);//从p+shift开始发，实现断点续传
		if(ret<=0)
		{
			return -1;
		}
		total=total+ret;
	}
	//发火车尾
	t.len=0;
	send(new_fd,&t.len,sizeof(int),0);
	return 0;
}
