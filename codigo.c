#include "Shared_Memory.h"

//resolver questao do incremento variaveis em infos !!!

sem_t *mutex_shm_sensores, *mutex_shm_alertas;

int process_task(char *buffer, Registos* registos, Alertas* alertas, Infos* infos, Sem_Log *semaforo_log){
    char *token, id_remove[CHAVE_LEN];
    int msg_id, mq_id;
    Sensor_thread *sensor = malloc(sizeof(Sensor_thread));
    Alertas *alerta_add = malloc(sizeof(Alertas));
    MQ message_send;

    if ((mq_id = msgget(MQ_ID, 0777)) < 0) {
        perror("msgget");
        exit(1);
    }
    mutex_shm_sensores = sem_open("MUTEX", 0);
    mutex_shm_alertas = sem_open("MUTEX_ALERTAS", 0);

    if((token = strtok(buffer, "#")) == NULL){
            return 0;
        }
        
        if(strcmp(token, "s") == 0){
            if((token = strtok(NULL, "#")) == NULL){
            return 0;
            }

            strcpy(sensor->id_sensor, token);

            if((token = strtok(NULL, "#")) == NULL){
                return 0;
            }

            strcpy(sensor->chave, token);

            if((token = strtok(NULL, "#")) == NULL){
                return 0;
            }

            sensor->value = atoi(token);

            sem_wait(mutex_shm_sensores);
            write_to_shared_memory(registos, infos, sensor);
            sem_post(mutex_shm_sensores);

            write_log("WORKER -> READ FROM SENSOR\n", semaforo_log);

            return 1;
    
        }

        if(strcmp(token, "u") == 0){

            if((token = strtok(NULL, "#")) == NULL){
                return 0;
            }

            if(strcmp(token, "stats") == 0){
                if((token = strtok(NULL, "#")) == NULL){
                    return 0;
                }
                msg_id = atoi(token);

                sem_wait(mutex_shm_sensores);
                print_stats(registos, infos, &message_send);
                sem_post(mutex_shm_sensores);

                write_log("WORKER -> PRINTING STATS\n", semaforo_log);

                message_send.id = msg_id;

                if (msgsnd(mq_id, &message_send, sizeof(MQ) - sizeof(long), 0) < 0) {
                    perror("msgsnd");
                    exit(1);
                }

            }
            if(strcmp(token, "reset") == 0){
                if((token = strtok(NULL, "#")) == NULL){
                    return 0;
                }
                msg_id = atoi(token);

                sem_wait(mutex_shm_sensores);
                reset(registos, infos, &message_send);
                sem_post(mutex_shm_sensores);

                write_log("WORKER -> RESETING\n", semaforo_log);

                message_send.id = msg_id;

                if (msgsnd(mq_id, &message_send, sizeof(MQ) - sizeof(long), 0) < 0) {
                    perror("msgsnd");
                    exit(1);
                }

            }
            if(strcmp(token, "sensors") == 0){
                if((token = strtok(NULL, "#")) == NULL){
                    return 0;
                }
                msg_id = atoi(token);
                
                sem_wait(mutex_shm_sensores);
                print_sensors(registos, infos, &message_send);
                sem_post(mutex_shm_sensores);  

                write_log("WORKER -> PRINTING SENSORS\n", semaforo_log);

                message_send.id = msg_id;
                
                if (msgsnd(mq_id, &message_send, sizeof(MQ) - sizeof(long), 0) < 0) {
                    perror("msgsnd");
                    exit(1);
                }

            }
            if(strcmp(token, "list_alerts") == 0){
                if((token = strtok(NULL, "#")) == NULL){
                    return 0;
                }
                msg_id = atoi(token);
                
                sem_wait(mutex_shm_alertas);
                list_alerts(alertas, infos, &message_send);
                sem_post(mutex_shm_alertas);

                write_log("WORKER -> PRINTING ALERTS\n", semaforo_log);

                message_send.id = msg_id;
                
                if (msgsnd(mq_id, &message_send, sizeof(MQ) - sizeof(long), 0) < 0) {
                    perror("msgsnd");
                    exit(1);
                }

            }
            if(strcmp(token, "add_alert") == 0){
                int flag;

                if((token = strtok(NULL, "#")) == NULL){
                    return 0;
                }
                strcpy(alerta_add->id, token); 

                if((token = strtok(NULL, "#")) == NULL){
                    return 0;
                }
                strcpy(alerta_add->nome, token);

                if((token = strtok(NULL, "#")) == NULL){
                    return 0;
                }
                alerta_add->min = atoi(token);

                if((token = strtok(NULL, "#")) == NULL){
                    return 0;
                }
                alerta_add->max = atoi(token);  

                if((token = strtok(NULL, "#")) == NULL){
                    return 0;
                }
                alerta_add->user_console = atoi(token);

                sem_wait(mutex_shm_alertas);
                flag = add_alert(alertas, infos, alerta_add);
                sem_post(mutex_shm_alertas);

                if(flag == -1){
                    //ja existe
                    strcpy(message_send.infos, "ERROR\n");
                    write_log("WORKER -> ADD_ALERT ERROR -> ALREADY EXISTS\n", semaforo_log);
                }

                if(flag == 0){
                    //cheio
                    strcpy(message_send.infos, "ERROR\n");
                    write_log("WORKER -> ADD_ALERT ERROR -> FULL\n", semaforo_log);
                }

                if(flag == 1){
                    //sucesso
                    strcpy(message_send.infos, "OK\n");
                    write_log("WORKER -> ADD_ALERT OK -> SUCESS\n", semaforo_log);
                }

                message_send.id = alerta_add->user_console;
                
                if (msgsnd(mq_id, &message_send, sizeof(MQ) - sizeof(long), 0) < 0) {
                    perror("msgsnd");
                    exit(1);
                }

            }
            if(strcmp(token, "remove_alert") == 0){

                if((token = strtok(NULL, "#")) == NULL){
                    return 0;
                }
                strcpy(id_remove, token); 

                if((token = strtok(NULL, "#")) == NULL){
                    return 0;
                }
                msg_id = atoi(token);

                sem_wait(mutex_shm_alertas);

                if(remove_alert(alertas, infos, id_remove)){
                    strcpy(message_send.infos, "OK\n");
                    write_log("WORKER -> REMOVE_ALERT OK -> REMOVED\n", semaforo_log);
                }else{
                    strcpy(message_send.infos, "ERROR\n");
                    write_log("WORKER -> REMOVE_ALERT OK -> DONT EXIST\n", semaforo_log);
                }

                sem_post(mutex_shm_alertas);

                message_send.id = msg_id;
                if (msgsnd(mq_id, &message_send, sizeof(MQ) - sizeof(long), 0) < 0) {
                    perror("msgsnd");
                    exit(1);
                }
            }
        }
    return 0;
}