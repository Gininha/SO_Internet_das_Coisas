/*
Luis Leite 2021199102
*/

#include "Shared_Memory.h"

#define DEBUG
#define NUM_THREADS 3

pthread_t threads[NUM_THREADS];
Infos *info;
Registos *tudo;
sem_t *mutex_shm, *mutex_fila_espera, *mutex_alertas;
sem_t *sensor_pipe, *console_pipe;
Sem_Log *semaforo_log;
Alertas *alertas;
pid_t *procs;
Configuracoes *configs;
Fila_espera *Queue = NULL;
int *procs_status;
int **worker_pipes;
int mq_id;


void cleanup(int singum){

    int i;
    char log_message[250];

    for (int i = 0; i < NUM_THREADS; i++) {
        
        pthread_cancel(threads[i]);
        
        if(i == 0){
            write_log(CONSOLE_READER_END, semaforo_log);
        }
        if(i == 1){
            write_log(SENSOR_READER_END, semaforo_log);
        }
        if(i == 2){
            write_log(DISPATCHER_END, semaforo_log);
        }
    }

    for(i = 0; i<configs->N_WORKERS + 1; i++){

        if(i == configs->N_WORKERS){

            write_log(ALERTS_WATCHER_END, semaforo_log);
            kill(procs[i], SIGTERM);
            waitpid(procs[i], NULL, 0); // Wait for child process to exit
        }else{

            sprintf(log_message, "%s %d %s", "Worker", procs[i], "leaving!!!\n");
            write_log(log_message, semaforo_log);
            kill(procs[i], SIGTERM);
            waitpid(procs[i], NULL, 0); // Wait for child process to exit
        }
    }

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

    //libertar semaforo mutex de acesso a shared memory
    sem_close(mutex_alertas);
	sem_unlink("MUTEX_ALERTAS");

    //Libertar semaforo mutex de acesso a fila de espera
    sem_close(mutex_fila_espera);
    sem_unlink("MUTEX_FILA_ESPERA");

    //Remove Message Queue
    if (msgctl(mq_id, IPC_RMID, NULL) == -1) {
        perror("msgctl");
        exit(1);
    }

    //libertar toda a shm
    get_rid_shm(tudo);
    get_rid_shm_infos(info);
    get_rid_shm_log(semaforo_log);
    get_rid_shm_alerts(alertas);
    get_rid_shm_Sensor_Pipe(sensor_pipe);
    get_rid_shm_Console_Pipe(console_pipe);


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

        if(process_task(buffer, tudo, alertas, info, semaforo_log)){

            pthread_mutex_lock(&info->mutex);

            info->condition = 1;
            pthread_cond_signal(&info->cond);
            
            pthread_mutex_unlock(&info->mutex);
        }

        sleep(1);
        procs_status[i] = 1;
        sem_post(&info->free_workers);

    }

    exit(0);
}

void alerts_watcher(){
    int i, j;
    MQ message_send;
    char buffer[500];

    #ifdef DEBUG
    printf("Alert Watcher [%d] created!!!\n", getpid());
    #endif

    write_log(ALERTS_WATCHER_START, semaforo_log);

    int alerta = 0;

    while(1){
        
        pthread_mutex_lock(&info->mutex);
        while (alerta == info->condition) {
            pthread_cond_wait(&info->cond, &info->mutex);
        }

        if(info->alerts_atual){
            for(i = 0; i < info->sensors_atual; i++){
                for(j = 0; j < info->alerts_atual; j++){
                    if((tudo[i].last_val > alertas[j].max || tudo[i].last_val < alertas[j].min) && strcmp(tudo[i].nome, alertas[j].nome) == 0){
                        message_send.id = alertas[j].user_console;
                        sprintf(buffer, "ALERT!!!\nvalue:%d   min:%d    max:%d\n", tudo[i].last_val, alertas[j].min, alertas[j].max);
                        strcpy(message_send.infos, buffer);
                        if (msgsnd(mq_id, &message_send, sizeof(MQ) - sizeof(long), 0) < 0) {
                            perror("msgsnd_alerts_watcher");
                            exit(1);
                        }
                        write_log(buffer, semaforo_log);
                        break;
                    }   
                }
            }
        }
        info->condition = 0;
        pthread_mutex_unlock(&info->mutex);
    }
    exit(0);
}


void *console_reader(void* p){
    int id = *((int*)p);
    int fd, quantidade_fila;
    char message[250], message_log[300];

    #ifdef DEBUG
    printf("Thread console_reader [%d] starting!!!\n", id);
    #endif

    write_log(CONSOLE_READER_START, semaforo_log);

    // Opens the pipe for reading
    if ((fd = open(CONSOLE_PIPE_NAME, O_RDWR)) < 0) {
        perror("Cannot open pipe for reading: ");
        exit(0); 
    }
    
    while (1) {

        read(fd, message, sizeof(message));
        printf("Console Reader-> %s\n", message);
        sprintf(message_log,"Console Reader-> %s\n", message);
        write_log(message_log, semaforo_log);

        Fila_espera *novo = malloc(sizeof(Fila_espera));

        printf("Console reader -> %s\n", message);
        novo->tipo = 'u';
        strcpy(novo->infos, message);
        novo->next = NULL;

        sem_wait(mutex_fila_espera);

        sem_getvalue(&info->empty, &quantidade_fila);
        if (quantidade_fila == 0) {
            sem_wait(&(info->empty));
        }

        Fila_espera *atual = Queue;
        if (atual == NULL) {
            Queue = novo;
        } else {
            while (atual->next != NULL) {
                atual = atual->next;
            }
            atual->next = novo;
        }

        sem_post(&(info->full));
        sem_post(mutex_fila_espera);
    }

    write_log(CONSOLE_READER_END, semaforo_log);
    
    pthread_exit(NULL);
}


void *sensor_reader(void* p){
    int id = *((int*)p);
    int fd;
    char message[250], message_log[300];
    int quantidade_fila;

    #ifdef DEBUG
    printf("Thread sensor_reader [%d] starting!!!\n", id);
    #endif

    write_log(SENSOR_READER_START, semaforo_log);

    // Opens the pipe for reading
    if ((fd = open(SENSOR_PIPE_NAME, O_RDWR)) < 0) {
        perror("Cannot open pipe for reading: ");
        exit(0);
    }

    while (1) {
        read(fd, message, sizeof(message));
        printf("Sensor Reader-> %s\n", message);
        sprintf(message_log, "Sensor Reader-> %s\n", message);
        write_log(message_log, semaforo_log);

        Fila_espera *novo = malloc(sizeof(Fila_espera));
        novo->tipo = 's';
        strcpy(novo->infos, message);
        novo->next = NULL;

        sem_wait(mutex_fila_espera);

        sem_getvalue(&info->empty, &quantidade_fila);
        if (quantidade_fila == 0) {
            printf(FILA_ESPERA_CHEIA);
            write_log(FILA_ESPERA_CHEIA, semaforo_log);
        } else {
            sem_getvalue(&info->full, &quantidade_fila);
            if (quantidade_fila == 0) {
                Queue = novo;
                sem_post(&(info->full));
                sem_wait(&(info->empty));
            } else {
                Fila_espera *atual = Queue;
                while (atual->next != NULL) {
                    atual = atual->next;
                }
                atual->next = novo;
                sem_post(&(info->full));
                sem_wait(&(info->empty));
            }
        }

        sem_post(mutex_fila_espera);

    }


    write_log(SENSOR_READER_END, semaforo_log);

    pthread_exit(NULL);
}


void *dispatcher(void* p){
    int id = *((int*)p);
    int worker_disponivel = -1;
    char buffer[1024], message_log[1024];

    #ifdef DEBUG
    printf("Thread dispatcher [%d] starting!!!\n", id);
    #endif

    write_log(DISPATCHER_START, semaforo_log);
    
    for(int i = 0; i<configs->N_WORKERS; i++){
        close(worker_pipes[i][0]); //Fecha os pipes para leitura
    }

    while(1){
        
        sem_wait(&info->free_workers);
        sem_wait(&info->full);
        sem_wait(mutex_fila_espera);

        for(int i = 0; i < configs->N_WORKERS; i++){
            if(procs_status[i]){
                worker_disponivel = i;
                break;
            }
        }  

        printf("Dispatcher-> %s\n", Queue->infos);
        sprintf(message_log, "Dispatcher-> Send: %s to Worker %d\n", Queue->infos, worker_disponivel+1);
        write_log(message_log, semaforo_log);

        sprintf(buffer, "%c#%s",Queue->tipo, Queue->infos);

        Fila_espera *aux = Queue;
        Queue = Queue->next;
        free(aux);

        sem_post(&info->empty);
        sem_post(mutex_fila_espera);

        //printf("%d -> %s\n", worker_disponivel, buffer);

        write(worker_pipes[worker_disponivel][1], buffer, strlen(buffer)+1);


        procs_status[worker_disponivel] = 0;
        
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

    //Criacao do semaforo mutex de acesso a shared memory
    sem_unlink("MUTEX");
	mutex_shm = sem_open("MUTEX", O_CREAT|O_EXCL, 0777, 1);

    //Criacao so semaforo mutex de acesso aos alertas
    sem_unlink("MUTEX_ALERTAS");
    mutex_alertas = sem_open("MUTEX_ALERTAS", O_CREAT|O_EXCL, 0777, 1);

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

    //Criacao dos unnamed semaphores (dispatcher -> workers)
    worker_pipes = (int **)malloc(configs->N_WORKERS * sizeof(int*));

    for(i = 0; i < configs->N_WORKERS; i++){
        worker_pipes[i] = (int*) malloc(2 * sizeof(int));
        pipe(worker_pipes[i]);
    }

    //Criacao Message Queue;
    if((mq_id = msgget(MQ_ID, IPC_CREAT|0777))<0){
        perror("msgget");
        exit(0);
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