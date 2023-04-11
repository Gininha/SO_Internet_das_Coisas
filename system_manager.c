/*
Luis Leite 2021199102
*/

#include "Shared_Memory.h"

#define DEBUG
#define NUM_THREADS 3

pthread_t threads[NUM_THREADS];
Infos *info;
Registos *tudo;
sem_t *mutex_shm;
Sem_Log *semaforo_log;
Alertas *alertas;

void worker(){
    
    char log_message[250];

    #ifdef DEBUG
    printf("Worker [%d] created!!!\n", getpid());
    #endif

    sprintf(log_message, "%s %d %s", "Worker", getpid(), "created!!!\n");

    write_log(log_message, semaforo_log);

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

    sem_wait(mutex_shm);
    write_log(log_message, semaforo_log);
    printf("Worker [%d] writing!!!\n", getpid());
    if(write_to_shared_memory(tudo, info, R)){
        sprintf(log_message, "%s %d %s", "Sensor", getpid(), "failed to add -> Max Capacity reached!!!\n");
        write_log(log_message, semaforo_log);
    }

    #ifdef DEBUG
    if(write_to_shared_memory(tudo, info, R)){
        sprintf(log_message, "%s %d %s", "Sensor", getpid(), "failed to add -> Max Capacity reached!!!\n");
        write_log(log_message, semaforo_log);
    }
    #endif

    sleep(2);
    sem_post(mutex_shm);

    #ifdef DEBUG
    printf("Worker [%d] leaving!!!\n", getpid());
    #endif

    sprintf(log_message, "%s %d %s", "Worker", getpid(), "leaving!!!\n");
    
    write_log(log_message, semaforo_log);

    exit(0);
}

void alerts_watcher(){
    #ifdef DEBUG
    printf("Alert Watcher [%d] created!!!\n", getpid());
    #endif

    write_log(ALERTS_WATCHER_START, semaforo_log);

    write_log(ALERTS_WATCHER_END, semaforo_log);

    #ifdef DEBUG
    printf("Alert Watcher [%d] leaving!!!\n", getpid());
    #endif
}


void *console_reader(void* p){
    int id = *((int*)p);

    #ifdef DEBUG
    printf("Thread console_reader [%d] starting!!!\n", id);
    #endif

    write_log(CONSOLE_READER_START, semaforo_log);
    
    write_log(CONSOLE_READER_END, semaforo_log);
    
    pthread_exit(NULL);
}


void *sensor_reader(void* p){
    int id = *((int*)p);

    #ifdef DEBUG
    printf("Thread sensor_reader [%d] starting!!!\n", id);
    #endif

    write_log(SENSOR_READER_START, semaforo_log);

    write_log(SENSOR_READER_END, semaforo_log);

    pthread_exit(NULL);
}


void *dispatcher(void* p){
    int id = *((int*)p);

    #ifdef DEBUG
    printf("Thread dispatcher [%d] starting!!!\n", id);
    #endif

    write_log(DISPATCHER_START, semaforo_log);
    
    write_log(DISPATCHER_END, semaforo_log);

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
    if(configs == NULL){
        return 0;
    }

    #ifdef DEBUG
    printf("Queue_sz: %d\nN_Workers: %d\nMax_Keys: %d\nMax_Sensors: %d\nMax_Alerts: %d\n", configs->QUEUE_SZ, configs->N_WORKERS, configs->MAX_KEYS, configs->MAX_SENSORS, configs->MAX_ALERTS);
    #endif

    //Criaçao shared memory
    tudo = create_shared_memory(configs->MAX_SENSORS);
    info = create_shared_memory_infos(configs);
    semaforo_log = create_shared_memory_log();
    alertas = create_shared_memory_alerts(configs->MAX_ALERTS);

    //Criaçao do semaforo mutex de acesso a shared memory
    sem_unlink("MUTEX");
	mutex_shm = sem_open("MUTEX", O_CREAT|O_EXCL, 0777, 1);

    write_log(PROG_START, semaforo_log);

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
    write_log(PROG_END, semaforo_log);

	//libertar semaforo mutex de acesso a shared memory
    sem_close(mutex_shm);
	sem_unlink("MUTEX");

    //libertar toda a shm
    get_rid_shm(tudo);
    get_rid_shm_infos(info);
    get_rid_shm_log(semaforo_log);
    get_rid_shm_alerts(alertas);
    
    exit(0);
}

