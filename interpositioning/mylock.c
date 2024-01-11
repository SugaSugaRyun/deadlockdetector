#define _GNU_SOURCE
#include <stdio.h>
#include <dlfcn.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <execinfo.h>

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

    return ret;
}