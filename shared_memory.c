#include "Shared_Memory.h"

int shmid, shmid2, shmid3, shmid4, shmid5;

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
    pointer_infos->max_alerts = configs->MAX_ALERTS;
    pointer_infos->max_keys = configs->MAX_KEYS;
    pointer_infos->max_sensors = configs->MAX_SENSORS;

    return pointer_infos;
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

void get_rid_shm_infos(Infos *infos){
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

void get_rid_shm_User_Pipe(sem_t* sem){

    sem_destroy(sem);

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

void get_rid_shm_Console_Pipe(sem_t* sem){

    sem_destroy(sem);

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
        printf("Registo[%d] -> Nome: %s\tMax val: %d\tMin val: %d\tMedia: %.2f\tLast_Val: %d\tTotal: %d\n",i+1, Pointer[i].nome, Pointer[i].max_val, Pointer[i].min_val, Pointer[i].media, Pointer[i].last_val, Pointer[i].total);
    }
}

int write_to_shared_memory(Registos *Pointer, Infos *infos, Registos *registo){

    int i;

    if(infos->keys_atual == infos->max_keys || infos->sensors_atual == infos->max_sensors){

        for(i=0; i<infos->sensors_atual; i++){

            if(strcmp(Pointer[i].nome, registo->nome) == 0){

                Pointer[i].last_val = registo->last_val;
                Pointer[i].min_val = registo->min_val;
                Pointer[i].max_val = registo->max_val;
                Pointer[i].media = registo->media;
                Pointer[i].total++;
                return 0;
            }
        }
        
        printf("Lotaçao maxima\n");
        return 1;
    }

    if(infos->sensors_atual == 0){
        strcpy(Pointer[0].nome, registo->nome);
        Pointer[0].last_val = registo->last_val;
        Pointer[0].min_val = registo->min_val;
        Pointer[0].max_val = registo->max_val;
        Pointer[0].media = registo->media;
        Pointer[0].total = registo->total;
        infos->keys_atual++;
        infos->sensors_atual++;
        return 0;
    }

    for(i=0; i<infos->sensors_atual; i++){

        if(strcmp(Pointer[i].nome, registo->nome) == 0){

            Pointer[i].last_val = registo->last_val;
            Pointer[i].min_val = registo->min_val;
            Pointer[i].max_val = registo->max_val;
            Pointer[i].media = registo->media;
            Pointer[i].total++;
            return 0;
        }
    }

    strcpy(Pointer[i].nome, registo->nome);
    Pointer[i].last_val = registo->last_val;
    Pointer[i].min_val = registo->min_val;
    Pointer[i].max_val = registo->max_val;
    Pointer[i].media = registo->media;
    Pointer[i].total = registo->total;
    infos->keys_atual++;
    infos->sensors_atual++;
    return 0;
}