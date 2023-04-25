/*
Luis Leite 2021199102
*/

#include "Shared_Memory.h"

#define DEBUG
#define NUM_THREADS 3

pthread_t threads[NUM_THREADS];
Infos *info;
Registos *tudo;
sem_t *mutex_shm, *mutex_fila_espera;
sem_t *sensor_pipe, *console_pipe;
Sem_Log *semaforo_log;
Alertas *alertas;
pid_t *procs;
Configuracoes *configs;
Fila_espera *Queue = NULL;
int *procs_status;
int **worker_pipes;

int stop_threads_flag = 0;

void cleanup(int singum){

    int i;
    char log_message[250];
    stop_threads_flag = 1;

    for (int i = 0; i < NUM_THREADS; i++) {
        //pthread_join(threads[i], NULL);
        pthread_cancel(threads[i]);
    }

    for(i = 0; i<configs->N_WORKERS + 1; i++){

        if(i == configs->N_WORKERS){

            write_log(ALERTS_WATCHER_END, semaforo_log);
            kill(procs[i], SIGTERM);

        }else{

            sprintf(log_message, "%s %d %s", "Worker", procs[i], "leaving!!!\n");
            write_log(log_message, semaforo_log);
            kill(procs[i], SIGTERM);
        }
    }

    print_shared_memory(tudo, info);

    write_log(PROG_END, semaforo_log);

    //Libertar Fila
    Fila_espera *temp;
    while(Queue != NULL){
        temp = Queue;
        Queue = Queue->next;
        free(temp);
    }

    //libertar semaforo mutex de acesso a shared memory
    sem_close(mutex_shm);
	sem_unlink("MUTEX");

    //Libertar semaforo mutex de acesso a fila de espera
    sem_close(mutex_fila_espera);
    sem_unlink("MUTEX_FILA_ESPERA");

    //libertar toda a shm
    get_rid_shm(tudo);
    printf("1\n");
    get_rid_shm_infos(info);
    printf("2\n");
    get_rid_shm_log(semaforo_log);
    printf("3\n");
    get_rid_shm_alerts(alertas);
    printf("4\n");
    get_rid_shm_Sensor_Pipe(sensor_pipe);
    printf("5\n");
    get_rid_shm_Console_Pipe(console_pipe);
    printf("6\n");

    exit(0);

}


void worker(int i){
    
    char log_message[250];
    char buffer[250];

    #ifdef DEBUG
    printf("Worker [%d] created!!!\n", i);
    #endif

    sprintf(log_message, "%s %d %s", "Worker", getpid(), "created!!!\n");

    write_log(log_message, semaforo_log);

    close(worker_pipes[i][1]); //Fechar o pipe para escrita escrita

    sem_wait(mutex_shm);
    procs_status[i] = 1;
    sem_post(&info->free_workers);
    sem_post(mutex_shm);

    while(1){
        read(worker_pipes[i][0], buffer, 250);

        process_task(buffer, tudo, info);

        sem_wait(mutex_shm);
        procs_status[i] = 1;
        sem_post(mutex_shm);
        sem_post(&info->free_workers);

        sleep(1);
    }

    exit(0);
}

void alerts_watcher(){
    #ifdef DEBUG
    printf("Alert Watcher [%d] created!!!\n", getpid());
    #endif

    write_log(ALERTS_WATCHER_START, semaforo_log);

    while(1){
        sleep(1);
    }
    
    exit(0);
}


void *console_reader(void* p){
    int id = *((int*)p);
    int fd;
    char message[250];

    #ifdef DEBUG
    printf("Thread console_reader [%d] starting!!!\n", id);
    #endif

    write_log(CONSOLE_READER_START, semaforo_log);
    
    while(!stop_threads_flag){

        // Opens the pipe for reading
        if ((fd = open(CONSOLE_PIPE_NAME, O_RDONLY)) < 0) {
            perror("Cannot open pipe for reading: ");
            exit(0); 
        }

        read(fd, message, sizeof(message));

        printf("Console reader -> %s\n", message);


        sleep(1);
    }

    write_log(CONSOLE_READER_END, semaforo_log);
    
    pthread_exit(NULL);
}


void *sensor_reader(void* p){
    int id = *((int*)p);
    int fd;
    char *token;
    char message[250];
    int quantidade_fila;

    #ifdef DEBUG
    printf("Thread sensor_reader [%d] starting!!!\n", id);
    #endif

    write_log(SENSOR_READER_START, semaforo_log);

    while(!stop_threads_flag){

        // Opens the pipe for reading
        if ((fd = open(SENSOR_PIPE_NAME, O_RDONLY)) < 0) {
            perror("Cannot open pipe for reading: ");
            exit(0);
        }

        read(fd, message, sizeof(message));
        write_log(message, semaforo_log);
        printf("Sensor Reader-> %s\n", message);
        
        Sensor_thread *temp = (Sensor_thread*)malloc(sizeof(Sensor_thread));

        if((token = strtok(message, "#")) == NULL){
            continue;
        }
        strcpy(temp->id_sensor, token);
        if((token = strtok(NULL, "#")) == NULL){
            continue;
        }
        strcpy(temp->chave, token);
        if((token = strtok(NULL, "#")) == NULL){
            continue;
        }
        temp->value = atoi(token);

        Fila_espera *novo = (Fila_espera*)malloc(sizeof(Fila_espera));
        novo->infos = (Sensor_thread*) temp;
        novo->tipo = 's';
        novo->next = NULL;

        sem_wait(mutex_fila_espera);

        sem_getvalue(&info->empty, &quantidade_fila);
        if(quantidade_fila == 0){
            printf(FILA_ESPERA_CHEIA);
            write_log(FILA_ESPERA_CHEIA, semaforo_log);

        }else{
            sem_getvalue(&info->full, &quantidade_fila);
            if(quantidade_fila == 0){
                Queue = novo;
                sem_post(&(info->full));
                sem_wait(&(info->empty));
            }else{
                Fila_espera *atual = Queue;
                while(atual->next != NULL){
                    atual = atual->next;
                }
                atual->next = novo;
                sem_post(&(info->full));
                sem_wait(&(info->empty));
            }
        }
        
        sem_post(mutex_fila_espera);
    
        sleep(1);
    }

    write_log(SENSOR_READER_END, semaforo_log);

    pthread_exit(NULL);
}


void *dispatcher(void* p){
    int id = *((int*)p);
    Sensor_thread *sensor;
    int worker_disponivel = -1;
    char buffer[250], tipo;

    #ifdef DEBUG
    printf("Thread dispatcher [%d] starting!!!\n", id);
    #endif

    write_log(DISPATCHER_START, semaforo_log);
    
    for(int i = 0; i<configs->N_WORKERS; i++){
        close(worker_pipes[i][0]); //Fecha os pipes para leitura
    }

    while(!stop_threads_flag){
        
        sem_wait(&info->free_workers);
        sem_wait(&info->full);
        sem_wait(mutex_fila_espera);

        for(int i = 0; i < configs->N_WORKERS; i++){
            if(procs_status[i]){
                worker_disponivel = i;
                break;
            }
        }

        sensor = (Sensor_thread*)Queue->infos;
        tipo = (char)Queue->tipo;
        Queue = Queue->next;

        sem_post(&info->empty);
        sem_post(mutex_fila_espera);

        sprintf(buffer, "%c#%s#%s#%d",tipo, sensor->id_sensor, sensor->chave, sensor->value);
        //printf("%d -> %s\n", worker_disponivel, buffer);

        write(worker_pipes[worker_disponivel][1], buffer, strlen(buffer)+1);

        sem_wait(mutex_shm);
        procs_status[worker_disponivel] = 0;
        sem_post(mutex_shm);

        sleep(1);
    }

    write_log(DISPATCHER_END, semaforo_log);

    pthread_exit(NULL);
}


int main(int argc, char *argv[]){

    if(argc != 2){
        printf("./home_iot <ficheiro_config>\n");
        return 0;
    }
    signal(SIGINT, cleanup);

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
    sensor_pipe = create_shared_memory_Sensor_Pipe();
    console_pipe = create_shared_memory_Console_Pipe();
    procs_status = create_worker_status(configs);

    //Criaçao do semaforo mutex de acesso a shared memory
    sem_unlink("MUTEX");
	mutex_shm = sem_open("MUTEX", O_CREAT|O_EXCL, 0777, 1);

    //Criacao do semaforo mutex para acesso a fila de espera
    sem_unlink("MUTEX_FILA_ESPERA");
    mutex_fila_espera = sem_open("MUTEX_FILA_ESPERA", O_CREAT|O_EXCL, 0777, 1);

    //Criaçao do named pipe sensor
    if ((mkfifo(SENSOR_PIPE_NAME, O_CREAT|O_EXCL|0777)<0) && (errno!= EEXIST)) {
        perror("Cannot create pipe: ");
        exit(0);
    }

    //Criaçao do named pipe user console
    if ((mkfifo(CONSOLE_PIPE_NAME, O_CREAT|O_EXCL|0777)<0) && (errno!= EEXIST)) {
        perror("Cannot create pipe: ");
        exit(0);
    }

    //Criacao dos unnamed semaphores
    worker_pipes = (int **)malloc(configs->N_WORKERS * sizeof(int*));

    for(i = 0; i < configs->N_WORKERS; i++){
        worker_pipes[i] = (int*) malloc(2 * sizeof(int));
        pipe(worker_pipes[i]);
    }

    write_log(PROG_START, semaforo_log);

    procs = malloc(sizeof(pid_t) * (configs->N_WORKERS + 1));

    //Criaçao processos worker
    for(i = 0; i<configs->N_WORKERS; i++){
        if((procs[i] = fork()) == 0){
            signal(SIGINT, SIG_IGN);
			worker(i);
			exit(0);

        }
    }

    //Criaçao processo alerts_watcher
    if((procs[i] = fork()) == 0){
        signal(SIGINT, SIG_IGN);
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


    while(1){
        sleep(1);
    }

    exit(0);
}

