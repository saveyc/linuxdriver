#include "stdio.h"
#include "unistd.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "stdlib.h"
#include "string.h"
#include "linux/ioctl.h"
/***************************************************************
Copyright © ALIENTEK Co., Ltd. 1998-2029. All rights reserved.
文件名		: ledApp.c
作者	  	: 左忠凯
版本	   	: V1.0
描述	   	: chrdevbase驱测试APP。
其他	   	: 无
使用方法	 ：./ledtest /dev/led  0 关闭LED
		     ./ledtest /dev/led  1 打开LED		
论坛 	   	: www.openedv.com
日志	   	: 初版V1.0 2019/1/30 左忠凯创建
***************************************************************/
#define CLOSE_CMD (__IO(0xEF,0x1))
#define OPEN_CMD  (__IO(0xEF,0x2))
#define SETPERIOD_CMD (__IO(0xEF,0x3))


/*
 * @description		: main主程序
 * @param - argc 	: argv数组元素个数
 * @param - argv 	: 具体参数
 * @return 			: 0 成功;其他 失败
 */
int main(int argc, char *argv[])
{
	int fd, ret;
	char *filename;
	unsigned int cmd;
	unsigned int arg;
	unsigned char str[100]; 
	
	if(argc != 3){
		printf("Error Usage!\r\n");
		return -1;
	}

	filename = argv[1];

	/* 打开led驱动 */
	fd = open(filename, O_RDWR);
	if(fd < 0){
		printf("file %s open failed!\r\n", argv[1]);
		return -1;
	}


	while(1){
		printf("INPUT CMD: ");
		ret = scanf("%d",&cmd);
		if(ret != 1){
			gets(str);
		}

		if(cmd == 1){
			cmd = CLOSE_CMD;
		}
		else(cmd == 2){
			cmd = OPEN_CMD;
		}
		else if(cmd ==3){
			cmd == SETPERIOD_CMD;
			printf("INPUT_TIMER_PERIOD:");
			ret = scanf("%d",&arg);
			if(ret != 1){
				gets(str);
			}
		}
		ioctl(fd,cmd,arg);
	}

	retvalue = close(fd); /* 关闭文件 */
	if(retvalue < 0){
		printf("file %s close failed!\r\n", argv[1]);
		return -1;
	}
	
	return 0;
}
