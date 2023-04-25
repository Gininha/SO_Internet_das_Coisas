/*
Luis Leite 2021199102
*/

#include "Shared_Memory.h"

#define DEBUG
#define BUF_SIZE 1024

#define stats "stats"
#define reset "reset"
#define sensors "sensors"
#define list_alerts "list_alerts"

Sem_Log *semaforo_log;
sem_t *sem_pipe;
int fd;

int main(){
    char opt[BUF_SIZE], id[BUF_SIZE], chave[BUF_SIZE], min[BUF_SIZE], max[BUF_SIZE];
    char *token;
    char buffer[250];

    semaforo_log = open_shared_memory_log();
    sem_pipe = open_shared_memory_Console_Pipe();

    //Abertura do Named Pipe
    if ((fd = open(CONSOLE_PIPE_NAME, O_WRONLY)) < 0) {
        perror("Cannot open pipe for reading: ");
        exit(0);
    }

    while(1){
        printf("exit\n");
        printf("stats\n");
        printf("reset\n");
        printf("sensors\n");
        printf("add_alert [id] [chave] [min] [max]\n");
        printf("remove_alert [id]\n");
        printf("list_alerts\n");

        scanf(" %99[^\n]", opt);

        token = strtok(opt, " ");

        if(strcmp(token, "exit") == 0){

            exit(0);
        }

        if(strcmp(token, stats) == 0){

            write(fd, stats, strlen(stats));
        }

        if(strcmp(token, reset) == 0){

            write(fd, reset, strlen(reset));
        }

        if(strcmp(token, sensors) == 0){

            write(fd, sensors, strlen(sensors));
        }

        if(strcmp(token, "add_alert") == 0){

            token = strtok(NULL, " ");
            strcpy(id, token);
            token = strtok(NULL, " ");
            strcpy(chave, token);
            token = strtok(NULL, " ");
            strcpy(min, token);
            token = strtok(NULL, " ");
            strcpy(max, token);

            #ifdef DEBUG
            printf("Id: %s  Chave: %s   Min: %s    Max: %s\n", id, chave, min, max);
            #endif
        }

        if(strcmp(token, "remove_alert") == 0){

            token = strtok(NULL, " ");
            strcpy(id, token);

            #ifdef DEBUG
            printf("Id: %s\n", id);
            #endif
        }

        if(strcmp(token, list_alerts) == 0){
            
            write(fd, list_alerts, strlen(list_alerts));
        }

        putchar('\n');
        putchar('\n');
    }
}