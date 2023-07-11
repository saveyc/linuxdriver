#include "stdio.h"
#include "unistd.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "stdlib.h"
#include "string.h"


#define KEY0VALUE  0XF0
#define KEY0INVAL  0X00

/*
 * @description		: main主程序
 * @param - argc 	: argv数组元素个数
 * @param - argv 	: 具体参数
 * @return 			: 0 成功;其他 失败
 */
int main(int argc, char *argv[])
{
	int fd, retvalue;
	char *filename;
	unsigned char keyvalue;
	
	if(argc != 2){
		printf("Error Usage!\r\n");
		return -1;
	}

	filename = argv[1];

	/* 打开led驱动 */
	fd = open(filename, O_RDWR);

	if(fd<0){
		printf("file %s open failed \r\n",argv[1]);
		return -1;
	}

	while(1){
		read(fd,&keyvalue,sizeof(keyvalue));
		if(keyvalue == KEY0VALUE){
			printf("KEY0 PRESS , value = %#X \r\n",keyvalue);
		}
	}

	ret = close(fd);
	if(ret <0){
		printf("file %s close failed \r\n",argv[1]);
		return -1;
	}
	return 0;
}
