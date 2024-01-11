#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

int main(int argc, char *argv[]){
    int fd;
    if((fd = open(".ddtrace", O_WRONLY)) == -1){
        if(mkfifo(".ddtrace", 0666) == -1){
            perror("mkfifo");
            exit(EXIT_FAILURE);
        }
        if((fd = open(".ddtrace", O_WRONLY)== -1)){
            perror("open");
            exit(EXIT_FAILURE);
        } 
    }

    char buf[256];
    puts("Q to EXIT\n");
    while(1){
        int len = read(STDIN_FILENO, buf, 256);
        if(strcmp(buf,"Q") == 0 || strcmp(buf, "q") == 0){
            puts("End\n");
            break;
        }
        int written = 0;
        while(written < len){
            written += write(fd, &buf[written], 1);
        }
    }

    return 0;
}