//生成随机字符串的函数
#include"fun.h"
void rand_string(char salt[])//8bit的随机字符串(盐值)
{
	//设置随机种子,这个种子最好先设置在循环外面，因为在循环里面会导致可能每次得到的种子是一样的，导致每次得到的随机字符串是一样的
	srand((unsigned int)time(0));

	//定义随机生成字符串表
	char *str="0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz/.<>?";
	int lstr=strlen(str);//计算字符串长度

	int i;
	char ss[2]="0";//用于接受每一个字符并带上结束符\0
	for(i=1;i<=8;i++)//按指定大小返回相应的字符串
	{
		sprintf(ss,"%c",str[(rand()%lstr)]);//rand()%lstr 可随机返回0-71之间的整数, str[0-71]可随机得到其中的字符
		strcat(salt,ss);//将随机生成的字符串连接到指定数组后面
	}
}
