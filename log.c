#include "log.h"

void write_to_log_procs(FILE *Pointer, char *string){
    
    if(Pointer == NULL){
        printf("Erro\n");
        return;
    }

    time_t now = time(NULL);           // get the current time
    struct tm *local_time = localtime(&now);  // convert it to local time
    char time_str[9];
    strftime(time_str, sizeof(time_str), "%T", local_time);  // format time as HH:MM:SS string

    fprintf(Pointer, "%s %s", time_str, string);

    return;

}


void write_to_log(FILE *Pointer, char *string){
    
    time_t now = time(NULL);           // get the current time
    struct tm *local_time = localtime(&now);  // convert it to local time
    char time_str[9];
    strftime(time_str, sizeof(time_str), "%T", local_time);  // format time as HH:MM:SS string

    Pointer = fopen("log.txt", "a");
    fprintf(Pointer, "%s %s", time_str, string);
    fclose(Pointer);

    return;

}

Configuracoes* leitura_ficheiro(char *nome){

    FILE * file = fopen(nome, "r");
    if(file == NULL) printf("Ficheiro nao encontrado\n");

    int a,b,c,d,e;
    Configuracoes *configs = malloc(sizeof(Configuracoes));

    if (fscanf(file, "%d %d %d %d %d", &a, &b, &c, &d, &e) != 5) {
        printf("Erro ao ler ficheiro\n");
    }

    configs->QUEUE_SZ = a;
    configs->N_WORKERS = b;
    configs->MAX_KEYS = c;
    configs->MAX_SENSORS = d;
    configs->MAX_ALERTS = e;

    fclose(file);

    return configs;
}