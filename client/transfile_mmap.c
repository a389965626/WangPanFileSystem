//客户端用mmap的方式发送文件
//开两趟火车，第一趟发送文件名(md5)，第二趟真正一次性地将mmap映射到内存的文件发送到客户端
#include"fun.h"
int transfile_mmap(int sfd,char *FILENAME,char *md5,off_t size)
{
	train t;
	t.len=strlen(md5);//先发火车头
	strcpy(t.buf,md5);
	send(sfd,&t,4+t.len,0);

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
	//客户端的文件映射到内存再发送(以只读的方式映射)
	p=mmap(NULL,size,PROT_READ,MAP_SHARED,fd,0);
	if((char*)-1==p)
	{
		perror("mmap");
		return -1;
	}
	printf("mmap success\n");

	//成功映射到内存，开始文件传输,开一趟大火车即可
	t.len=(int)size;
	send(sfd,&t.len,sizeof(int),0);//先发大小
	total=0;
	while(total<t.len)
	{
		ret=send(sfd,p+total,t.len-total,0);
		total=total+ret;
	}
	//发火车尾
	t.len=0;
	send(sfd,&t.len,sizeof(int),0);

	printf("puts file success\n");
	return 0;
}
