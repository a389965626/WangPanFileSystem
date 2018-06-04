//服务器端登录模块
#include"fun.h"
//根据用户名查salt盐值的函数
int mysql_search_salt(char *user_name,char *salt)//成功返回1，失败返回0
{
	int t,r;
	int ret=-1;
	MYSQL *conn;
	MYSQL_RES *res;
	MYSQL_ROW row;
	char *server="localhost";
	char *user="root";
	char *password="chenjunyu1994";
	char *database="user";

	char query[300]="select salt from user where user_name='";
	sprintf(query,"%s%s%s",query,user_name,"'");//连接成为最后的查询语句
	//puts(query);
	conn=mysql_init(NULL);
	if(!mysql_real_connect(conn,server,user,password,database,0,NULL,0))
	{
		//printf("Error connecting to database:%s\n",mysql_error(conn));//连接数据库失败
		return 0;
	}
	t=mysql_query(conn,query);//连接成功
	if(t) //查找语句错误
	{
		//printf("Error user making query:%s\n",mysql_error(conn));
		mysql_close(conn);
		return 0;//查找失败，返回0
	}
	else//查找成功
	{
		//printf("Query user made...\n");
		res=mysql_use_result(conn);
		if(res)
		{
			while((row=mysql_fetch_row(res))!=NULL)
			{
				ret=0;
				for(t=0;t<mysql_num_fields(res);t++)
				{
					strcpy(salt,row[t]);//将查询结果连接到salt盐值数组
				}
			}
			if(ret==-1)//没有查询到信息
			{
				mysql_free_result(res);
				mysql_close(conn);
				return 0;
			}
			else
			{
	    		mysql_free_result(res);
		    	mysql_close(conn);
		    	return 1;
			}
		}
	}
}

//根据用户名查找服务器数据库的密文,第一次查找盐值salt成功的话，本次查找密文passwd一定成功,除非连接数据库失败
int mysql_search_passwd(char *user_name,char *passwd)//成功返回1，失败返回0
{
	int t,r;
	int ret=-1;
	MYSQL *conn;
	MYSQL_RES *res;
	MYSQL_ROW row;
	char *server="localhost";
	char *user="root";
	char *password="chenjunyu1994";
	char *database="user";
	char query[300]="select passwd from user where user_name='";
	sprintf(query,"%s%s%s",query,user_name,"'");

	conn=mysql_init(NULL);
	if(!mysql_real_connect(conn,server,user,password,database,0,NULL,0))
	{
		//printf("Error connecting to database:%s\n",mysql_error(conn));//连接数据库失败
		return 0;
	}
	t=mysql_query(conn,query);//连接数据库成功
	if(t)//查询语句错误
	{
		//printf("Error user making query:%s\n",mysql_error(conn));
		mysql_close(conn);
		return 0;//查询失败，返回0
	}
	else
	{
    	res=mysql_use_result(conn);
    	if(res)
    	{
			ret=0;
			while((row=mysql_fetch_row(res))!=NULL)
			{
				for(t=0;t<mysql_num_fields(res);t++)
				{
					strcat(passwd,row[t]);
				}
			}
    	}
		if(ret==-1)
		{
			mysql_free_result(res);
			mysql_close(conn);
			return 0;
		}
		else
		{
        	mysql_free_result(res);
        	mysql_close(conn);
	        return 1;//成功查询，返回1
		}
	}
}

//插入user_name、salt、passwd（user_name是唯一的，不能重名）,插入时有可能失败，user_name重名则会失败
int mysql_user_insert(char *user_name,char *salt,char *passwd) //成功返回1，失败返回0
{
	MYSQL *conn;
	MYSQL_RES *res;
	MYSQL_ROW row;
	char* user="root";
	char* server="localhost";
	char *password="chenjunyu1994";
	char *database="user";
	char query[200]="insert into user(user_name,salt,passwd) values('";
	sprintf(query,"%s%s%s%s%s%s%s%s%s%s%s%s",query,user_name,"'",",","'",salt,"'",",","'",passwd,"'",")");//values('chenjunyu','asdwer','123455advsvsv.....')
	int t,r;
	conn=mysql_init(NULL);
	if(!mysql_real_connect(conn,server,user,password,database,0,NULL,0))//连接失败
	{
		//printf("Error connecting to database:%s\n",mysql_error(conn));
		return 0;//失败返回0
	}
	t=mysql_query(conn,query);//连接成功,插入失败
	if(t)//插入语句错误
	{
		//printf("Error user making query:%s\n",mysql_error(conn));
		mysql_close(conn);
		return 0;//失败返回0
	}
	else
	{
		if(mysql_affected_rows(conn))
		{
    		//printf("insert user success\n");
	    	mysql_close(conn);
	    	return 1;//插入成功返回1.
		}
		else
		{
			//printf("insert user fail\n");
			mysql_close(conn);
			return 0;
		}
	}
}

int server_register(int new_fd)//成功登陆返回0,否则返回-1
{
	int ret;
	char buf[1000]="0";//接受或发送的buf
	char passwd[100]="0";//存放查找数据库得到的密文
	int flag;//判断密码是否正确或插入用户信息是否正确的标志;

	//data s;使用服务器端全局变量s，让每个模块均可以知道s中的各信息。
	memset(&s,0,sizeof(data));

	//接受标志
	recv(new_fd,&s.flag,sizeof(int),0);

 	//!0则表明是直接登录，此时应该接受客户端的信息并检查数据库，将盐值放过去
	if(s.flag) 
	{
		//接受用户名并查找数据库
		recv(new_fd,s.user_name,sizeof(s.user_name),0);

		//访问数据库并返回盐值
		ret=mysql_search_salt(s.user_name,s.salt);

		//将盐值发送过去,当salt为空时，则表明没有查询到，发送过去的salt长度为0
		send(new_fd,s.salt,strlen(s.salt),0);
		if(strlen(s.salt)==0)//子进程服务器也判断
		{
			//表明登录失败
			return -1;
		}

		//接受密文.
		recv(new_fd,s.passwd,sizeof(s.passwd),0);

		memset(passwd,0,sizeof(passwd));
		//根据用户名查找服务器数据库的密文
		mysql_search_passwd(s.user_name,passwd);
		if(strcmp(s.passwd,passwd))  //如果不相等，则证明密码错误
		{
			memset(&flag,0,sizeof(int));//发送flag=0，表明密码错误
			send(new_fd,&flag,sizeof(int),0);
			return -1;//登录失败
		}
		else//相等
		{
			memset(&flag,0,sizeof(int));
			flag=1;
			send(new_fd,&flag,sizeof(int),0);
			return 0;//登陆成功
		}
	}
	else  //=0则表明是要注册账号,此时由客户端自行申请盐值salt，并将盐值和密文、账号发送过来
	{
		recv(new_fd,s.salt,12,0);
		recv(new_fd,s.passwd,98,0);
		recv(new_fd,s.user_name,sizeof(s.user_name),0);

		//printf("salt:%s\n",s.salt);
		//printf("用户名:%s\n",s.user_name);
		//printf("密码:%s\n",s.passwd);

		//插入用户信息到数据库，并返回插入结果，因为有可能数据库已经存在相同的用户名，则此时有可能插入失败
		memset(&flag,0,sizeof(int));
		flag=mysql_user_insert(s.user_name,s.salt,s.passwd);//flag获取返回值

		//判断插入用户信息是否成功,成功则返回1，失败返回0)
		if(flag==0)
		{
			send(new_fd,&flag,sizeof(int),0);
			return -1;//登录失败
		}
		else
		{
			send(new_fd,&flag,sizeof(int),0);
			return 0;//插入成功，登陆成功
		}
	}
}
