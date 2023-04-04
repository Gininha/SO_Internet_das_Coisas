#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <semaphore.h>

#define NUM_THREADS 3
#define NUM_PROCESSES 6

sem_t mutex;
char filename[] = "output.txt";

void write_to_log(int fd, char *string) {
    time_t now = time(NULL);
    struct tm *local_time = localtime(&now);
    char time_str[9];
    strftime(time_str, sizeof(time_str), "%T", local_time);
    dprintf(fd, "%s %s", time_str, string);
}

void *thread_worker(void *arg) {
    int thread_id = *(int *) arg;
    char log_message[100];
    sprintf(log_message, "Thread %d created!!!\n", thread_id);
    sem_wait(&mutex);
    int fd = open(filename, O_WRONLY | O_CREAT | O_APPEND, 0644);
    write_to_log(fd, log_message);
    close(fd);
    sem_post(&mutex);
    sprintf(log_message, "Thread %d leaving!!!\n", thread_id);
    sem_wait(&mutex);
    fd = open(filename, O_WRONLY | O_CREAT | O_APPEND, 0644);
    write_to_log(fd, log_message);
    close(fd);
    sem_post(&mutex);
    return NULL;
}

void process_worker(int process_id) {
    char log_message[100];
    sprintf(log_message, "Process %d created!!!\n", process_id);
    sem_wait(&mutex);
    int fd = open(filename, O_WRONLY | O_CREAT | O_APPEND, 0644);
    write_to_log(fd, log_message);
    close(fd);
    sem_post(&mutex);
    sprintf(log_message, "Process %d leaving!!!\n", process_id);
    sem_wait(&mutex);
    fd = open(filename, O_WRONLY | O_CREAT | O_APPEND, 0644);
    write_to_log(fd, log_message);
    close(fd);
    sem_post(&mutex);
    exit(0);
}

int main() {
    int i;
    int thread_ids[NUM_THREADS];
    pthread_t threads[NUM_THREADS];
    sem_init(&mutex, 0, 1);
    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    close(fd);
    for (i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i + 1;
        pthread_create(&threads[i], NULL, thread_worker, &thread_ids[i]);
    }
    for (i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    for (i = 0; i < NUM_PROCESSES; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            exit(1);
        } else if (pid == 0) {
            /* child process */
            process_worker(i + 1);
        }
    }
    for (i = 0; i < NUM_PROCESSES; i++) {
        wait(NULL);
    }
    sem_destroy(&mutex);
    return 0;
}
