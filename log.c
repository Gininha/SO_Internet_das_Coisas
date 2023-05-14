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


Configuracoes* leitura_ficheiro(char *nome) {
    FILE *file = fopen(nome, "r");
    if (file == NULL) {
        printf("Ficheiro nao encontrado\n");
        return NULL;
    }

    Configuracoes *configs = malloc(sizeof(Configuracoes));
    char line[100];  // Assuming a maximum line length of 100 characters

    int lineCount = 0;
    while (fgets(line, sizeof(line), file)) {
        lineCount++;

        int value;
        if (sscanf(line, "%d", &value) != 1) {
            printf("Erro ao ler ficheiro. Valor invalido encontrado na linha %d\n", lineCount);
            fclose(file);
            free(configs);
            return NULL;
        }

        switch (lineCount) {
            case 1:
                if(value < 1){
                    printf("Queue Size >= 1\n");
                    fclose(file);
                    free(configs);
                    return NULL;
                }
                configs->QUEUE_SZ = value;
                break;
            case 2:
                if(value < 1){
                    printf("N_workers >= 1\n");
                    fclose(file);
                    free(configs);
                    return NULL;
                }
                configs->N_WORKERS = value;
                break;
            case 3:
                if(value < 1){
                    printf("Max Keys >= 1\n");
                    fclose(file);
                    free(configs);
                    return NULL;
                }
                configs->MAX_KEYS = value;
                break;
            case 4:
                if(value < 1){
                    printf("Max Sensores >= 1\n");
                    fclose(file);
                    free(configs);
                    return NULL;
                }
                configs->MAX_SENSORS = value;
                break;
            case 5:
                if(value < 0){
                    printf("Max Alerts >= 0\n");
                    fclose(file);
                    free(configs);
                    return NULL;
                }
                configs->MAX_ALERTS = value;
                break;
            default:

                break;
        }
    }

    fclose(file);
    return configs;
}
