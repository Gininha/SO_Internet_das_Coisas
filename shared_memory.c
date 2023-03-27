#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <stdlib.h>

#include "Shared_Memory.h"


int shmid, shmid2;

Registos* create_shared_memory(int num_registos) {

    Registos *pointer_registos;

    // allocate shared memory
    if ((shmid = shmget(IPC_PRIVATE, sizeof(Registos)*num_registos, IPC_CREAT | 0777)) == -1) {
        perror("shmget");
        exit(1);
    }

    // attach shared memory to pointer
    if ((pointer_registos = (Registos*) shmat(shmid, NULL, 0)) == (void*) -1) {
        perror("shmat");
        exit(1);
    }

    return pointer_registos;
}

Infos* create_shared_memory_infos(){

    Infos *pointer_infos;

    // allocate shared memory
    if ((shmid2 = shmget(IPC_PRIVATE, sizeof(Infos), IPC_CREAT | 0777)) == -1) {
        perror("shmget");
        exit(1);
    }

    // attach shared memory to pointer
    if ((pointer_infos = (Infos*) shmat(shmid2, NULL, 0)) == (void*) -1) {
        perror("shmat");
        exit(1);
    }

    return pointer_infos;
}

void get_rid_shm(Registos *registo){
    // Detach shared memory
    if (shmdt(registo) == -1) {
        perror("shmdt");
        exit(1);
    }

    // Destroy shared memory
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("shmctl");
        exit(1);
    }
}

void get_rid_shm_infos(Infos *infos){
    // Detach shared memory
    if (shmdt(infos) == -1) {
        perror("shmdt");
        exit(1);
    }

    // Destroy shared memory
    if (shmctl(shmid2, IPC_RMID, NULL) == -1) {
        perror("shmctl");
        exit(1);
    }
}

void print_shared_memory(Registos *Pointer, Infos *infos){

    int i;

    printf("Quantidade de registos: %d\n", infos->keys_atual);

    for(i=0; i<infos->keys_atual; i++){
        printf("Registo[%d] -> %s\n",i+1, Pointer[i].nome);
    }
}

void write_to_shared_memory(Registos *Pointer, Infos *infos, Registos *registo){

    int i;

    if(infos->keys_atual == infos->max_keys){
        printf("Lotaçao maxima\n");
        return;
    }

    if(infos->keys_atual == 0){
        strcpy(Pointer[0].nome, registo->nome);
        Pointer[0].last_val = registo->last_val;
        Pointer[0].min_val = registo->min_val;
        Pointer[0].max_val = registo->max_val;
        Pointer[0].media = registo->media;
        Pointer[0].total = registo->total;
        infos->keys_atual++;
        return;
    }

    for(i=0; i<infos->keys_atual; i++){

        if(strcmp(Pointer[i].nome, registo->nome) == 0){

            Pointer[i].last_val = registo->last_val;
            Pointer[i].min_val = registo->min_val;
            Pointer[i].max_val = registo->max_val;
            Pointer[i].media = registo->media;
            Pointer[i].total++;
            return;
        }
    }

    strcpy(Pointer[i].nome, registo->nome);
    Pointer[i].last_val = registo->last_val;
    Pointer[i].min_val = registo->min_val;
    Pointer[i].max_val = registo->max_val;
    Pointer[i].media = registo->media;
    Pointer[i].total = registo->total;
    infos->keys_atual++;
    return;
}