//将信息插入虚拟目录系统
#include"fun.h"
int mysql_virtual_file_system_insert(int procode,char *filename,char type,char * belong_user_name,char *md5)//成功返回1，失败返回0
{
	MYSQL *conn;//code自动增长，不用插入
	MYSQL_RES *res;
	MYSQL_ROW row;
	char* user="root";
	char* server="localhost";
	char *password="chenjunyu1994";
	char *database="user";
	char query[200]="insert into virtual_file_system(procode,filename,type,belong_user_name,md5) values('";
	sprintf(query,"%s%d%s%s%s%s%s%s%s%c%s%s%s%s%s%s%s%s%s%s",query,procode,"'",",","'",filename,"'",",","'",type,"'",",","'",belong_user_name,"'",",","'",md5,"'",")");
	//puts(query);
	int t,r;
	conn=mysql_init(NULL);
	if(!mysql_real_connect(conn,server,user,password,database,0,NULL,0))//连接失败
	{
		//printf("Error connecting to database:%s\n",mysql_error(conn));
		return 0;
	}
	t=mysql_query(conn,query);//连接成功
	if(t)//插入语句错误
	{
		//printf("Error virtual_file_system  making query:%s\n",mysql_error(conn));
		return 0;
	}
	else
	{
		if(mysql_affected_rows(conn))
		{
			//printf("insert virtual_file_system success\n");
			mysql_close(conn);
			return 1;
		}
		else
		{
			//printf("insert virtual_file_system fail\n");
			mysql_close(conn);
			return 0;
		}
	}
}	
