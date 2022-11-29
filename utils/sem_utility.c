#include <string.h>
#include <sys/sem.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <err.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h> 
#include "./sem_utility.h"


int useSem(int key, void (*errorHandler)(int err)) {
    int semid = semget(key, 1, 0);
    if (semid == -1) {
        if (errorHandler == NULL) {
            perror("useSem -> semget");
            exit(EXIT_FAILURE);
        }
        else {
            errorHandler(SERRGET);
            exit(EXIT_FAILURE);
        }

    }
    return semid;


}

int createSem(int key, int initValue, void (*errorHandler)(int err)) {
    int semid = semget(key, 1, IPC_CREAT | IPC_EXCL | 0666);
    if (errno == EEXIST) return errno;
    if (semid == -1) {
        if (errorHandler == NULL) {
            perror("useSem -> semget");
            exit(EXIT_FAILURE);
        }
        else {
            errorHandler(SERRGET);
            exit(EXIT_FAILURE);
        }

    }
    union semun arg;
    arg.val = initValue;
    if (semctl(semid, 0, SETVAL, arg) == -1) {
        if (errorHandler == NULL) {
            perror("useSem -> semctl");
            exit(EXIT_FAILURE);
        }
        else {
            errorHandler(SERRCTL);
            exit(EXIT_FAILURE);
        }
    }

    return semid;
}


void mutex(int semid, int op, void (*errorHandler)(int err)) {
    struct sembuf* buf = (struct sembuf*)malloc(sizeof(struct sembuf));
    buf->sem_num = 0;
    buf->sem_flg = 0;
    buf->sem_op = op;
    if (semop(semid, buf, 1) == -1) {
        free(buf);
        if (errorHandler == NULL) {
            perror("mutex->semop");
            exit(EXIT_FAILURE);

        }
        else {
            errorHandler(SERROP);
            exit(EXIT_FAILURE);
        }
    }
    free(buf);
}
