#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <dlfcn.h>
#include <pthread.h>
#include <execinfo.h>

#define _LOCK 1
#define _UNLOCK 0

#ifdef DEBUG
    #define debug(fn) fn
#else
    #define debug(fn)
#endif

int main(int argc, char *argv[]){
    int fd;
    while(1){
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
        int lock_type = -1;
        pthread_t tid;
        pthread_mutex_t *mutex;
        unsigned int retaddr;

        read(fd, &lock_type, sizeof(lock_type));
        read(fd, &tid, sizeof(tid));
        read(fd, mutex, sizeof(mutex));
        read(fd, &retaddr, sizeof(retaddr));
        debug(
            printf("Lock type: %d", lock_type);
            printf("tid: %lu", tid);
            printf("mutex: %x", mutex);
            printf("Return address: %x", retaddr);
        );
        close(fd);
    }

    return 0;
}