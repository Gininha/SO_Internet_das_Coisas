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

typedef struct{
    int QUEUE_SZ;
    int N_WORKERS;
    int MAX_KEYS;
    int MAX_SENSORS;
    int MAX_ALERTS;
}Configuracoes;

void write_to_log(char *string);

Configuracoes *leitura_ficheiro(char *nome);
