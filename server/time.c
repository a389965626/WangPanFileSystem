#include"fun.h"
char *gettime()
{
	char *p;
	time_t t;
	t=time(NULL);
	p=ctime(&t);
	//字符串时间共24个字符
	return p;
}
