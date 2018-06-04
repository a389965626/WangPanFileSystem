#include<stdio.h>
#include<stdlib.h>
#include<ctype.h>
#include <termio.h>
#include<sys/stat.h>
#include<unistd.h>
#include<sys/types.h>
#include<dirent.h>
#include<strings.h>
#include<string.h>
#include<fcntl.h>
#include<pwd.h>
#include<grp.h>
#include<sys/mman.h>
#include<time.h>
#include<sys/wait.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<sys/sem.h>
#include<signal.h>
#include<sys/msg.h>
#include<pthread.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<sys/epoll.h>
#include<sys/uio.h>
#include<mysql/mysql.h>
#include <errno.h>
#include <shadow.h>
#include<crypt.h>
#include <memory.h>

typedef struct   //id不需要，因为id自动增长
{
	int flag;//0代表注册，1代表登录
	char salt[30];//存放盐值,由客户端自行产生
	char user_name[30];//存放用户名
	char passwd[100];//存放密文,由盐值和密码通过crypt获得
}data;

data s;//将其设为全局变量，则所有的模块均可以使用

typedef struct
{
	pid_t pid;
	int fd;
	short busy;
}pdata;

typedef struct
{
	int len;//文件名长度
	char buf[1000];//文件名   这个是火车头和火车尾，用于通信
}train;


void child_handle(int fds);
void make_child(pdata *p,int cnum);
void recv_fd(int fds,int *fd);
void send_fd(int fds,int fd);
int mysql_search_salt(char *user_name,char *salt);
int mysql_search_passwd(char *user_name,char *passwd);
int mysql_user_insert(char *user_name,char *salt,char *passwd);
int mysql_log_insert(char *client_name,char *client_connect_time,char *client_optime,char *client_request,char *client_opresult);
int mysql_virtual_file_system_insert(int procode,char *filename,char type,char * belong_user_name,char *md5);
char *gettime();
int makedir(int code,char *dirname);
int server_register(int new_fd);
int changedir(char *virtual_route,char *order,int *code,int *procode);
int mysql_search_filecode(int **code,char *order);
int mysql_search_procode(int **procode);
int  mysql_search_code(int *code,int procode,char *user_name);
int remove_file_dir(int code,char *filename);
int mysql_virtual_file_system_delete_file_directory(int procode,char *filename);
int mysql_virtual_file_system_search_type(int procode,char *filename,char *type);
int ls(int code,int new_fd);
int recvfile(int new_fd);
int transfile(int new_fd,char *FILENAM,char *md5,int shift);
int mysql_search_all_md5(char *md5);
int mysql_search_md5(int code,char *filename,char *belong_user_name,char *md5);
int recvfile_mmap(int new_fd,off_t size);
int transfile_mmap(int new_fd,char *FILENAME,char *md5,off_t size,int shift);
