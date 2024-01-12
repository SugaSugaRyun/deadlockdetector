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

pthread_mutex_t mutex_FIFO = PTHREAD_MUTEX_INITIALIZER;

int trust_write(int fd, void *buf, int size){
    int total_write = 0;
    while(total_write < size){
        total_write += write(fd, buf+total_write, size-total_write);
    }
    return total_write;
}

int pthread_mutex_lock(pthread_mutex_t *mutex){
    int (*pthread_mutex_lockp)(pthread_mutex_t *mutex);
    char * error ;
    int (*pthread_mutex_unlockp)(pthread_mutex_t *mutex);
    pthread_mutex_lockp = dlsym(RTLD_NEXT, "pthread_mutex_lock") ;
	if ((error = dlerror()) != 0x0){
		exit(EXIT_FAILURE) ;
    }
    pthread_mutex_unlockp = dlsym(RTLD_NEXT, "pthread_mutex_unlock") ;
	if ((error = dlerror()) != 0x0){
		exit(EXIT_FAILURE) ;
    }
    int ret = pthread_mutex_lockp(mutex);

    int i ;
    void * arr[10] ;
    char ** stack ; 
    size_t sz = backtrace(arr, 10) ;
    stack = backtrace_symbols(arr, sz) ;

    debug(
    fprintf(stderr, "Stack trace\n") ;
    fprintf(stderr, "============\n") ;
    for (i = 0 ; i < sz ; i++)
        fprintf(stderr, "[%d] %s\n", i, stack[i]) ;
    fprintf(stderr, "============\n\n") ;
    );

    int fd;
    if((fd = open(".ddtrace", O_WRONLY)) == -1){
        if(mkfifo(".ddtrace", 0666) == -1){
            perror("mkfifo");
            exit(EXIT_FAILURE);
        }
        if((fd = open(".ddtrace", O_WRONLY)) == -1){
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
    if (sscanf(stack[1], "%*[^(+0x]%*c%x", &retaddr) != 1) {
        printf("Parsing failed.\n");
    }
    retaddr -= 4;
    debug(
        printf("Lock type: %d\n", lock_type);
        printf("tid: %lu\n", tid);
        printf("mutex: %x\n", mutex);
        printf("Return address: %x\n", retaddr);
        printf("\n\n\n");
    );

    pthread_mutex_lockp(&mutex_FIFO);
    trust_write(fd, &lock_type, sizeof(lock_type));
    trust_write(fd, &tid, sizeof(tid));
    trust_write(fd, &mutex, sizeof(&mutex));
    trust_write(fd, &retaddr, sizeof(retaddr));
    pthread_mutex_unlockp(&mutex_FIFO);

    close(fd);
    return ret;
}

int pthread_mutex_unlock(pthread_mutex_t *mutex){
    int (*pthread_mutex_lockp)(pthread_mutex_t *mutex);
    char * error ;
    int (*pthread_mutex_unlockp)(pthread_mutex_t *mutex);
    pthread_mutex_lockp = dlsym(RTLD_NEXT, "pthread_mutex_lock") ;
	if ((error = dlerror()) != 0x0){
		exit(EXIT_FAILURE) ;
    }
    pthread_mutex_unlockp = dlsym(RTLD_NEXT, "pthread_mutex_unlock") ;
	if ((error = dlerror()) != 0x0){
		exit(EXIT_FAILURE) ;
    }

    int i ;
    void * arr[10] ;
    char ** stack ; 
    size_t sz = backtrace(arr, 10) ;
    stack = backtrace_symbols(arr, sz) ;

    debug(
    fprintf(stderr, "Stack trace\n") ;
    fprintf(stderr, "============\n") ;
    for (i = 0 ; i < sz ; i++)
        fprintf(stderr, "[%d] %s\n", i, stack[i]) ;
    fprintf(stderr, "============\n\n") ;
    );

    int fd;
    if((fd = open(".ddtrace", O_WRONLY)) == -1){
        if(mkfifo(".ddtrace", 0666) == -1){
            perror("mkfifo");
            exit(EXIT_FAILURE);
        }
        if((fd = open(".ddtrace", O_WRONLY)) == -1){
            perror("open");
            exit(EXIT_FAILURE);
        } 
    }
    //lock or unlock , thread id         , send mutex addr        , addr of caller (backtrace [1])
    //      4byte    , sizeof(pthread_t) , sizeof(pthread_mutex_t), sizeof(addr)
    int lock_type = _UNLOCK;
    pthread_t tid = pthread_self();
    char program_name[256];
    unsigned int retaddr, address;
    if (sscanf(stack[1], "%*[^(+0x]%*c%x", &retaddr) != 1) {
        printf("Parsing failed.\n");
    }
    retaddr -= 4;
    debug(
        printf("Lock type: %d\n", lock_type);
        printf("tid: %lu\n", tid);
        printf("mutex: %x\n", mutex);
        printf("Return address: %x\n", retaddr);
        printf("\n\n\n");
    );

    pthread_mutex_lockp(&mutex_FIFO);
    trust_write(fd, &lock_type, sizeof(lock_type));
    trust_write(fd, &tid, sizeof(tid));
    trust_write(fd, &mutex, sizeof(&mutex));
    trust_write(fd, &retaddr, sizeof(retaddr));
    pthread_mutex_unlockp(&mutex_FIFO);

    close(fd);

    int ret = pthread_mutex_unlockp(mutex);

    return ret;
}