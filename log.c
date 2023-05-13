/*
Luis Leite 2021199102
*/
#include "log.h"


void write_log(char *string, Sem_Log *semaforo_log){
    
    sem_wait(&(semaforo_log->mutex_log));
    pthread_mutex_lock(&(semaforo_log->mutex_threads));

    time_t now = time(NULL);           // get the current time
    struct tm *local_time = localtime(&now);  // convert it to local time
    char time_str[10];
    strftime(time_str, sizeof(time_str), "%T ", local_time);  // format time as HH:MM:SS string

    if (write(semaforo_log->log_fd, time_str, strlen(time_str)) == -1) {
        perror("Error writing to log file");
        return;
    }

    if (write(semaforo_log->log_fd, string, strlen(string)) == -1) {
        perror("Error writing to log file");
        return;
    }

    pthread_mutex_unlock(&(semaforo_log->mutex_threads));
    sem_post(&(semaforo_log->mutex_log));
}


Configuracoes* leitura_ficheiro(char *nome){

    FILE * file = fopen(nome, "r");
    if(file == NULL){
        printf("Ficheiro nao encontrado\n");
        return NULL;
    }
    int a,b,c,d,e;
    Configuracoes *configs = malloc(sizeof(Configuracoes));

    if (fscanf(file, "%d %d %d %d %d", &a, &b, &c, &d, &e) != 5) {
        printf("Erro ao ler ficheiro\n");
    }

    configs->QUEUE_SZ = a;
    configs->N_WORKERS = b;
    configs->MAX_KEYS = c;
    configs->MAX_SENSORS = d;
    configs->MAX_ALERTS = e;

    fclose(file);

    return configs;
}