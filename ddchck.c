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
/*
=====map=====
    -------> x
    |
    |
    |
    V
    y
*/
#define _LOCK 1
#define _UNLOCK 0

#ifdef DEBUG
    #define debug(fn) fn
#else
    #define debug(fn)
#endif

int mutex_num = 0;
pthread_t *mutex_owner;
int **mutex_map;

void upscale_map(){
    mutex_num += 1;
    int **new_map = (int**)malloc(sizeof(int*) * mutex_num);
    for(int i = 0; i < mutex_num; i++){
        new_map[i] = (int*)malloc(sizeof(int) * mutex_num);
        memset(new_map[i], 0, sizeof(int) * mutex_num);
    }

    for(int i = 0; i < mutex_num - 1; i++){
        for(int j = 0; j < mutex_num - 1; j++){
            new_map[i][j] = mutex_map[i][j];
        }
    }

    for (int i = 0; i < mutex_num - 1; ++i) {
        free(mutex_map[i]);
    }
    free(mutex_map);
    mutex_map = new_map;
}

int trust_read(int fd, void *buf, int size){
    int total_read= 0;
    while(total_read < size){
        total_read += read(fd, buf+total_read, size-total_read);
    }
    return total_read;
}

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
        printf("pipe connected\n");
        int lock_type = -1;
        pthread_t tid;
        pthread_mutex_t *mutex;
        unsigned int retaddr;

        trust_read(fd, &lock_type, sizeof(lock_type));
        trust_read(fd, &tid, sizeof(tid));
        trust_read(fd, &mutex, sizeof(&mutex));
        trust_read(fd, &retaddr, sizeof(retaddr));
        debug(
            printf("Lock type: %d\n", lock_type);
            printf("tid: %lu\n", tid);
            printf("mutex: %x\n", mutex);
            printf("Return address: %x\n", retaddr);
            printf("\n\n\n");
        );
        close(fd);
    }

    return 0;
}