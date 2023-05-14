/*
Luis Leite 2021199102
*/

#include "Shared_Memory.h"

#define DEBUG
#define BUF_SIZE 50

#define stats "stats"
#define reset "reset"
#define sensors "sensors"
#define list_alerts "list_alerts"
#define add_alert "add_alert"
#define remove_alert "remove_alert"

Sem_Log *semaforo_log;
sem_t *sem_pipe;
int fd;
pid_t pid;

void cleanup(int signum){
    kill(pid, SIGTERM);
    waitpid(pid, NULL, 0); // Wait for child process to exit
    close(fd);
    exit(0);
}

int main(int argc, char *argv[]){
    char opt[BUF_SIZE], id[BUF_SIZE], chave[BUF_SIZE], min[BUF_SIZE], max[BUF_SIZE];
    char *token;
    char buffer[250];
    int my_id;
    int mq_id;
    MQ message_queue;

    if(argc != 2){
        printf("./user_console <id>\n");
        return 0;
    }

    signal(SIGINT, cleanup);

    my_id = atoi(argv[1]);

    semaforo_log = open_shared_memory_log();
    sem_pipe = open_shared_memory_Console_Pipe();

    //Abertura do Named Pipe
    if ((fd = open(CONSOLE_PIPE_NAME, O_WRONLY)) < 0) {
        perror("Cannot open pipe for reading: ");
        exit(0);
    }

    //Abertura da Message Queue
    if ((mq_id = msgget(MQ_ID, 0777)) < 0) {
        perror("msgget");
        exit(1);
    }

    if((pid = fork()) == 0){
        char buffer_2[600];

        while(1){

            if (msgrcv(mq_id, &message_queue, sizeof(MQ) - sizeof(long), my_id, 0) < 0) {
                perror("msgrcv");
                exit(1);
            }

            printf("%s", message_queue.infos);
            sprintf(buffer_2, "User_console %d recebeu:\n%s", my_id, message_queue.infos);
            write_log(buffer_2, semaforo_log);
        }
    }

    while(1){
        strcpy(buffer, "");
        printf("-----------------------\n");
        printf("->exit\n");
        printf("->stats\n");
        printf("->reset\n");
        printf("->sensors\n");
        printf("->add_alert [id] [chave] [min] [max]\n");
        printf("->remove_alert [id]\n");
        printf("->list_alerts\n");
        printf("-----------------------\n");

        scanf(" %99[^\n]", opt);

        token = strtok(opt, " ");

        if(strcmp(token, "exit") == 0){

            cleanup(SIGINT);
        }

        if(strcmp(token, stats) == 0){

            sprintf(buffer, "%s#%d", stats, my_id);
            write(fd, buffer, strlen(buffer)+1);

        }

        if(strcmp(token, reset) == 0){

            sprintf(buffer, "%s#%d", reset, my_id);
            write(fd, buffer, strlen(buffer)+1);

        }

        if(strcmp(token, sensors) == 0){

            sprintf(buffer, "%s#%d", sensors, my_id);
            write(fd, buffer, strlen(buffer)+1);

        }

        if(strcmp(token, add_alert) == 0){

            token = strtok(NULL, " ");
            strcpy(id, token);
            token = strtok(NULL, " ");
            strcpy(chave, token);
            token = strtok(NULL, " ");
            strcpy(min, token);
            token = strtok(NULL, " ");
            strcpy(max, token);

            sprintf(buffer, "%s#%s#%s#%s#%s#%d", add_alert, id, chave, min, max, my_id);
            write(fd, buffer, strlen(buffer)+1);

        }

        if(strcmp(token, remove_alert) == 0){

            token = strtok(NULL, " ");
            strcpy(id, token);

            sprintf(buffer, "%s#%s#%d", remove_alert, id, my_id);
            write(fd, buffer, strlen(buffer)+1);

        }

        if(strcmp(token, list_alerts) == 0){

            sprintf(buffer, "%s#%d", list_alerts, my_id);
            write(fd, buffer, strlen(buffer)+1);

        }

    }
}