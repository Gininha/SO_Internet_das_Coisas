/*
Luis Leite 2021199102
*/

#include "Shared_Memory.h"

Sem_Log *semaforo_log;
sem_t *sem_pipe;
char id[33];
int fd;
int contagem;

void cleanup(int signum){
    char buffer[150];

    if(signum == SIGTSTP){
        printf("Enviadas %d leituras\n", contagem);
        sprintf(buffer, "CTRL Z -> Enviadas %d mensagens por sensor %s\n", contagem, id);
        write_log(buffer, semaforo_log);
        return;
    }

    close(fd);

    printf("Enviadas %d leituras\n", contagem);

    if(signum == SIGINT)
        sprintf(buffer, "Sensor %s terminado (CTRL C) -> Enviadas %d leituras\n", id, contagem);
    if(signum == SIGPIPE)
        sprintf(buffer, "Sensor %s terminado (Pipe closed) -> Enviadas %d leituras\n", id, contagem);

    write_log(buffer, semaforo_log);

    exit(0);
}

int main(int argc, char *argv[]){

    int random;
    char log_message[250];
    contagem = 0;

    signal(SIGINT, cleanup);
    signal(SIGPIPE, cleanup);
    signal(SIGTSTP, cleanup);

    if (argc != 6){
        printf("$ sensor {identificador do sensor} {intervalo entre envios (s)} {chave} {min val} {max val}\n");
        return 0;
    }

    if ( strlen(argv[1]) < 3 || strlen(argv[1]) > 32 ){
        printf("3 < {identificador do sensor} < 32\n");
        return 0;
    }
    strcpy(id, argv[1]);

    if ( strlen(argv[3]) < 3 || strlen(argv[3]) > 32 ){
        printf("3 < {chave} < 32\n");
        return 0;
    }

    int intervalo = atoi(argv[2]);
    int min_val = atoi(argv[4]);
    int max_val = atoi(argv[5]);
    
    semaforo_log = open_shared_memory_log();
    sem_pipe = open_shared_memory_Sensor_Pipe();
    
    //Abertura do Named Pipe
    if ((fd = open(SENSOR_PIPE_NAME, O_WRONLY)) < 0) {
        perror("Cannot open pipe for reading: ");
        exit(0);
    }

    while(1){
        random = rand() % (max_val - min_val + 1) + min_val;

        printf("Sensor %s#%s#%d\n", argv[1], argv[3], random);
        sprintf(log_message, "%s#%s#%d\n", argv[1], argv[3], random);

        int tamanho = strlen(log_message);
        log_message[tamanho-1] = '\0'; 
        write(fd, log_message, sizeof(log_message));

        sprintf(log_message, "Sensor -> %s#%s#%d\n", argv[1], argv[3], random);
        write_log(log_message, semaforo_log);
        
        contagem++;
        
        sleep(intervalo);
    }

    return 0;
} 