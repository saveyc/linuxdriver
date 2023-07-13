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
	struct pollfd fds;
	fd_set readfds;
	struct timeval timeout;
	unsigned char data;
	
	if(argc != 2){
		printf("Error Usage!\r\n");
		return -1;
	}

	filename = argv[1];

	fd = open(filename, O_RDWR| O_NONBLOCK);
	if(fd < 0){
		printf("file %s open failed!\r\n", argv[1]);
		return -1;
	}

#if 1

	fds.fd = fd;
	fds.event = POLLIN;

	while(1){
		ret = poll(&fds,1,500);
		if(ret){
			ret = read(fd,&data,sizeof(data));
			if(ret < 0){

			}
			else{
				if(data){
					printf(" key value = %d \r\n",data);
				}
			} 
		}
		else if( ret == 0){

		}
		else if( ret < 0){

		}
	}

#endif

	while(1){
		FD_ZERO(&readfds);
		FD_SET(fd,&readfds);
		timeout.tv_sec =0;
		timeout.tv_usec = 500000;
		ret = select(fd+1,&readfs,NULL,NULL,&timeout);
		switch(ret){
			case 0: //超时
			    break;
			case -1: 
			    break;
			default:
				if(FD_ISSET(fd,&readfds)){
					ret  = read(fd,&data,sizeof(data));
					if(ret < 0){

					}
					else{
						if(data){
							printf("key value %d \r\n",data);
						}
					}
				}
				break;
		}

	}

	ret = close(fd); /* 关闭文件 */
	if(ret < 0){
		printf("file %s close failed!\r\n", argv[1]);
		return -1;
	}
	
	return 0;
}
