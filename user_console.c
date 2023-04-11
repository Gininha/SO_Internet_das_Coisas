/*
Luis Leite 2021199102
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define DEBUG
#define BUF_SIZE 1024

int main(){
    char opt[BUF_SIZE], id[BUF_SIZE], chave[BUF_SIZE], min[BUF_SIZE], max[BUF_SIZE];
    char *token;

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

        if(strcmp(token, "stats") == 0){

            #ifdef DEBUG
            printf("1\n");
            #endif
        }

        if(strcmp(token, "reset") == 0){

            #ifdef DEBUG
            printf("2\n");
            #endif
        }

        if(strcmp(token, "sensors") == 0){

            #ifdef DEBUG
            printf("3\n");
            #endif
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

        if(strcmp(token, "list_alerts") == 0){
            
            #ifdef DEBUG
            printf("4\n");
            #endif
        }

        putchar('\n');
        putchar('\n');
    }
}