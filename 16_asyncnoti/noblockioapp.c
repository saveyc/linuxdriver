#include "stdio.h"
#include "unistd.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "stdlib.h"
#include "string.h"
#include "poll.h"
#include "sys/select.h"
#include "sys/time.h"
#include "linux/ioctl.h"
#include "signal.h"


static int fd =0;

static void sigio_signal_func(int signum)
{
	int err = 0;
	unsigned int keyvalue = 0;
	err = read(fd,&keyvalue,sizeof(keyvalue));
	if(err < 0){
		return;
	}
	else{
		printf("sigio signal keyvalue = %d \r\n", keyvalue);
	}

}
/*
 * @description		: main主程序
 * @param - argc 	: argv数组元素个数
 * @param - argv 	: 具体参数
 * @return 			: 0 成功;其他 失败
 */
int main(int argc, char *argv[])
{
	
	int flags;
	int *filename;

	if(argc !=2){
		printf("Error Usage!\r\n");
		return -1;
	}

	filename = argv[1];

	fd = open(filename,O_RDWR);
	if(fd<0){
		printf("can't open file %s \r\n",filename);
		return -1;	
	}

	signal(SIGIO,sigio_signal_func);
	fcntl(fd,F_SETOWN,getpid());
	flags =  fcntl(fd,F_GETFD);
	fcntl(fd,F_SETFL,flag | FASYNC);

	while(1)
	{
		sleep(2);
	}


	ret = close(fd); /* 关闭文件 */
	if(ret < 0){
		printf("file %s close failed!\r\n", argv[1]);
		return -1;
	}
	
	return 0;
}
