#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>

int is_end = 0;

pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

void noise(){
    usleep(rand()/1000);
}

void child_thread(void *arg){
    pthread_t mytid = pthread_self();
    printf("thread created : %lu\n", mytid);
    while(!is_end){
        pthread_mutex_lock(&mtx);
        printf("loop!\n");
        noise();
        pthread_mutex_unlock(&mtx);
    }
}

int main(int argc, char *argv[]){
    pthread_t tid;
    if(pthread_create(&tid, NULL, (void*)child_thread, NULL) != 0){
        perror("pthread_create");
        exit(EXIT_FAILURE);
    }

    char buf[256];
    read(STDIN_FILENO, buf, 256);
    is_end = 1;

    pthread_join(tid, NULL);
}