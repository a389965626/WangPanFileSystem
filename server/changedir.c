//改变目录的函数
#include"fun.h"
//virtual_route:虚拟目录,用户刚进入时 virtual_route 这个数组是空的，这个数组与real_virtual_route合并得到真实的虚拟目录
//形参:当前虚拟目录，命令，当前目录,上层目录(virtual_route,code,procode均是传入传出参数)

int changedir(char *virtual_route,char *order,int *code,int *procode) 
{
	//第一个:cd到上一层目录
	//命令是向上cd并且上层目录是0，则无法往上cd
	if(strcmp(order,"..")==0&&*procode==0)
	{
		//printf("you have reached the top directory\n");
		strcpy(virtual_route,"\0");
		return -1;//返回-1表示失败
	}
	else if(strcmp(order,"..")==0)//仅仅是往上走一层且上层目录不是0时    /user_name/filename/file1/file2
	{
		int l=strlen(virtual_route);
		int i;
		for(i=l-1;i>=0;i--)
		{
			if(virtual_route[i]=='/') //找到第一个/的地方
			{
				virtual_route[i]='\0';//将其置为结束标志，表明上移了一个目录
				break;
			}
		}
		//之后改变code和procode的值(因为改变了目录，code和procode的值一定会发生改变。
		//改变当前目录，向上cd，上层目录变成当前目录
		*code=*procode;

		//然后改变上层目录，上层目录的上层目录变为此时的上层目录
		//select procode from .... where code=procode;
		mysql_search_procode(&procode);//根据procode查找procode,procode为传入传出参数
		return 0;//返回0表示成功
	}

	//第二个: 如果cd命令不是cd ..，则一定是cd filename  (cd 当前文件夹下的文件夹)往下cd

	//或者根据cd filename 的 filename 名去查找虚拟目录，//select code from ... where procode=code(上一级目录是code) and filename= order and type='d';  在当前目录code下查找
	else
	{
    	int temp=*code;//先保存code
    	int ret=mysql_search_filecode(&code,order);//在当前目录code下查找是否有对应的目录，有才能cd
    	if(ret==0)
    	{
    		//printf("error order\n");
    		return -1;//返回-1代表cd失败
    	}
    	//否则表明查找成功，这个命令是正确的，可以执行
    	*procode=temp;//改变procode，且code作为传入传出参数已经发生了改变

    	//再改变虚拟目录数组virtual_route的值
    	sprintf(virtual_route,"%s%s%s",virtual_route,"/",order);//拼接得到虚拟目录的当前目录
    	return 0;//返回0代表cd成功
	}
}
int mysql_search_filecode(int **code,char *order)//判断命令是否正确，在正确的同时直接改变当前目录和上级目录
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

	char query[300]="select code from virtual_file_system where procode=";
	sprintf(query,"%s%d%s",query,**code," ");
	sprintf(query,"%s%s%s%s%s%s%s%s%s",query,"and"," ","filename","=","'",order,"'"," ");
	sprintf(query,"%s%s%s%s%s%s%s%s",query,"and"," ","type","=","'","d","'");
	//puts(query);

	conn=mysql_init(NULL);
	if(!mysql_real_connect(conn,server,user,password,database,0,NULL,0))
	{
		//printf("Error connecting to database:%s\n",mysql_error(conn));
		return 0;//失败返回0
	}
    t=mysql_query(conn,query);//连接成功
    if(t) //查找语句错误
	{
   		//printf("Error virtual_file_system making query:%s\n",mysql_error(conn));
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
					**code=atoi(row[t]);
				}
			}
			if(ret==-1)  //表明没有进入while循环
			{
				mysql_free_result(res);
				mysql_close(conn);
				return 0;//查找失败
			}
			else
			{
	    		mysql_free_result(res);
	    		mysql_close(conn);
	    		return 1;//成功返回1
			}
		}
	}
}

int mysql_search_procode(int **procode)
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
	char query[300]="select procode from virtual_file_system where code=";
	sprintf(query,"%s%d",query,**procode);//连接成为最后的查询语句
	//puts(query);
	conn=mysql_init(NULL);
	if(!mysql_real_connect(conn,server,user,password,database,0,NULL,0))
	{
		//printf("Error connecting to database:%s\n",mysql_error(conn));//连接数据库失败
		return 0;//失败返回0
	}
	t=mysql_query(conn,query);//连接成功
	if(t) //查找失败
	{
		//printf("Error virtual_file_system making query:%s\n",mysql_error(conn));
		mysql_close(conn);
		return 0;//查找失败，返回0
	}
	else
	{
		 //printf("Query virtual_file_system made...\n");
		 res=mysql_use_result(conn);
		 if(res)
		 {
			 while((row=mysql_fetch_row(res))!=NULL)
			 {
				 ret=0;
				 for(t=0;t<mysql_num_fields(res);t++)
				 {
					 **procode=atoi(row[t]);
				 }
			 }
			 if(ret==-1)
			 {
				  mysql_free_result(res);
				  mysql_close(conn);
				  return 0;//查找失败
			 }
			 else
			 {
		         mysql_free_result(res);
		      	 mysql_close(conn);
		         return 1;//成功返回1
			 }
		 }
 	}
}
