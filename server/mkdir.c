//新建一个目录的函数
//在虚拟目录系统中新建一个目录，实际上这个目录并没有实际存在磁盘上，只是放在虚拟目录，磁盘上的大目录只存文件，不存目录。

#include"fun.h"
//在当前目录code下新建一个目录，新建目录后并不改变实际的当前目录，
//新建目录的procode=当前目录code，
//新建目录实际就是在数据库中插入一条信息。

int makedir(int code,char *dirname)//0代表创建成功，-1代表创建失败
{
	int ret;
    char pathname[100];
	//procode=code
	ret=mysql_virtual_file_system_insert(code,dirname,'d',s.user_name,NULL);//在virtual_file_system中插入一个新的条目
	if(ret==1)
	{
		//printf("mkdir success\n");
		return 0;
	}
	else
	{
		//printf("mkdir fail\n");
		return -1;
	}
}
