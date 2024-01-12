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

static char *name_FIFO = ".ddtrace";

#ifdef DEBUG
    #define debug(fn) fn
#else
    #define debug(fn)
#endif

int mutex_num = 0;
pthread_t *mutex_owner = NULL;
int **mutex_map = NULL;
pthread_mutex_t **mutex_list = NULL;
unsigned int *mutex_return = NULL;

int *real_DFS(int node, int visited[]){
    visited[node] = 1;
    int *cycle = NULL;
    for(int i = 0; i < mutex_num; i++){
        if(mutex_map[node][i] == 1){
            if(visited[i] == 1){ // cycle detected
                cycle = (int*)malloc(sizeof(int) * mutex_num);
                for(int j = 0; j < mutex_num; j++){
                    cycle[j] = visited[j];
                }
                return cycle;
            }
            else{
                cycle = real_DFS(i, visited);
                if(cycle != NULL){
                    return cycle;
                }
            }
        }
    }
    return cycle;
}

int *DFS(int start_node){
    int visited[mutex_num];
    memset(visited, 0, mutex_num);
    return real_DFS(start_node, visited);
}

int *check_cycle(){
    int *result;
    for(int i = 0; i < mutex_num; i++){
        result = DFS(i);
        if(result != NULL){
            break;
        }
    }
    return result;
}

void viz_map(){
    printf("=====MAP=====\n");
    for(int i = 0; i < mutex_num; i++){
        for(int j = 0; j < mutex_num; j++){
            printf("%2d", mutex_map[i][j]);
        }
        printf("\n");
    }
    printf("=============\n");
}

void upscale_map(){
    int **new_map = (int**)malloc(sizeof(int*) * mutex_num);
      for(int i = 0; i < mutex_num; i++){
        new_map[i] = (int*)malloc(sizeof(int) * mutex_num);
        memset(new_map[i], 0, sizeof(int) * mutex_num);
    }

    if(mutex_map != NULL){
        for(int i = 0; i < mutex_num - 1; i++){
            for(int j = 0; j < mutex_num - 1; j++){
                new_map[i][j] = mutex_map[i][j];
            }
        }

        for (int i = 0; i < mutex_num - 1; ++i) {
            free(mutex_map[i]);
        }
        free(mutex_map);
    }

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
    if(remove(name_FIFO) == 0){
        printf("removed\n");
    }
    else{
        printf("not removed\n");
    }
    if (mkfifo(name_FIFO, 0666)) {
		if (errno != EEXIST) {
			perror("fail to open fifo: ") ;
			exit(EXIT_FAILURE) ;
		}
	}

    while(1){
        if((fd = open(name_FIFO, O_RDONLY)) == -1){
            perror("open");
            exit(EXIT_FAILURE);
        }
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

        //데이터를 받음 -> mutex 체크 -> mutex 소유한 thread 체크 -> map update -> cycle 체크
        if(lock_type == _LOCK){
            //new mutex?
            int is_new_mutex = 1;
            for(int i = 0; i < mutex_num; i++){
                if(mutex_list[i] == mutex){
                    is_new_mutex = 0;
                    break;
                }
            }

            if(is_new_mutex){
                pthread_mutex_t **mutex_new_list = (pthread_mutex_t**)malloc(sizeof(pthread_mutex_t*) * (mutex_num + 1));
                pthread_t *mutex_new_owner = (pthread_t*)malloc(sizeof(pthread_t) * (mutex_num + 1));
                unsigned int *mutex_new_return = (unsigned int*)malloc(sizeof(unsigned int) * (mutex_num + 1));
                memset(mutex_new_list, 0, sizeof(pthread_mutex_t*) * (mutex_num + 1));
                memset(mutex_new_owner, 0, sizeof(pthread_t) * (mutex_num + 1));
                memset(mutex_new_return, 0, sizeof(unsigned int) * (mutex_num + 1));
                for(int j = 0; j< mutex_num; j++){
                    mutex_new_list[j] = mutex_list[j];
                    mutex_new_owner[j] = mutex_owner[j];
                    mutex_new_return[j] = mutex_return[j];
                }
                if(mutex_list != NULL){
                    free(mutex_list);
                }
                if(mutex_owner != NULL){
                    free(mutex_owner);
                }
                if(mutex_return != NULL){
                    free(mutex_return);
                }
                mutex_new_list[mutex_num] = mutex;
                mutex_list = mutex_new_list;
                mutex_owner = mutex_new_owner;
                mutex_return = mutex_new_return;
                mutex_num += 1;
                upscale_map();
            }

            // //mutex owner set
            // int owner_index = -1;
            // for(int i = 0; i < mutex_num; i++){
            //     if(mutex_list[i] == mutex){
            //         owner_index = i;
            //         if(mutex_owner[i] != 0){
            //             break;
            //         }
            //         mutex_owner[i] = tid;
            //         break;
            //     }
            // }
            
            //find index of input mutex
            int index_mutex;
            for(int i = 0; i < mutex_num ; i++){
                if(mutex_list[i] == mutex){
                    index_mutex = i;
                    break;
                }
            }

            mutex_return[index_mutex] = retaddr;

            //owner exist?
            if(mutex_owner[index_mutex] == 0){ //not - exist
                mutex_owner[index_mutex] = tid;
            }

            //edge create
            // -> this thread has mutex?
            for(int i = 0; i < mutex_num; i++){
                if(i == index_mutex) continue;
                if(mutex_owner[i] == tid){
                    mutex_map[i][index_mutex] = 1;
                }
            }
            viz_map();
            //TODO: cycle
            
            int *participate;
            participate = check_cycle();
            if(participate != NULL){ //cycle detected!
                //An alert must print out the identifiers of the threads that are involved in the deadlock,
                //and also memory addresses of the mutexes involved in the deadlock
                
                printf("=D=E=A=D=L=O=C=K=\n"); 
                printf("------Mutex------|------Thread------|-----Line Number-----\n");
                for(int i = 0; i < mutex_num; i++){
                    if(participate[i] == 1){
                        printf("%-17x|%-18lu|", mutex_list[i], mutex_owner[i]);
                        // popen()
                        char command[256];
                        snprintf(command, 256, "addr2line -e ./target %x", mutex_return[i]);
                        FILE *fp;
                        fp = popen(command, "r");
                        if (fp == NULL) {
                            perror("popen");
                            exit(EXIT_FAILURE);
                        }
                        char line_out[256];
                        
                        if (fgets(line_out, sizeof(line_out), fp) != NULL) {
                            char * line = strtok(line_out, ":");
                            line = strtok(NULL, ":");
                            printf("%s", line);
                        } else {
                            perror("fgets");
                        }
                        if (pclose(fp) == -1) {
                            perror("pclose");
                            exit(EXIT_FAILURE);
                        }
                    }
                }
                printf("=D=E=A=D=L=O=C=K=\n"); 
                free(participate);
                break;
            }
        }
        else if(lock_type == _UNLOCK){
            int index;
            for(int i = 0; i < mutex_num; i++){
                if(mutex_list[i] == mutex){
                    index = i;
                    break;
                }
            }
            mutex_owner[index] = 0;
            for(int i = 0; i < mutex_num; i++){
                mutex_map[index][i] = 0;
                mutex_map[i][index] = 0;
            }
            viz_map();
        }
        else{
            fprintf(stderr, "lock_type error\n");
            exit(EXIT_FAILURE);
        }

    }

    return 0;
}
