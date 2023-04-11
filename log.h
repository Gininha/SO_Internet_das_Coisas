#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <semaphore.h> 
#include <string.h>

#define PROG_START "Programa Iniciado\n"
#define PROG_END "Programa Finalizado\n"
#define DISPATCHER_START "Thread Dispatcher started\n"
#define DISPATCHER_END "Thread Dispatcher finished\n"
#define SENSOR_READER_START "Thread Sensor_Reader started\n"
#define SENSOR_READER_END "Thread Sensor_Reader finished\n"
#define CONSOLE_READER_START "Thread Console_Reader started\n"
#define CONSOLE_READER_END "Thread Console_Reader finished\n"
#define ALERTS_WATCHER_START "Process Alerts_Watcher started\n"
#define ALERTS_WATCHER_END "Process Alerts_Watcher finished\n"

#define CHAVE_LEN 33

typedef struct{
    int QUEUE_SZ;
    int N_WORKERS;
    int MAX_KEYS;
    int MAX_SENSORS;
    int MAX_ALERTS;
}Configuracoes;

typedef struct{
    char nome[CHAVE_LEN];
    int last_val;
    int min_val;
    int max_val;
    double media;
    int total;
}Registos;

typedef struct{
    int max_keys;
    int keys_atual;
    int max_sensors;
    int sensors_atual;
    int max_alerts;
    int alerts_atual;
}Infos;

typedef struct{
    sem_t mutex_log;
    pthread_mutex_t mutex_threads;
}Sem_Log;

void write_log(char *string, Sem_Log *semaforo_log);


Configuracoes *leitura_ficheiro(char *nome);
