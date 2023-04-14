#include<stdio.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<stdlib.h>
#include<string.h>

static char userdata[] = {"usr data"};

int main(int argc, char* argv[]) 
{
    int fd,retvalue;

    char* filename = NULL;
    char readbuf[100],writebuf[100];
    if(argc != 3){
        printf("REEOR Usage! \r\n");
        return -1;
    }

    filename = argv[1];

    fd = open(filename,O_RDWR);

    if(fd<0){
        printf("Can't open file %s\r\n",filename);
        return -1;
    }

    if(atoi(argv[2]) == 1){
        retvalue = read(fd,readbuf,50);
        if(retvalue <0){
            printf("read file failed! %s\r\n",filename);
        }
        else{
            print("read date: %s\r\n",readbuf);
        }
    }

    if(atoi(argv[2]) == 2){
        memcpy(writebuf,userdata,sizeof(userdata));
        retvalue = write(fd,writebuf,50);
        if(retvalue<0){
            printf("write file %s failed \r\n",filename);
        }
    }

    retvalue = close(fd);
    if(retvalue<0){
        printf("close file %s failed \r\n",filename);
        return -1;
    }

    return 0;
}


