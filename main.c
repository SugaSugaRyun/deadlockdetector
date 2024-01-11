#include <stdio.h>
#include <pthread.h>

int main(void){
    printf("%d",sizeof(pthread_t));
    return 0;
}