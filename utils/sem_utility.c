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

#if defined(__linux__)

union semun {
    int val;
    struct semid_ds* buf;
    unsigned short* array;
    struct seminfo* __buf;

};
#endif 

int useSem(int key, void (*errorHandler)(int err)) {
    int semid;
    semid = semget(key, 1, 0);
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
    int semid;
    union semun arg;

    semid = semget(key, 1, IPC_CREAT | IPC_EXCL | 0666);
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

int createMultipleSem(int key, int nSem, int initValue, void (*errorHandler)(int err)) {
    int semid;
    union semun arg;
    int i;
    semid = semget(key, nSem, IPC_CREAT | IPC_EXCL | 0666);
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


    arg.val = initValue;

    for (i = 0; i < nSem; i++) {
        if (semctl(semid, i, SETVAL, arg) == -1) {
            if (errorHandler == NULL) {
                perror("useSem -> semctl");
                exit(EXIT_FAILURE);
            }
            else {
                errorHandler(SERRCTL);
                exit(EXIT_FAILURE);
            }
        }
    }


    return semid;
}



void removeSem(int key, void (*errorHandler)(int err)) {
    if (semctl(key, 0, IPC_RMID, NULL) == -1) {


        if (errorHandler == NULL) {
            perror("removeSem->semctl");
            exit(EXIT_FAILURE);
        }
        else {
            errorHandler(SERRCTL);
            exit(EXIT_FAILURE);
        }
    }
}

void mutex(int semid, int op, void (*errorHandler)(int err)) {
    struct sembuf* buf;
    buf = (struct sembuf*)malloc(sizeof(struct sembuf));
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

void mutexPro(int semid, int semIdx, int op, void (*errorHandler)(int err)) {
    struct sembuf* buf;
    buf = (struct sembuf*)malloc(sizeof(struct sembuf));
    buf->sem_num = semIdx;
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

