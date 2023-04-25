#include "Shared_Memory.h"

void process_task(char *buffer, Registos* registos, Infos* infos){
    char *token;
    Sensor_thread *sensor = malloc(sizeof(Sensor_thread));

    if((token = strtok(buffer, "#")) == NULL){
            return;
        }
        
        if(strcmp(token, "s") == 0){
            if((token = strtok(NULL, "#")) == NULL){
            return;
            }

            strcpy(sensor->id_sensor, token);

            if((token = strtok(NULL, "#")) == NULL){
                return;
            }

            strcpy(sensor->chave, token);

            if((token = strtok(NULL, "#")) == NULL){
                return;
            }

            sensor->value = atoi(token);

            write_to_shared_memory(registos, infos, sensor);
    
        }
}