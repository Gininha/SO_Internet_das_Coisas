/*
Luis Leite 2021199102
*/

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
#include <errno.h>
#include <sys/stat.h>
#include <sys/msg.h>

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
#define FILA_ESPERA_CHEIA "Lotaçao cheia -> sensor descartado!!!\n"

#define SENSOR_PIPE_NAME "SENSOR_PIPE"
#define CONSOLE_PIPE_NAME "CONSOLE_PIPE"

#define MQ_ID 1447

#define CHAVE_LEN 33

typedef struct{
    int QUEUE_SZ;
    int N_WORKERS;
    int MAX_KEYS;
    int MAX_SENSORS;
    int MAX_ALERTS;
}Configuracoes;

typedef struct{
    char id[CHAVE_LEN];
    char nome[CHAVE_LEN];
    int last_val;
    int min_val;
    int max_val;
    double media;
    int soma;
    int total;
}Registos;

typedef struct{
    char id[CHAVE_LEN];
    char nome[CHAVE_LEN];
    int min;
    int max;
    int user_console;
}Alertas;

typedef struct{
    int max_keys;
    int keys_atual;
    int max_sensors;
    int sensors_atual;
    int max_alerts;
    int alerts_atual;
    int msq_id;
    int condition;
    sem_t full;
    sem_t empty;
    sem_t free_workers;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
}Infos;

typedef struct{
    sem_t mutex_log;
    pthread_mutex_t mutex_threads;
    int log_fd;
}Sem_Log;

typedef struct Fila_espera{
    char infos[512];
    char tipo;                  //s -> sensor || u -> user
    struct Fila_espera *next;
}Fila_espera;

typedef struct{
    char id_sensor[CHAVE_LEN];
    char chave[CHAVE_LEN];
    int value;
}Sensor_thread;

typedef struct MQ{
    long id;
    char infos[500];
}MQ;

void write_log(char *string, Sem_Log *semaforo_log);

Configuracoes *leitura_ficheiro(char *nome);

int process_task(char *buffer, Registos* registos, Alertas* alertas, Infos* infos, Sem_Log *semaforo_log);
