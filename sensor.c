#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "Shared_Memory.h"

int main(int argc, char *argv[]){

    int random;

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

    int forever = atoi(argv[2]);
    int min_val = atoi(argv[4]);
    int max_val = atoi(argv[5]);
    

    while(1){
        random = rand() % (max_val - min_val + 1) + min_val;

        printf("%s#%s#%d\n",argv[1], argv[3], random);

        sleep(forever);
    }

    return 0;
} 