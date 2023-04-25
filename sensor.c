/*
Luis Leite 2021199102
*/

#include "Shared_Memory.h"

Sem_Log *semaforo_log;
sem_t *sem_pipe;
int fd;

void cleanup(int signum){
    close(fd);
    exit(0);
}

int main(int argc, char *argv[]){

    int random;
    char log_message[250];

    signal(SIGINT, cleanup);

    if (argc != 6){
        printf("$ sensor {identificador do sensor} {intervalo entre envios (s)} {chave} {min val} {max val}\n");
        return 0;
    }

    if ( strlen(argv[1]) < 3 || strlen(argv[1]) > 32 ){
        printf("3 < {identificador do sensor} < 32\n");
        return 0;
    }

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

        printf("%s#%s#%d\n", argv[1], argv[3], random);
        sprintf(log_message, "%s#%s#%d\n", argv[1], argv[3], random);
        write_log(log_message, semaforo_log);

        int tamanho = strlen(log_message);
        log_message[tamanho-1] = '\0'; 
        write(fd, log_message, sizeof(log_message));

        sleep(intervalo);
    }

    return 0;
} 