//循环创建cnum个子进程的函数
#include"fun.h"

void child_handle(int fds)
{
	int new_fd;
	int ret;
	int flag;//通知服务器主进程 自己是否处理结束

	char virtual_route[128]="\0";//存储虚拟目录，这个目录中实际没有/user_name,方便与real_route拼接获得文件或目录的真实地址
	char real_virtual_route[150]="\0";//实际存储的当前所在的真实虚拟目录，用/user_name/....表示，通过/user_name和virtual_route拼接得到

	//procode,code作为传入，传出参数。有利于对虚拟目录进行判断和访问。
	//code存放当前目录的值(cd便代表着当前目录，procode便代表着上级目录)，procode存放上级目录的值
	//cd，ls，pwd命令均需要这2个数的值
	int procode=0,code=0;


	while(1)//子进程循环服务
	{
        recv_fd(fds,&new_fd);//得到new_fd，开始进行登录模块

		char start_log[25]="0";//记录登录时间
		strncpy(start_log,gettime(),strlen(gettime())-1);

    	ret=server_register(new_fd);//服务器端的登录模块，在子进程中进行

		char end_log[25]="0";
		strncpy(end_log,gettime(),strlen(gettime())-1);

    	if(ret==-1)//表明客户端没能连接或注册成功
    	{
			mysql_log_insert(s.user_name,start_log,end_log,"log","fail");//记录登录日志
    		close(new_fd);
			flag=1;
			write(fds,&flag,sizeof(int));//通过管道告知父进程处理结束
    		continue;
    	}
		//否则表明客户端连接或注册成功，子进程开始循环与客户端进行通信
		mysql_log_insert(s.user_name,start_log,end_log,"log","success");

		//第一步:如果是注册，则s.flag==0，表明我们应该在数据库中插入虚拟目录信息
		if(s.flag==0) 
		{
			//将目录信息插入虚拟目录
			mysql_virtual_file_system_insert(0,s.user_name,'d',s.user_name,NULL);
		}

		//否则flag!=0,表明只是登录，在虚拟目录系统中已经有了相应的目录信息，接下来只需进行相应操作即可。

		//第二步:用户进入后，初始化数据结构，我们得获得prodecode,code，才能继续进行操作
		strcpy(virtual_route,"\0");
		sprintf(real_virtual_route,"%s%s%s",real_virtual_route,"/",s.user_name);// /user_name
		procode=0;//代表顶级目录
		mysql_search_code(&code,procode,s.user_name);//根据用户的用户名找到用户的当前目录code(为了防止同名目录下有同用户名相同的目录现象，必须保证所查找的procode为0)

		//第三步:用户初始化后，我们开始写命令函数:cd  pwd  ls  remove mkdir  gets  puts
		char buf[1000]="0";  //接受客户端命令的buf。
		fd_set rdset;
		while(1)  //开始循环接受命令并执行
		{
			 FD_ZERO(&rdset);
			 FD_SET(new_fd,&rdset);
			 ret=select(new_fd+1,&rdset,NULL,NULL,NULL);
			 if(ret>0)
			 {
				 if(FD_ISSET(new_fd,&rdset))//客户端发送消息(命令)过来
				 {
					 memset(buf,0,sizeof(buf));//清空缓冲区
					 ret=recv(new_fd,buf,sizeof(buf)-1,0);//类似read
					 if(ret==0)//返回0,写端(客户端)退出
					 {
						 close(new_fd);//关闭连接
						 flag=1;
						 write(fds,&flag,sizeof(int));//通过管道告知父进程处理结束
						 break;//跳出内层循环，在外层开始重新等待父进程发送来new_fd
					 }
					 //以下开始拆分命令,
					 //cd ..
					 if(strcmp(buf,"cd ..")==0)
					 {
						 char start_cd1[25]="0";//记录cd..的时间
						 strncpy(start_cd1,gettime(),strlen(gettime())-1);

						 ret=changedir(virtual_route,"..",&code,&procode);
						 if(ret==0) //changedir成功
						 {
							 mysql_log_insert(s.user_name,start_log,start_cd1,"cd ..","success");
					    	 sprintf(real_virtual_route,"%s%s%s","/",s.user_name,virtual_route);
					    	 send(new_fd,real_virtual_route,strlen(real_virtual_route),0);//将改变后的路径发送过去
						 }
						 else
						 {
							 mysql_log_insert(s.user_name,start_log,start_cd1,"cd ..","fail");
							 char *p="you have reached the top directory";
							 send(new_fd,p,strlen(p),0);
						 }
					 }
					 //cd filename
					 else if(buf[0]=='c'&&buf[1]=='d'&&buf[2]==' '&&buf[3]!='.')  //cd filename
					 {
						 char start_cd2[25]="0";//记录cd filename的时间
						 int l=strlen(buf);
						 char order[128]="0";
						 int i;
						 int j=0;
						 for(i=3;i<=l;i++)
						 {
							 order[j++]=buf[i];//将命令与filename拆分
						 }
						 strncpy(start_cd2,gettime(),strlen(gettime())-1);
						 ret=changedir(virtual_route,order,&code,&procode);
						 if(ret==0)
						 {
							 mysql_log_insert(s.user_name,start_log,start_cd2,"cd direcory","success");
							 sprintf(real_virtual_route,"%s%s%s","/",s.user_name,virtual_route);
							 send(new_fd,real_virtual_route,strlen(real_virtual_route),0);//将改变后的路径发送过去
						 }
						 else
						 {
							 mysql_log_insert(s.user_name,start_log,start_cd2,"cd direcory","fail");
							 char *p="error filename";
							 send(new_fd,p,strlen(p),0);
						 }
					 }
					 //pwd
					 else if(strcmp(buf,"pwd")==0)  //pwd
					 {
						 char start_pwd[25]="0";//记录pwd的时间
						 strncpy(start_pwd,gettime(),strlen(gettime())-1);
						 mysql_log_insert(s.user_name,start_log,start_pwd,"pwd","success");

						 sprintf(real_virtual_route,"%s%s%s","/",s.user_name,virtual_route);
						 send(new_fd,real_virtual_route,strlen(real_virtual_route),0);//发送当前实际虚拟目录
					 }
					 //mkdir dirname
					 else if(buf[0]=='m'&&buf[1]=='k'&&buf[2]=='d'&&buf[3]=='i'&&buf[4]=='r'&& buf[5]==' ')
					 {
						 char start_mkdir[25]="0";//记录pwd的时间
						 int l=strlen(buf);
						 char order[128]="0";
						 int i;
						 int j=0;
						 for(i=6;i<=l;i++)
						 {
							 order[j++]=buf[i];//将命令与dirname拆分
						 }
						 strncpy(start_mkdir,gettime(),strlen(gettime())-1);
						 ret=makedir(code,order);
						 if(ret==0)
						 {
							 mysql_log_insert(s.user_name,start_log,start_mkdir,"makedir","success");
							 char *p="mkdir success";
							 send(new_fd,p,strlen(p),0);
						 }
						 else
						 {
							 mysql_log_insert(s.user_name,start_log,start_mkdir,"makedir","fail");
							 char *p="mkdir fail";
							 send(new_fd,p,strlen(p),0);
						 }
					 }
					 //ls
					 else if(strcmp(buf,"ls")==0)
					 {
						 char start_ls[25]="0";//记录ls的时间
						 strncpy(start_ls,gettime(),strlen(gettime())-1);
						 ret=ls(code,new_fd);//ls成功，在ls的函数中会直接发送
						 if(ret==-1)
						 {
							 mysql_log_insert(s.user_name,start_log,start_ls,"ls","fail");
							 char *p=" ";//没有则什么都不显示
							 send(new_fd,p,strlen(p),0);
						 }
						 else
						 {
							 mysql_log_insert(s.user_name,start_log,start_ls,"ls","success");
						 }
					 }
					 //remove filename，考虑在删除文件的时候，在删除后再次查询数据库是否有相同的md5码，有则不实际删除文件，否则就实际删除大目录下的文件
					 else  if(buf[0]=='r'&&buf[1]=='e'&&buf[2]=='m'&&buf[3]=='o'&&buf[4]=='v'&& buf[5]=='e'&&buf[6]==' ')
					 {
						 char start_remove[25]="0";//记录remove的时间
						 int l=strlen(buf);
						 char order[128]="0";
						 int i;
						 int j=0;
						 for(i=7;i<=l;i++)
						 {
							 order[j++]=buf[i];//将命令与fiename拆分
						 }
						 strncpy(start_remove,gettime(),strlen(gettime())-1);
						 ret=remove_file_dir(code,order);//删除文件时，考虑在数据库中没有md5的情况下实际删除磁盘文件
						 if(ret==-1)
						 {
							 mysql_log_insert(s.user_name,start_log,start_remove,"remove","fail");
							 char *p="remove fail";
							 send(new_fd,p,strlen(p),0);
						 }
						 else
						 {
							 mysql_log_insert(s.user_name,start_log,start_remove,"remove","success");
							 char *p="remove scuuess";
							 send(new_fd,p,strlen(p),0);
						 }
						
					 }
					 //gets filename,接受下载请求
					 else if(buf[0]=='g'&&buf[1]=='e'&&buf[2]=='t'&&buf[3]=='s'&&buf[4]==' ')
					 {
						 char start_gets[25]="0";//记录gets的时间
						 strncpy(start_gets,gettime(),strlen(gettime())-1);
						 int l=strlen(buf);
						 char order[128]="0";
						 int i;
						 int j=0;
						 int send_flag;//是否能够发送文件的标志
						 int shift;//接受文件偏移量
						 for(i=5;i<=l;i++)
						 {
							 order[j++]=buf[i];
						 }
						 char md5[64]="\0";//用来存储md5码

						 //服务器得到文件名之后，去数据库查询该对应的md5码，查询到则证明有该文件，则直接调用发送函数
						 ret=mysql_search_md5(code,order,s.user_name,md5);//查询数据库是否有满足条件的md5码,有的话则直接告诉客户端可以接受文件了
						// printf("md5=%s\n",md5);
						 if(ret==0)//=0则表明没有查到对应的md5码，表明客户端是无法下载的
						 {
							 send_flag=0;//0表明服务器当前目录下没有属于该用户的对应的文件，客户端无法下载
							 send(new_fd,&send_flag,sizeof(int),0);
							 mysql_log_insert(s.user_name,start_log,start_gets,"gets","fail");
							// printf("no such file\n");
						 }
						 //否则表明服务器有该文件，客户端可以进行下载
						 else
						 {
							 send_flag=1;
							 //printf("statr transmission\n");
							 send(new_fd,&send_flag,sizeof(int),0);
							 recv(new_fd,&shift,sizeof(int),0);//接受偏移量
							 //printf("shift=%d\n",shift);

							 //获得本地文件的大小，并判断size-shift(即剩下的)是否<=100MB,<=100MB则可以直接按照原先的方法进行发送，否则表明应该使用mmap来进行发送
							 struct stat l;
							 char route[128]="\0";
							 sprintf(route,"%s%s%s",route,"/home/a389965626//Baiduwangpan_root_directory/",md5);//文件路径
							 stat(route,&l);//得到文件大小
							 //将文件大小发送到客户端，以便客户端用ftruncate扩充文件大小并进行mmap
							 send(new_fd,&l.st_size,sizeof(off_t),0);
							 if(l.st_size-shift<=100*1024*1024)//<=100MB,按照之前的方法发送即可
							 {
						    	 transfile(new_fd,order,md5,shift);//开始循环发送文件
			    				 mysql_log_insert(s.user_name,start_log,start_gets,"gets","success");
							 }
							 //否则，表明服务器要发送的文件大小是要>100MB的，应该用mmap进行发送
							 else
							 {
								 transfile_mmap(new_fd,order,md5,l.st_size,shift);
								 mysql_log_insert(s.user_name,start_log,start_gets,"gets","success");
							 }
						 }
					 }

					 //puts filename
					 else if(buf[0]=='p'&&buf[1]=='u'&&buf[2]=='t'&&buf[3]=='s'&&buf[4]==' ')
					 {
						 char start_puts[25]="0";//记录puts的时间
						 strncpy(start_puts,gettime(),strlen(gettime())-1);
						 int l=strlen(buf);
						 char order[128]="0";
						 int i;
						 int j=0;
						 int recv_flag;
						 for(i=5;i<=l;i++)
						 {
							 order[j++]=buf[i];
						 }
						 char md5[64]="0";//用来存储md5码
						 send(new_fd,&recv_flag,sizeof(int),0);//让客户端发送md5,让md5的发送和命令的发送不在一起
						 recv(new_fd,md5,sizeof(md5)-1,0);//接受md5，并去服务器查找
						 //printf("md5=%s\n",md5);
						 ret=mysql_search_all_md5(md5);//只要服务器数据库的所有记录中一个有相同的md5码，则证明大目录下一定有相应的文件，则此时只需要插入信息，而不需要实际上传文件，实现秒传
						 if(ret==1)  //查找成功，再查找当前目录下是否有，因为当前目录下有的话则不用插入数据库信息了
						 {
							 ret=mysql_search_md5(code,order,s.user_name,md5);
							 if(ret==0)  //当前目录没有
							 {
						    	 mysql_virtual_file_system_insert(code,order,'-',s.user_name,md5);
							 }
							 recv_flag=0;
							 send(new_fd,&recv_flag,sizeof(int),0);
							// printf("秒传成功\n");
							 mysql_log_insert(s.user_name,start_log,start_puts,"puts","success");
						 }
						 else   //证明没有找到对应的md5码，表明服务器没有对应的文件，应该既插入数据库，又要上传文件
						 {
							 mysql_virtual_file_system_insert(code,order,'-',s.user_name,md5);
							 recv_flag=1;
							 send(new_fd,&recv_flag,sizeof(int),0);
							 //当客户端要发送文件是，这边先接受文件大小，根据文件大小来决定到底是否使用mmap

							 off_t size;
							 recv(new_fd,&size,sizeof(off_t),0);
							 if(size<=100*1024*1024)    //当文件大小<=100MB时，可以直接按照以前的发送方式，不用mmap
							 {
					    		 recvfile(new_fd);
						    	 mysql_log_insert(s.user_name,start_log,start_puts,"puts","success");
							 }
							 else       //否则表明文件>100MB,应该使用mmap
							 {
								 recvfile_mmap(new_fd,size);//服务器端用mmap的方式接受文件,将new_fd，size传过去
								 mysql_log_insert(s.user_name,start_log,start_puts,"puts","success");
							 }
						 }
					 }
				 }
			 }
		}
	}
}

int mysql_search_all_md5(char *md5)//查找是否有对应的md5
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
	char query[300]="select md5 from virtual_file_system where md5='";
	sprintf(query,"%s%s%s",query,md5,"'");
	//puts(query);
	conn=mysql_init(NULL);
	if(!mysql_real_connect(conn,server,user,password,database,0,NULL,0))
	{
		//printf("Error connecting to database:%s\n",mysql_error(conn));
		return 0;
	}
	t=mysql_query(conn,query);
	if(t)
	{
		//printf("Error virtual_file_system making query:%s\n",mysql_error(conn));
		mysql_close(conn);return 0;
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
				return 1;
			}
		}
	}
}

int mysql_search_md5(int code,char *filename,char *belong_user_name,char *md5)//查找md5的函数
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
	char query[300]="select md5 from virtual_file_system where procode=";
	sprintf(query,"%s%d%s%s%s%s%s",query,code," and filename='",filename,"' and type='-' and belong_user_name='",belong_user_name,"'");
	//puts(query);
	conn=mysql_init(NULL);
	if(!mysql_real_connect(conn,server,user,password,database,0,NULL,0))
	{
		//printf("Error connecting to database:%s\n",mysql_error(conn));
		return 0;
	}
	t=mysql_query(conn,query);
	if(t)
	{
		//printf("Error virtual_file_system making query:%s\n",mysql_error(conn));
		mysql_close(conn);
		return 0;
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
					strcat(md5,row[t]);
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
				return 1;
			}
		}
	}
}



void make_child(pdata *p,int cnum)
{
	int i;
	int fds[2];
	int ret;
	pid_t pid;
	for(i=0;i<cnum;i++)//循环创建cnum个子进程，p[i]用于保存子进程信息，用于通信
	{
		ret=socketpair(AF_LOCAL,SOCK_STREAM,0,fds);//两条父子通信的全双工管道,父子传递文件描述符(内核控制信息)不能用pipe无名管道
		if(ret==-1)
		{
			perror("socketpair");
			return;
		}
		pid=fork();//创建子进程
		if(pid==0)//子进程
		{
			close(fds[1]);//子进程用fds[0];
			child_handle(fds[0]);//子进程处理函数,子进程需要父进程发来描述符,因此只需传递管道过去即可
		}
		//父进程
		close(fds[0]);//父进程用fds[1];
		p[i].pid=pid;
		p[i].fd=fds[1];
	}
}
//创建子进程成功


int  mysql_search_code(int *code,int procode,char *user_name)//成功返回1，失败返回0
{
	MYSQL *conn;
	MYSQL_RES *res;
	MYSQL_ROW row;
	char* server="localhost";
	char* user="root";
	char* password="chenjunyu1994";
	char* database="user";
	char query[300]="select code from virtual_file_system where filename='";//根据用户名找到用户的当前目录
	sprintf(query,"%s%s%s",query,user_name,"' and procode=0 and type ='d'");
	//puts(query);
	int t,r;
	int ret=-1;
	conn=mysql_init(NULL);
	if(!mysql_real_connect(conn,server,user,password,database,0,NULL,0))//连接数据库失败
	{
		//printf("Error connecting to database:%s\n",mysql_error(conn));
		return -1;
	}
	else
	{
		//printf("Connected...\n");
	}
	t=mysql_query(conn,query);//连接成功
	if(t)//查找语句错误
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
					*code=atoi(row[t]);//查找数据库返回的都是字符串，因此得atoi转化
				}
				//printf("you be here\n");
			}
			if(ret==-1)
			{
				//printf("query fail\n");
				mysql_free_result(res);
				mysql_close(conn);
				return 0;
			}
			mysql_free_result(res);
			mysql_close(conn);
			return 1;
		}
	}
}
