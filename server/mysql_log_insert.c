//插入日志文件，包括client_connect_time，client_optime，client_request，client_opresult
#include"fun.h"
int mysql_log_insert(char *client_name,char *client_connect_time,char *client_optime,char *client_request,char *client_opresult)//成功返回1，失败返回0
{
	MYSQL *conn;
	MYSQL_RES *res;
	MYSQL_ROW row;
	char* user="root";
	char* server="localhost";
	char *password="chenjunyu1994";
	char *database="user";
	char query[200]="insert into log(client_name,client_connect_time,client_optime,client_request,client_opresult) values('";
	sprintf(query,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",query,client_name,"'",",","'",client_connect_time,"'",",","'",client_optime,"'",",","'",client_request,"'",",","'",client_opresult,"'",")");
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
		//printf("Error log making query:%s\n",mysql_error(conn));
		mysql_close(conn);
		return 0;
	}
	else
	{
		if(mysql_affected_rows(conn))
		{
			//printf("insert log success\n");
			mysql_close(conn);
			return 1;
		}
		else
		{
    		//printf("insert log fail\n");
		    mysql_close(conn);
	    	return 0;
		}
	}
}
