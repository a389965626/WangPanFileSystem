//客户端登录模块
#include"fun.h"

int client_register(int sfd)//成功登陆返回0,失败返回-1
{
	char buf[1000]="0";//存放存服务器端发来的信息或客户端要发送的信息
	data s;
	memset(&s,0,sizeof(data));
	int flag;//接受是否成功成功登陆的标志
	int ret;
	char *passwd;//存放明文密码

	//登录提示语
	printf("0代表注册账号,非0代表登陆账号\n请输入你的选择并输入用户名和密码\n");

	printf("登录or注册:");
	fflush(stdout);//刷新输出缓冲区
	scanf("%d",&s.flag);

	//必须先把标志发送个服务器端，服务器端才能知道到底是登录还是注册
	send(sfd,&s.flag,sizeof(int),0);

	//flag!=0 表示是要登录已有账号
	if(s.flag)
	{
    	printf("用户名:");
    	fflush(stdout);
    	scanf("%s",s.user_name);
		send(sfd,s.user_name,strlen(s.user_name),0);//发送用户名过去，用于服务器查找盐值并传递过来

		//输入密码
    	passwd=getpass("密码:");

		//输入用户名和密码后，服务器端传来盐值，与客户端的明文密码passwd结合(crypt)得到密文，密文放入结构体传递给服务器端，服务器端开始比较
		//接受盐值
	    recv(sfd,s.salt,sizeof(s.salt),0);

		//判断盐值是否正确，正确才能加密并判断，因为有可能客户端根本没有这个盐值(账户)。
		if(strlen(s.salt)==0)//假定客户端没有找到账户时，无盐值,表明此时不存在账号
		{
			printf("无此用户\n");
			return -1;//登录失败
		}

		//接受盐值之后获得密文并放入结构体,发送给服务器  char *crypt(const char *key, const char *salt);key:要加密的明文;salt:密钥。
		strcpy(s.passwd,crypt(passwd,s.salt));
		send(sfd,s.passwd,strlen(s.passwd),0);

		memset(&flag,0,sizeof(int));
		recv(sfd,&flag,sizeof(int),0);//接受是否成功登陆
		if(flag==0)
		{
			printf("密码错误\n");
			return -1;//登录失败
		}
		else //if(flag==1) 
		{
			printf("密码正确，成功登录\n");
			return 0;//登陆成功
		}
	}
				
	else  //flag==0表示是要注册新账号
	{
		char salt[30]="\0";//得到随机字符串(8bit)

		//客户端自己生成盐值,$6$是盐值必备的，否则得到的密文不是86bit
		rand_string(salt);
		strcpy(s.salt,"$6$");
		sprintf(s.salt,"%s%s%s",s.salt,salt,"$");//$6$........$

		//输入用户名
		printf("用户名:");
		fflush(stdout);
		scanf("%s",s.user_name);

		//输入密码
		passwd=getpass("密码:");
		//输入用户名和密码之后，将明文密码与盐值进行合并计算得到密文并放入结构体，再将结构体发送给服务器;
		strcpy(s.passwd,crypt(passwd,s.salt));
		//printf("salt:%s\n",s.salt);
		//printf("密码:%s\n",passwd);
		//printf("密文:%s\n",crypt(passwd,s.salt));
		//printf("用户名:%s\n",s.user_name);

		memset(buf,0,sizeof(buf));
		strcat(buf,s.salt);
		strcat(buf,s.passwd);
		strcat(buf,s.user_name);
		//printf("buf:%s\n",buf);

		//传递盐值，用户名，密文
		ret=send(sfd,buf,strlen(buf),0);
		if(ret<0)
		{
			printf("server exit\n");
			return -1;
		}

		memset(&flag,0,sizeof(int));
		recv(sfd,&flag,sizeof(int),0);//接受是否成功注册并成功登陆
		if(flag==0)
		{
			printf("注册失败，已经存在同名用户\n");
			return -1;
		}
		else
		{
			printf("注册成功，登录成功\n");
			return 0;
		}
	}
}
