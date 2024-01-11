#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

int main(int argc, char *argv[]){
    int fd;
    if((fd = open(".ddtrace", O_RDONLY)) == -1){
        if(mkfifo(".ddtrace", 0666) == -1){
            perror("mkfifo");
            exit(EXIT_FAILURE);
        }
        if((fd = open(".ddtrace", O_RDONLY)) == -1){
            perror("open");
            exit(EXIT_FAILURE);
        } 
    }

    char buf[256];
    while(read(fd, buf, 1) != 0){
        printf("%s",buf);
    }

    return 0;
}