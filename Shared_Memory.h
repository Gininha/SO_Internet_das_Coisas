#include <stdio.h>

#define CHAVE_LEN 33

typedef struct{
    char nome[CHAVE_LEN];
    int last_val;
    int min_val;
    int max_val;
    double media;
    int total;
}Registos;

typedef struct{
    int max_keys;
    int keys_atual;
}Infos;

Registos* create_shared_memory(int num_registos);
Infos* create_shared_memory_infos();

void get_rid_shm(Registos *registo);
void get_rid_shm_infos(Infos *infos);

void write_to_shared_memory(Registos *Pointer, Infos *infos, Registos *registo);
void print_shared_memory(Registos *Pointer, Infos *infos);