/*
Luis Leite 2021199102
*/

#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <semaphore.h> 
#include <pthread.h>
#include <signal.h>

#include "log.h"

#define SHM_PROG "/tmp"
#define SHM_REG_ID 1
#define SHM_INFOS_ID 2
#define SHM_LOG_ID 3
#define SHM_SENSOR_PIPE_ID 4
#define SHM_CONSOLE_PIPE_ID 5
#define SHM_ALERTS_ID 6
#define SHM_STATUS 7

Registos* create_shared_memory(int num_registos);
Infos* create_shared_memory_infos(Configuracoes* configs);
Sem_Log* create_shared_memory_log();
sem_t* create_shared_memory_Sensor_Pipe();
sem_t* create_shared_memory_Console_Pipe();
Alertas* create_shared_memory_alerts(int num_registos);
int* create_worker_status(Configuracoes* configs);

Registos* open_shared_memory(int num_registos);
Infos* open_shared_memory_infos();
Sem_Log* open_shared_memory_log();
sem_t* open_shared_memory_Sensor_Pipe();
sem_t* open_shared_memory_Console_Pipe();

void get_rid_shm(Registos *registo);
void get_rid_shm_infos(Infos *infos);
void get_rid_shm_log(Sem_Log *log);
void get_rid_shm_Sensor_Pipe(sem_t* sem);
void get_rid_shm_Console_Pipe(sem_t* sem);
void get_rid_shm_alerts(Alertas *alertas);

int write_to_shared_memory(Registos *Pointer, Infos *infos, Sensor_thread *registo);
void print_shared_memory(Registos *Pointer, Infos *infos);