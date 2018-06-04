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

#define READ_DATA_SIZE  1024
#define MD5_SIZE        16
#define MD5_STR_LEN     (MD5_SIZE * 2)
#ifndef MD5_H
#define MD5_H

typedef struct
{
	unsigned int count[2];
	unsigned int state[4];
	unsigned char buffer[64];   
} MD5_CTX;


#define F(x,y,z) ((x & y) | (~x & z))
#define G(x,y,z) ((x & z) | (y & ~z))
#define H(x,y,z) (x^y^z)
#define I(x,y,z) (y ^ (x | ~z))
#define ROTATE_LEFT(x,n) ((x << n) | (x >> (32-n)))

#define FF(a,b,c,d,x,s,ac) \
{ \
		a += F(b,c,d) + x + ac; \
		a = ROTATE_LEFT(a,s); \
		a += b; \
}
#define GG(a,b,c,d,x,s,ac) \
{ \
		a += G(b,c,d) + x + ac; \
		a = ROTATE_LEFT(a,s); \
		a += b; \
}
#define HH(a,b,c,d,x,s,ac) \
{ \
		a += H(b,c,d) + x + ac; \
		a = ROTATE_LEFT(a,s); \
		a += b; \
}
#define II(a,b,c,d,x,s,ac) \
{ \
		a += I(b,c,d) + x + ac; \
		a = ROTATE_LEFT(a,s); \
		a += b; \
}                                            
void MD5Init(MD5_CTX *context);
void MD5Update(MD5_CTX *context, unsigned char *input, unsigned int inputlen);
void MD5Final(MD5_CTX *context, unsigned char digest[16]);
void MD5Transform(unsigned int state[4], unsigned char block[64]);
void MD5Encode(unsigned char *output, unsigned int *input, unsigned int len);
void MD5Decode(unsigned int *output, unsigned char *input, unsigned int len);

#endif


int Compute_file_md5(const char *file_path, char *md5_str);





typedef struct   //id不需要，因为id自动增长
{
	int flag;//0代表注册，1代表登录
	char salt[30];//存放盐值,由客户端自行产生
	char user_name[30];//存放用户名
	char passwd[100];//存放密文,由盐值和密码通过crypt获得
}data;

typedef struct
{
	int len;//文件名长度
	char buf[1000];
}train;

int client_register(int sfd);
void rand_string(char salt[]);
void rand_string(char salt[]);
int transfile(int sfd,char *FILENAME,char *md5);
int recvfile(int sfd,char *FILENAME,int shift);
int transfile_mmap(int sfd,char *FILENAME,char *md5,off_t size);
int recvfile_mmap(int sfd,char *FILENAME,int shift,off_t size);
