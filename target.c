#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

pthread_mutex_t lock[2];

int opt;

void * thread_function1(){
	printf("thread1\n");
	pthread_mutex_lock(&lock[0]);
	if(opt == 'd'){
		sleep(1.5);
	}
	pthread_mutex_lock(&lock[1]);
	pthread_mutex_unlock(&lock[1]);
	pthread_mutex_unlock(&lock[0]);
	return NULL;
}

void * thread_function2(){
	printf("thread2\n");
	sleep(1);
	pthread_mutex_lock(&lock[1]);
	pthread_mutex_lock(&lock[0]);
	pthread_mutex_unlock(&lock[0]);
	pthread_mutex_unlock(&lock[1]);
	return NULL;
}

int main(int argc, char * argv[]){
	opt = getopt(argc, argv, "d");
	
	pthread_t threads[2];
	
	for(int i=0; i<2; i++){
		pthread_mutex_init(&lock[i], NULL);
	}
	pthread_create(&threads[0], NULL, thread_function1, NULL);
	pthread_create(&threads[1], NULL, thread_function2, NULL);
	pthread_join(threads[0], NULL);
	pthread_join(threads[1], NULL);
}

