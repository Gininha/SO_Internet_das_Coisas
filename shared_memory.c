/*
Luis Leite 2021199102
*/

#include "Shared_Memory.h"

int shmid, shmid2, shmid3, shmid4, shmid5, shmid6, shmid7;

Registos* open_shared_memory(int num_registos){

    Registos *pointer_registos;
    key_t key;

    if ((key = ftok(SHM_PROG, SHM_REG_ID)) == -1) {
        perror("ftok");
        exit(1);
    }

    // allocate shared memory
    if ((shmid = shmget(key, sizeof(Registos)*num_registos, 0777)) == -1) {
        perror("shmget");
        exit(1);
    }

    // attach shared memory to pointer
    if ((pointer_registos = (Registos*) shmat(shmid, NULL, 0)) == (void*) -1) {
        perror("shmat");
        exit(1);
    }

    return pointer_registos;
}

Registos* create_shared_memory(int num_registos) {

    Registos *pointer_registos;
    key_t key;

    if ((key = ftok(SHM_PROG, SHM_REG_ID)) == -1) {
        perror("ftok");
        exit(1);
    }

    // allocate shared memory
    if ((shmid = shmget(key, sizeof(Registos)*num_registos, IPC_CREAT | 0777)) == -1) {
        perror("shmget");
        exit(1);
    }

    // attach shared memory to pointer
    if ((pointer_registos = (Registos*) shmat(shmid, NULL, 0)) == (void*) -1) {
        perror("shmat");
        exit(1);
    }

    return pointer_registos;
}

Alertas* create_shared_memory_alerts(int num_registos){

    Alertas *pointer;
    key_t key;

    if ((key = ftok(SHM_PROG, SHM_ALERTS_ID)) == -1) {
        perror("ftok");
        exit(1);
    }

    // allocate shared memory
    if ((shmid6 = shmget(key, sizeof(Alertas)*num_registos, IPC_CREAT | 0777)) == -1) {
        perror("shmget");
        exit(1);
    }

    // attach shared memory to pointer
    if ((pointer = (Alertas*) shmat(shmid6, NULL, 0)) == (void*) -1) {
        perror("shmat");
        exit(1);
    }

    return pointer;
}


Infos* open_shared_memory_infos(){

    Infos *pointer_infos;
    key_t key;

    if ((key = ftok(SHM_PROG, SHM_INFOS_ID)) == -1) {
        perror("ftok");
        exit(1);
    }

    // allocate shared memory
    if ((shmid2 = shmget(key, sizeof(Infos), 0777)) == -1) {
        perror("shmget");
        exit(1);
    }

    // attach shared memory to pointer
    if ((pointer_infos = (Infos*) shmat(shmid2, NULL, 0)) == (void*) -1) {
        perror("shmat");
        exit(1);
    }

    return pointer_infos;
}

Infos* create_shared_memory_infos(Configuracoes *configs){

    Infos *pointer_infos;
    key_t key;

    if ((key = ftok(SHM_PROG, SHM_INFOS_ID)) == -1) {
        perror("ftok");
        exit(1);
    }

    // allocate shared memory
    if ((shmid2 = shmget(key, sizeof(Infos), IPC_CREAT | 0777)) == -1) {
        perror("shmget");
        exit(1);
    }

    // attach shared memory to pointer
    if ((pointer_infos = (Infos*) shmat(shmid2, NULL, 0)) == (void*) -1) {
        perror("shmat");
        exit(1);
    }

    pointer_infos->alerts_atual = 0;
    pointer_infos->keys_atual = 0;
    pointer_infos->sensors_atual = 0;
    pointer_infos->condition = 0;
    pointer_infos->max_alerts = configs->MAX_ALERTS;
    pointer_infos->max_keys = configs->MAX_KEYS;
    pointer_infos->max_sensors = configs->MAX_SENSORS;

    pthread_mutexattr_t attrmutex;
    pthread_condattr_t attrcondv;

    /* Initialize attribute of mutex. */
    pthread_mutexattr_init(&attrmutex);
    pthread_mutexattr_setpshared(&attrmutex, PTHREAD_PROCESS_SHARED);

    /* Initialize attribute of condition variable. */
    pthread_condattr_init(&attrcondv);
    pthread_condattr_setpshared(&attrcondv, PTHREAD_PROCESS_SHARED);

    pthread_cond_init(&pointer_infos->cond, &attrcondv);
    pthread_mutex_init(&pointer_infos->mutex, &attrmutex);

    sem_init(&(pointer_infos->empty), 1, configs->QUEUE_SZ);
    sem_init(&(pointer_infos->full), 1, 0);
    sem_init(&(pointer_infos->free_workers), 1, 0);

    return pointer_infos;
}


int* create_worker_status(Configuracoes* configs){

    int *status;
    key_t key;

    if ((key = ftok(SHM_PROG, SHM_STATUS)) == -1) {
        perror("ftok");
        exit(1);
    }

    // allocate shared memory
    if ((shmid = shmget(key, sizeof(int)*configs->N_WORKERS, IPC_CREAT | 0777)) == -1) {
        perror("shmget");
        exit(1);
    }

    // attach shared memory to pointer
    if ((status = (int*) shmat(shmid, NULL, 0)) == (void*) -1) {
        perror("shmat");
        exit(1);
    }

    for(int i=0; i < configs->N_WORKERS; i++){
        status[i] = 0;
    }

    return status;
}

Sem_Log* open_shared_memory_log(){

    Sem_Log *pointer;
    key_t key;

    if ((key = ftok(SHM_PROG, SHM_LOG_ID)) == -1) {
        perror("ftok");
        exit(1);
    }

    // allocate shared memory
    if ((shmid3 = shmget(key, sizeof(Sem_Log), 0777)) == -1) {
        perror("shmget");
        exit(1);
    }

    // attach shared memory to pointer
    if ((pointer = (Sem_Log*) shmat(shmid3, NULL, 0)) == (void*) -1) {
        perror("shmat");
        exit(1);
    }

    pointer->log_fd= open("log.txt", O_CREAT | O_WRONLY | O_APPEND, 0644);
    if (pointer->log_fd == -1) {
        perror("Error opening log file");
        exit(EXIT_FAILURE);
    }

    return pointer;
}

Sem_Log* create_shared_memory_log(){

    Sem_Log *pointer;
    key_t key;

    if ((key = ftok(SHM_PROG, SHM_LOG_ID)) == -1) {
        perror("ftok");
        exit(1);
    }

    // allocate shared memory
    if ((shmid3 = shmget(key, sizeof(Sem_Log), IPC_CREAT | 0777)) == -1) {
        perror("shmget");
        exit(1);
    }

    // attach shared memory to pointer
    if ((pointer = (Sem_Log*) shmat(shmid3, NULL, 0)) == (void*) -1) {
        perror("shmat");
        exit(1);
    }

    sem_init(&(pointer->mutex_log), 1, 1);

    pthread_mutex_init(&(pointer->mutex_threads), NULL);

    pointer->log_fd= open("log.txt", O_CREAT | O_WRONLY | O_APPEND, 0644);
    if (pointer->log_fd == -1) {
        perror("Error opening log file");
        exit(EXIT_FAILURE);
    }

    return pointer;
}

sem_t* create_shared_memory_Console_Pipe(){

    sem_t *mutex;
    key_t key;

    if ((key = ftok(SHM_PROG, SHM_CONSOLE_PIPE_ID)) == -1) {
        perror("ftok");
        exit(1);
    }

    // allocate shared memory
    if ((shmid5 = shmget(key, sizeof(sem_t), IPC_CREAT | 0777)) == -1) {
        perror("shmget");
        exit(1);
    }

    // attach shared memory to pointer
    if ((mutex = (sem_t*) shmat(shmid5, NULL, 0)) == (void*) -1) {
        perror("shmat");
        exit(1);
    }

    sem_init(mutex, 1, 1);

    return mutex;
}

sem_t* create_shared_memory_Sensor_Pipe(){

    sem_t *mutex;
    key_t key;

    if ((key = ftok(SHM_PROG, SHM_SENSOR_PIPE_ID)) == -1) {
        perror("ftok");
        exit(1);
    }

    // allocate shared memory
    if ((shmid4 = shmget(key, sizeof(sem_t), IPC_CREAT | 0777)) == -1) {
        perror("shmget");
        exit(1);
    }

    // attach shared memory to pointer
    if ((mutex = (sem_t*) shmat(shmid4, NULL, 0)) == (void*) -1) {
        perror("shmat");
        exit(1);
    }

    sem_init(mutex, 1, 1);

    return mutex;
}

sem_t* open_shared_memory_Sensor_Pipe(){

    sem_t *mutex;
    key_t key;

    if ((key = ftok(SHM_PROG, SHM_SENSOR_PIPE_ID)) == -1) {
        perror("ftok");
        exit(1);
    }

    // allocate shared memory
    if ((shmid4 = shmget(key, sizeof(sem_t), 0777)) == -1) {
        perror("shmget");
        exit(1);
    }

    // attach shared memory to pointer
    if ((mutex = (sem_t*) shmat(shmid4, NULL, 0)) == (void*) -1) {
        perror("shmat");
        exit(1);
    }

    sem_init(mutex, 1, 1);

    return mutex;
}

sem_t* open_shared_memory_Console_Pipe(){

    sem_t *mutex;
    key_t key;

    if ((key = ftok(SHM_PROG, SHM_CONSOLE_PIPE_ID)) == -1) {
        perror("ftok");
        exit(1);
    }

    // allocate shared memory
    if ((shmid4 = shmget(key, sizeof(sem_t), 0777)) == -1) {
        perror("shmget");
        exit(1);
    }

    // attach shared memory to pointer
    if ((mutex = (sem_t*) shmat(shmid4, NULL, 0)) == (void*) -1) {
        perror("shmat");
        exit(1);
    }

    sem_init(mutex, 1, 1);

    return mutex;
}

void get_rid_shm(Registos *registo){
    // Detach shared memory
    if (shmdt(registo) == -1) {
        perror("shmdt");
        exit(1);
    }

    // Destroy shared memory
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("shmctl");
        exit(1);
    }
}

void get_rid_shm_alerts(Alertas *alertas){
    // Detach shared memory
    if (shmdt(alertas) == -1) {
        perror("shmdt");
        exit(1);
    }

    // Destroy shared memory
    if (shmctl(shmid6, IPC_RMID, NULL) == -1) {
        perror("shmctl");
        exit(1);
    }
    
}

void get_rid_shm_infos(Infos *infos){

    sem_destroy(&infos->empty);
    sem_destroy(&infos->full);

    // Detach shared memory
    if (shmdt(infos) == -1) {
        perror("shmdt");
        exit(1);
    }

    // Destroy shared memory
    if (shmctl(shmid2, IPC_RMID, NULL) == -1) {
        perror("shmctl");
        exit(1);
    }

}

void get_rid_shm_log(Sem_Log *log){

    sem_destroy(&(log->mutex_log));

    pthread_mutex_destroy(&(log->mutex_threads));

    // Detach shared memory
    if (shmdt(log) == -1) {
        perror("shmdt");
        exit(1);
    }

    // Destroy shared memory
    if (shmctl(shmid3, IPC_RMID, NULL) == -1) {
        perror("shmctl");
        exit(1);
    }

}

void get_rid_shm_Sensor_Pipe(sem_t* sem){

    // Detach shared memory
    if (shmdt(sem) == -1) {
        perror("shmdt_sensor_pipe");
        exit(1);
    }

    // Destroy shared memory
    if (shmctl(shmid4, IPC_RMID, NULL) == -1) {
        perror("shmctl_sensor_pipe");
        exit(1);
    }

}

void get_rid_shm_Console_Pipe(sem_t* sem){

    // Detach shared memory
    if (shmdt(sem) == -1) {
        perror("shmdt");
        exit(1);
    }

    // Destroy shared memory
    if (shmctl(shmid5, IPC_RMID, NULL) == -1) {
        perror("shmctl");
        exit(1);
    }

}


void print_shared_memory(Registos *Pointer, Infos *infos){

    int i;

    printf("Quantidade de registos: %d\n", infos->keys_atual);

    for(i=0; i<infos->keys_atual; i++){
        printf("Registo[%d] -> Chave: %s\tMax val: %d\tMin val: %d\tMedia: %.2f\tLast_Val: %d\tTotal: %d\n",i+1, Pointer[i].nome, Pointer[i].max_val, Pointer[i].min_val, Pointer[i].media, Pointer[i].last_val, Pointer[i].total);
    }
}

void print_sensors(Registos *Pointer, Infos *infos, MQ *message_send){
    int i;
    char buffer[500];
    message_send->infos[0] = '\0';

    strcat(message_send->infos, "Sensors\tID\n");

    for(i = 0; i < infos->sensors_atual; i++){
        sprintf(buffer, "%s\n", Pointer[i].nome);
        strcat(message_send->infos, buffer);
    }
}

void reset(Registos *Pointer, Infos *infos, MQ *message_send){
    int i;
    message_send->infos[0] = '\0';

    for(i = 0; i < infos->sensors_atual;){
        Pointer[i].id[0] = '\0';
        Pointer[i].nome[0] = '\0';
        Pointer[i].last_val = 0;
        Pointer[i].max_val = 0;
        Pointer[i].media = 0;
        Pointer[i].min_val = 0;
        Pointer[i].soma = 0;
        Pointer[i].total = 0;
        infos->sensors_atual--;
        infos->keys_atual--;
    }

    strcat(message_send->infos, "OK\n");
}

void print_stats(Registos *Pointer, Infos *infos, MQ *message_send){

    int i;
    char buffer[500];
    message_send->infos[0] = '\0';

    strcat(message_send->infos, "Stats\tKEY\tLAST\tMIN\tMAX\tAVG\tCOUNT\n");

    for(i=0; i<infos->sensors_atual; i++){
        sprintf(buffer, "Registo[%d] -> Chave: %s\tLast_Val: %d\tMin val: %d\tMax val: %d\tMedia: %.2f\ttTotal: %d\n",i+1, Pointer[i].nome, Pointer[i].last_val, Pointer[i].min_val, Pointer[i].max_val, Pointer[i].media, Pointer[i].total);
        strcat(message_send->infos, buffer);
    }

}

void list_alerts(Alertas *alertas, Infos *infos, MQ *message_send){
    int i;
    char buffer[500];
    message_send->infos[0] = '\0';

    strcat(message_send->infos, "Alerts List\tID\tKEY\tMAX\tMIN\n");

    for(i=0; i<infos->alerts_atual; i++){
        sprintf(buffer, "Alerta[%d] -> Id: %s   Key: %s   Max: %d    Min: %d\n",i+1, alertas[i].id, alertas[i].nome, alertas[i].max, alertas[i].min);
        strcat(message_send->infos, buffer);
    }
}

int add_alert(Alertas *alertas, Infos *infos, Alertas *alerta_add){
    int i;

    if(infos->keys_atual == infos->max_keys || infos->alerts_atual == infos->max_alerts){

        printf("Alertas maximos / Keys maximas atingido\n");
        return 0;
    }

    for(i = 0; i < infos->alerts_atual; i++){
        if(strcmp(alertas->id, alerta_add->id) == 0){
            return -1;
        }
    }


    strcpy(alertas[infos->alerts_atual].id, alerta_add->id);
    strcpy(alertas[infos->alerts_atual].nome, alerta_add->nome);
    alertas[infos->alerts_atual].max = alerta_add->max;
    alertas[infos->alerts_atual].min = alerta_add->min;
    alertas[infos->alerts_atual].user_console = alerta_add->user_console;
    infos->alerts_atual++;
    infos->keys_atual++;
    return 1;

}

int remove_alert(Alertas *alertas, Infos *infos, char *id_remove){
    
    int i, j;

    for(i=0; i<infos->alerts_atual; i++){
        if((strcmp(id_remove, alertas[i].id) == 0)){
            if(i == infos->alerts_atual-1){
                alertas[i].nome[0] = '\0';
                alertas[i].id[0] = '\0';
                infos->alerts_atual--;
                infos->keys_atual--;
                return 1;
            }
            for(j = i; j<infos->alerts_atual-1; j++){
                strcpy(alertas[j].id, alertas[j+1].id);
                strcpy(alertas[j].nome, alertas[j+1].nome);
                alertas[j].min = alertas[j+1].min;
                alertas[j].max = alertas[j+1].max;
            }
            infos->alerts_atual--;
            infos->keys_atual--;
            return 1;
        }
    }

    return 0;
}

int write_to_shared_memory(Registos *Pointer, Infos *infos, Sensor_thread *registo){

    int i;

    if(infos->keys_atual == infos->max_keys || infos->sensors_atual == infos->max_sensors){

        for(i=0; i<infos->sensors_atual; i++){

            if(strcmp(Pointer[i].id, registo->id_sensor) == 0){

                Pointer[i].last_val = registo->value;
                if(Pointer[i].min_val > registo->value)
                    Pointer[i].min_val = registo->value;
                if(Pointer[i].max_val < registo->value)
                    Pointer[i].max_val = registo->value;
                Pointer[i].soma += registo->value;
                Pointer[i].total++;
                Pointer[i].media = (float)Pointer[i].soma/Pointer[i].total;
                return 0;
            }
        }
        
        printf("Lotaçao maxima\n");
        return 1;
    }

    if(infos->sensors_atual == 0){
        strcpy(Pointer[0].id, registo->id_sensor);
        strcpy(Pointer[0].nome, registo->chave);
        Pointer[0].last_val = registo->value;
        Pointer[0].min_val = registo->value;
        Pointer[0].max_val = registo->value;
        Pointer[0].soma = registo->value;
        Pointer[0].total = 1;
        Pointer[0].media = registo->value;
        infos->keys_atual++;
        infos->sensors_atual++;
        return 0;
    }

    for(i=0; i<infos->sensors_atual; i++){

        if(strcmp(Pointer[i].id, registo->id_sensor) == 0){

            Pointer[i].last_val = registo->value;
            if(Pointer[i].min_val > registo->value)
                Pointer[i].min_val = registo->value;
            if(Pointer[i].max_val < registo->value)
                Pointer[i].max_val = registo->value;
            Pointer[i].soma += registo->value;
            Pointer[i].total++;
            Pointer[i].media = Pointer[i].soma/Pointer[i].total;
            return 0;
        }
    }

    strcpy(Pointer[i].id, registo->id_sensor);
    strcpy(Pointer[i].nome, registo->chave);
    Pointer[i].last_val = registo->value;
    Pointer[i].min_val = registo->value;
    Pointer[i].max_val = registo->value;
    Pointer[i].media = registo->value;
    Pointer[i].total = 1;
    Pointer[i].soma = registo->value;
    infos->keys_atual++;
    infos->sensors_atual++;
    return 0;
}