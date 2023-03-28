#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <semaphore.h> 
#include <string.h>

#include "Shared_Memory.h"
#include "log.h"

#define DEBUG
#define NUM_THREADS 3

pthread_t threads[NUM_THREADS];
Infos *info;
Registos *tudo;
sem_t *mutex;
FILE *log_file;


void worker(){
    
    char log_message[100];
    FILE *f = fopen("log.txt", "a");

    #ifdef DEBUG
    printf("Worker [%d] created!!!\n", getpid());
    #endif

    sprintf(log_message, "%s %d %s", "Worker", getpid(), "created!!!\n");
    write_to_log_procs(f, log_message);

    Registos *R = malloc(sizeof(Infos));

    char nome[33];
    sprintf(nome, "%d", getpid());
    strcpy(R->nome, nome);
    R->last_val = 1;
    R->min_val = 1;
    R->max_val = 1;
    R->total = 1;
    R->media = 1.0;

    sprintf(log_message, "%s %d %s", "Worker", getpid(), "writing!!!\n");

    sem_wait(mutex);
    write_to_shared_memory(tudo, info, R);
    write_to_log_procs(f, log_message);
    sem_post(mutex);

    #ifdef DEBUG
    printf("Worker [%d] leaving!!!\n", getpid());
    #endif

    sprintf(log_message, "%s %d %s", "Worker", getpid(), "leaving!!!\n");
    write_to_log_procs(f, log_message);

    fclose(f);
    exit(0);
}

void alerts_watcher(){
    #ifdef DEBUG
    printf("Alert Watcher [%d] created!!!\n", getpid());
    #endif

    write_to_log(log_file, ALERTS_WATCHER_START);

    write_to_log(log_file, ALERTS_WATCHER_END);

    #ifdef DEBUG
    printf("Alert Watcher [%d] leaving!!!\n", getpid());
    #endif
}


void *console_reader(void* p){
    int id = *((int*)p);

    #ifdef DEBUG
    printf("Thread console_reader [%d] starting!!!\n", id);
    #endif

    write_to_log(log_file, CONSOLE_READER_START);

    write_to_log(log_file, CONSOLE_READER_END);
    
    pthread_exit(NULL);
}


void *sensor_reader(void* p){
    int id = *((int*)p);

    #ifdef DEBUG
    printf("Thread sensor_reader [%d] starting!!!\n", id);
    #endif

    write_to_log(log_file, SENSOR_READER_START);

    write_to_log(log_file, SENSOR_READER_END);

    pthread_exit(NULL);
}


void *dispatcher(void* p){
    int id = *((int*)p);

    #ifdef DEBUG
    printf("Thread dispatcher [%d] starting!!!\n", id);
    #endif

    write_to_log(log_file, DISPATCHER_START);

    write_to_log(log_file, DISPATCHER_END);

    pthread_exit(NULL);
}


int main(int argc, char *argv[]){

    if(argc != 2){
        printf("./home_iot <ficheiro_config>\n");
        return 0;
    }

    Configuracoes *configs;
    int threads_id[NUM_THREADS];
    int i;

    configs = leitura_ficheiro(argv[1]);

    #ifdef DEBUG
    printf("Queue_sz: %d\nN_Workers: %d\nMax_Keys: %d\nMax_Sensors: %d\nMax_Alerts: %d\n", configs->QUEUE_SZ, configs->N_WORKERS, configs->MAX_KEYS, configs->MAX_SENSORS, configs->MAX_ALERTS);
    #endif

    write_to_log(log_file, PROG_START);

    //Criaçao shared memory
    tudo = create_shared_memory(configs->MAX_KEYS);
    info = create_shared_memory_infos();

    info->keys_atual = 0;
    info->max_keys =  configs->MAX_KEYS;

    //Criaçao do semaforo mutex de acesso a shared memory
    sem_unlink("MUTEX");
	mutex = sem_open("MUTEX", O_CREAT|O_EXCL, 0777, 1);


    //Criaçao processos worker
    for(i = 0; i<configs->N_WORKERS; i++){
        if(fork() == 0){

			worker();
			exit(0);

        }
    }

    //Criaçao processo alerts_watcher
    if(fork() == 0){

        alerts_watcher();
        exit(0);

    }

    for(i = 0; i<NUM_THREADS; i++){
        threads_id[i] = i+1;
    }


    //Criaçao threads
    pthread_create(&threads[0], NULL, console_reader, &threads_id[0]);
    pthread_create(&threads[1], NULL, sensor_reader, &threads_id[1]);
    pthread_create(&threads[2], NULL, dispatcher, &threads_id[2]);   

    //Espera pelas threads
    for(i = 0; i<NUM_THREADS; i++){
        pthread_join(threads[i], NULL);
        #ifdef DEBUG
        printf("Thread [%d] finished!!!\n", i+1);
        #endif
    }

    #ifdef DEBUG
    printf("All Threads finished!!!\n");
    #endif

    //Espera dos processos
    for(i = 0; i<configs->N_WORKERS+1; i++){
        wait(NULL);
        #ifdef DEBUG
        printf("Process [%d] finished!!!\n", i+1);
        #endif
    }

    #ifdef DEBUG
    printf("All Processes finished!!!\n");
    #endif

    print_shared_memory(tudo, info);

	//libertar semaforo mutex de acesso a shared memory
    sem_close(mutex);
	sem_unlink("MUTEX");
    
    //libertar toda a shm
    get_rid_shm(tudo);
    get_rid_shm_infos(info);

    write_to_log(log_file, PROG_END);
    
    exit(0);
}

