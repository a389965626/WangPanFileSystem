//实现ls命令，
#include"fun.h"
int ls(int code,int new_fd) //根据当前目录code显示当前目录下的所有文件以及目录,  即查找procode=code的项
{
	char buf[128]="\0";
	int procode=code;
	int ret=-1;
	int t,r;
	MYSQL *conn;
	MYSQL_RES *res;
	MYSQL_ROW row;
	char *server="localhost";
	char *user="root";
	char *password="chenjunyu1994";
	char *database="user";
	char query[300]="select type,filename from virtual_file_system where procode=";
	sprintf(query,"%s%d",query,procode);
	//puts(query);
	conn=mysql_init(NULL);
	if(!mysql_real_connect(conn,server,user,password,database,0,NULL,0))
	{
		//printf("Error connecting to database:%s\n",mysql_error(conn));
		return -1;
	}
	else
	{
		//printf("Connected...\n");
	}
	t=mysql_query(conn,query);
	if(t)
	{
		//printf("Error making query:%s\n",mysql_error(conn));
		mysql_close(conn);
		return -1;
	}
	else
	{
		//printf("Query made...\n");
		res=mysql_use_result(conn);
		if(res)
		{
			while((row=mysql_fetch_row(res))!=NULL)
			{
				ret=0;
				for(t=0;t<mysql_num_fields(res);t++)//每次得到一行的一列
				{
					strcat(buf,row[t]);
					strcat(buf," ");
				}
				strcat(buf,"\n");
				send(new_fd,buf,strlen(buf),0);
				memset(buf,0,sizeof(buf));
			}
			if(ret==-1)  //表明没有进入while循环
			{
				//printf("query fail\n");
				mysql_free_result(res);
				mysql_close(conn);
				return -1;//查找失败
			}
			mysql_free_result(res);
			mysql_close(conn);
			return 0;
		}
	}
}
