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

void trust_write(int fd, void *buf, int size){
    int total_write = 0;
    while(total_write < size){
        total_write += write(fd, buf+total_write, size-total_write);
    }
    return;
}

int pthread_mutex_lock(pthread_mutex_t *mutex){
    int (*pthread_mutex_lockp)(pthread_mutex_t *mutex);
    char * error ;

    pthread_mutex_lockp = dlsym(RTLD_NEXT, "pthread_mutex_lock") ;
	if ((error = dlerror()) != 0x0){
		exit(EXIT_FAILURE) ;
    }
    printf("lock called\n");
    int ret = pthread_mutex_lockp(mutex);

    int i ;
    void * arr[10] ;
    char ** stack ; 
    size_t sz = backtrace(arr, 10) ;
    stack = backtrace_symbols(arr, sz) ;

    fprintf(stderr, "Stack trace\n") ;
    fprintf(stderr, "============\n") ;
    for (i = 0 ; i < sz ; i++)
        fprintf(stderr, "[%d] %s\n", i, stack[i]) ;
    fprintf(stderr, "============\n\n") ;

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

    //lock or unlock , thread id         , send mutex addr        , addr of caller (backtrace [1])
    //      1byte    , sizeof(pthread_t) , sizeof(pthread_mutex_t), sizeof(addr)
    int lock_type = _LOCK;
    pthread_t tid = pthread_self();
    char program_name[256];
    unsigned int retaddr, address;
    if (sscanf(stack[1], "%255s(+%x) [0x%x]", program_name, &retaddr, &address) == 3) {
        // 추출된 값 출력
        printf("Program Name: %s\n", program_name);
        printf("Return Address: %x\n", retaddr);
        printf("Address: %x\n", address);
    } else {
        // 실패 시 에러 메시지 출력
        printf("Parsing failed.\n");
    }
    retaddr -= 4;

    //lock
    trust_write(fd, &lock_type, sizeof(lock_type));
    trust_write(fd, &tid, sizeof(tid));
    trust_write(fd, mutex, sizeof(mutex));
    trust_write(fd, &retaddr, sizeof(retaddr));
    //unlock

    close(fd);
    return ret;
}