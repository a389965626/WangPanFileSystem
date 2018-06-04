//remove删除当前目录下的目录以及文件

//当仅仅是删除文件时，直接删除虚拟目录系统下的文件,然后再检查删除后数据库中是否还有对应的md5码，有的话则不实际删除文件，没有的话则实际删除文件

//删除时，当前目录是不会发生改变的，因为我们就是在当前目录下进行删除的
#include"fun.h"
int remove_file_dir(int code,char *filename)   //删除当前目录下的文件及目录
{
	int procode=code;//当前目录下的文件或目录，则文件或目录的procode一定=code;
	char type;//接受类型,作为一个传出参数
	int ret;
	char md5[64]="\0";

	//在删除之前先查找待删除的文件， 查找type，如果成功查找则可以删除了，查找到文件类型时得保留它的md5，以便查询
	int ret1=mysql_search_md5(code,filename,s.user_name,md5);//如果成功则表明要删除的是文件且这个文件是存在的，因此得到其对应的md5
	//失败表明，要么删除的是目录，要么删除的文件不存在，这两种情况都得不到md5码

	ret=mysql_virtual_file_system_delete_file_directory(procode,filename);//根据上级目录，以及文件名或目录名来查找要删除的类型
	if(ret==0)  
	{
		//printf("nu such file or directory\n");
		return -1;//删除失败返回-1
	}
	else
	{
		//删除成功，则开始检查整个数据库是否还有对应的文件，有的话则表明不用删除实际大目录下的文件，否则我们得实际删除大目录下的文件
		if(ret1==1)  //表明删除的是文件且找到了对应md5)
		{
     		ret=mysql_search_all_md5(md5);
	    	if(ret==1)  //如果仍然有对应的文件，则不用处理，不用删除实际文件
    		{
	    		//printf("delete success\n");
	    		return 0;
    		}
        	else    //否则表明已经没有用户拥有该文件了，可以直接删除了
    		{
	     		char real_route[128]="\0";
		    	sprintf(real_route,"%s%s%s",real_route,"/home/a389965626/Baiduwangpan_root_directory/",md5);
	    		unlink(real_route);
				//printf("delete success\n");
				return 0;
			}
		}
		else   //否则表明删除的是目录且成功删除
		{
			//printf("delete success\n");
			return 0;
		}
	}
}

int mysql_virtual_file_system_delete_file_directory(int procode,char *filename)
{
	int t,r;
	MYSQL *conn;
	MYSQL_RES *res;
	MYSQL_ROW row;
	char *server="localhost";
	char *user="root";
	char *password="chenjunyu1994";
	char *database="user";
	char query[300]="delete from virtual_file_system where procode=";
	sprintf(query,"%s%d%s%s%s%s%s%s%s",query,procode," and ","filename='",filename,"' and ","belong_user_name='",s.user_name,"'");
	//puts(query);
	conn=mysql_init(NULL);
	if(!mysql_real_connect(conn,server,user,password,database,0,NULL,0))//连接数据库失败
	{
		//printf("Error connecting to database:%s\n",mysql_error(conn));
		return 0;
	}
	else
	{
		//printf("connected...:%s\n",mysql_error(conn));

	}
	t=mysql_query(conn,query);
	if(t)  //删除语句错误
	{
		//printf("Error making query:%s\n",mysql_error(conn));
		mysql_close(conn);
		return 0;
	}
	else  //删除语句正确,并且成功删除
	{
		if(mysql_affected_rows(conn))
		{
    		//printf("delete success\n");
    		mysql_close(conn);
    		return 1;
		}
		else
		{
			//printf("delete fail\n");
			mysql_close(conn);
			return 0;
		}
	}
}





int mysql_virtual_file_system_search_type(int procode,char *filename,char *type)
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
	char query[300]="select type from virtual_file_system where procode=";
	sprintf(query,"%s%d%s%s%s%s%s%s%s%s%s%s%s%s%s%s",query,procode," ","and"," ","filename=","'",filename,"'"," ","and"," ","belong_user_name=","'",s.user_name,"'");
	//puts(query);
	conn=mysql_init(NULL);
	if(!mysql_real_connect(conn,server,user,password,database,0,NULL,0))//连接数据库失败
	{
		//printf("Error connecting to database:%s\n",mysql_error(conn));
		return 0;//失败返回0
	}
	t=mysql_query(conn,query);//连接成功
	if(t)//查询语句错误
	{
		//printf("Error virtual_file_system making query:%s\n",mysql_error(conn));
		mysql_close(conn);
		return 0;//查找失败，返回0
	}
	else//查询语句成功
	{
		//printf("Query user made...\n");
		res=mysql_use_result(conn);
		if(res)
		{
			while((row=mysql_fetch_row(res))!=NULL)
			{
				ret=0;
				//printf("you be nere\n");
				for(t=0;t<mysql_num_fields(res);t++)
				{
					//printf("(row[t]=%s\n",row[t]);
					*type=row[t][0];
				}
			}
			if(ret==-1)  //表明没有进入while循环
			{
				//printf("query fail\n");
				mysql_free_result(res);
				mysql_close(conn);
				return 0;//查找失败
			}
			//printf("type=%cn",*type);
			mysql_free_result(res);
			mysql_close(conn);
			return 1;//成功返回1
		}
	}
}
