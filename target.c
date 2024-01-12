#include <stdio.h>  
#include <unistd.h>  
#include <pthread.h> 
#include <stdbool.h>
#include <stdlib.h>
#include <sys/time.h>

pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER; 
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER; 

bool flag = false;

void * func1(void * arg) {

    // printf("still...\n");
    pthread_mutex_lock(&mutex1);
    pthread_mutex_lock(&mutex2);

    pthread_mutex_unlock(&mutex1);
    pthread_mutex_unlock(&mutex2);

}

int main() {

    pthread_t tid1, tid2;

    char input[100];

    if(pthread_create(&tid1, NULL, func1, NULL) != 0) { 
		fprintf(stderr, "pthread create error\n");
		exit(EXIT_FAILURE);
	}

	if(pthread_create(&tid2, NULL, func1, NULL) != 0) {
		fprintf(stderr, "pthread create error\n");
		exit(EXIT_FAILURE);
	}

    flag = true;

    if(pthread_join(tid1, NULL) != 0) { 
		fprintf(stderr, "pthread join error\n");
		exit(EXIT_FAILURE);
	}

	if(pthread_join(tid2, NULL) != 0) { 
		fprintf(stderr, "pthread join error\n");
		exit(EXIT_FAILURE);
	}

    pthread_mutex_destroy(&mutex1); 
    pthread_mutex_destroy(&mutex2); 

    printf("The End");

    return 0;


}

