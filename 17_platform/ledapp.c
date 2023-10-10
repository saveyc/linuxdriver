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

#define  LEDON    1
#define  LEDOFF   0


/*
 * @description		: main主程序
 * @param - argc 	: argv数组元素个数
 * @param - argv 	: 具体参数
 * @return 			: 0 成功;其他 失败
 */

int main(int argc, char *argv[])
{
	
	int flags;
	char *filename;
	int fd, retvalue;
	unsigned char databuf[2];

	if(argc !=3){
		printf("Error Usage!\r\n");
		return -1;
	}

	filename = argv[1];

	fd = open(filename,O_RDWR);
	if(fd<0){
		printf("can't open file %s \r\n",filename);
		return -1;	
	}

	databuf[0] = atoi(argv[2]);

	retvalue = write(fd,databuf,sizeof(databuf));

	if(retvalue < 0){
		printf("LED Control Failed \r\n");
		close(fd);
		return -1;
	}

	retvalue = close(fd);
	if(retvalue <0){
		printf("file %s close fail\r\n",argv[1]);
		return -1;
	}
	return 0;
}
