//循环发送文件的函数
//对于服务器端:真正存着的文件的名字是他的md5码。在数据库中既存有文件名，也存有md码。而在客户端中存着的文件的名字就是他子的文件名
//因此，服务器端和客户端的发送和接受文件的函数是不一样的。
#include"fun.h"
int transfile(int new_fd,char *FILENAME,char *md5,int shift)//将new_fd,FILENAME(文件名),偏移值传过来
{
	train t;//定义火车
	if(shift==0)//如果偏移的的字节=0，表明客户端没有该文件，则此时按照以前的方法，先发文件名
	{
    	t.len=strlen(FILENAME);//先发火车头
    	strcpy(t.buf,FILENAME);
	    send(new_fd,&t,4+t.len,0);
	}
	//否则表明没有shift!=0，开始偏移客户端的文件并开始传送。
	//获得文件指针
	int fd;
	int total;
	int ret;
	char *p;
	char file_route[128]="\0";//存放文件的实际目录,我们说有的文件都是放在一个大目录下面，大目录下面只有文件，没有目录。因此可以确定文件的路径
	sprintf(file_route,"%s%s","/home/a389965626/Baiduwangpan_root_directory/",md5);//得到真正物理路径
	//puts(file_route);//打印验证
	fd=open(file_route,O_RDONLY);//打开文件,得到文件描述符
	if(fd==-1)
	{
		perror("open");
		return -1;//失败返回-1
	}
	lseek(fd,(off_t)shift,SEEK_SET);//从文件头开始偏移到指定位置并开始传送
	while((t.len=read(fd,t.buf,sizeof(t.buf)))>0)//循环发送
	{
		send(new_fd,&t.len,sizeof(int),0);//先发大小，再发内容
		total=0;  //发送到total=len位置
		p=t.buf; //p保留开始位置
		while(total<t.len)
		{
			ret=send(new_fd,p+total,t.len-total,0);
			if(ret<=0)
			{
				return -1;
			}
			total=total+ret;
		}
	}
	send(new_fd,&t,4+t.len,0);//最后发火车尾
	//发送完毕，返回
	//printf("transmission finish\n");
	return 0;
}
