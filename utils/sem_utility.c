#include <string.h>
#include <sys/sem.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <err.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h> 
#include "./errorHandler.h"
#include "./sem_utility.h"
/*
#if defined(__linux__)

*/

union semun {
    int val;
    struct semid_ds* buf;
    unsigned short* array;
    struct seminfo* __buf;

};
/*
#endif 

*/

int useSem(int key, void (*errorHandler)(int err, char* errCtx), char* errCtx) {
    int semid;
    semid = semget(key, 1, 0);
    if (semid == -1) {
        if (errorHandler == NULL) {
            throwError("useSem -> semget" , "useSem");
            exit(EXIT_FAILURE);
        }
        else {
            errorHandler(SERRGET, errCtx);
            exit(EXIT_FAILURE);
        }

    }
    return semid;


}

int createSem(int key, int initValue, void (*errorHandler)(int err, char* errCtx), char* errCtx) {
    int semid;
    union semun arg;

    semid = semget(key, 1, IPC_CREAT | IPC_EXCL | 0666);
    if (errno == EEXIST) return errno;
    if (semid == -1) {
        if (errorHandler == NULL) {
            throwError("useSem -> semget", "createSem");
            exit(EXIT_FAILURE);
        }
        else {
            errorHandler(SERRGET, errCtx);
            exit(EXIT_FAILURE);
        }

    }
    arg.val = initValue;
    if (semctl(semid, 0, SETVAL, arg) == -1) {
        if (errorHandler == NULL) {
            throwError("useSem -> semctl", "createSem");
            exit(EXIT_FAILURE);
        }
        else {
            errorHandler(SERRCTL, errCtx);
            exit(EXIT_FAILURE);
        }
    }

    return semid;
}

int createMultipleSem(int key, int nSem, int initValue, void (*errorHandler)(int err, char* errCtx), char* errCtx) {
    int semid;
    union semun arg;
    int i;
    semid = semget(key, nSem, IPC_CREAT | IPC_EXCL | 0666);
    if (errno == EEXIST) return errno;
    if (semid == -1) {
        if (errorHandler == NULL) {
            throwError("useSem -> semget", "createMultipleSem");
            exit(EXIT_FAILURE);
        }
        else {
            errorHandler(SERRGET, errCtx);
            exit(EXIT_FAILURE);
        }

    }


    arg.val = initValue;

    for (i = 0; i < nSem; i++) {
       if (semctl(semid, i, SETVAL, arg) == -1) {
            if (errorHandler == NULL) {
                throwError("useSem -> semctl", "createMultipleSem");
                exit(EXIT_FAILURE);
            }
            else {
                errorHandler(SERRCTL, errCtx);
                exit(EXIT_FAILURE);
            }
        }
    }


    return semid;
}



void removeSem(int semid, void (*errorHandler)(int err, char* errCtx), char* errCtx) {
    if (semctl(semid, 0, IPC_RMID, NULL) == -1) {


        if (errorHandler == NULL) {
            throwError("removeSem->semctl", "removeSem");
            exit(EXIT_FAILURE);
        }
        else {
            errorHandler(SERRCTL, errCtx);
            exit(EXIT_FAILURE);
        }
    }
}

void mutex(int semid, int op, void (*errorHandler)(int err, char* errCtx), char* errCtx) {
    struct sembuf* buf;
    buf = (struct sembuf*)malloc(sizeof(struct sembuf));
    buf->sem_num = 0;
    buf->sem_flg = 0;
    buf->sem_op = op;
    if (semop(semid, buf, 1) == -1) {
        free(buf);
        if (errorHandler == NULL) {
            throwError("mutex->semop", "mutex");
            exit(EXIT_FAILURE);

        }
        else {
            errorHandler(SERROP, errCtx);
            exit(EXIT_FAILURE);
        }
    }
    free(buf);
}

void mutexPro(int semid, int semIdx, int op, void (*errorHandler)(int err, char* errCtx), char* errCtx) {
    struct sembuf* buf;
    buf = (struct sembuf*)malloc(sizeof(struct sembuf));
    buf->sem_num = semIdx;
    buf->sem_flg = 0;
    buf->sem_op = op;
    if (semop(semid, buf, 1) == -1) {
        free(buf);
        if (errorHandler == NULL) {
            throwError("mutex->semop", "mutexPro");
            exit(EXIT_FAILURE);

        }
        else {
            errorHandler(SERROP, errCtx);
            exit(EXIT_FAILURE);
        }
    }
    free(buf);
}

int getWaitingPxCount(int semid, int idx) {
    return semctl(semid, idx, GETNCNT);
}
int getWaitingZeroPxCount(int semid, int idx) {
    return semctl(semid, idx, GETZCNT);
}

void getAllVAlues(int semid, int length){
    union semun arg;
    int* v = calloc(length, sizeof(int));
    int* v2= calloc(length, sizeof(int));
    int i;
    arg.array = calloc(length, sizeof(unsigned short));
    
    for (i = 0; i < length; i++) {

        arg.array[i] = -2;
    }
    if(semctl(semid, 0,GETALL, arg) == -1){
        throwError("SEMCTL", "getAllValues");
    }
    for (i = 0; i < length; i++) {
        v[i] = getWaitingZeroPxCount(semid, i);
    }
    for (i = 0; i < length; i++) {
        v2[i] = getWaitingPxCount(semid, i);
    }
    for (i = 0; i < length; i++) {
        printf("------------\n");
        printf("VAL SEM IDX %d: %d\n", i, arg.array[i]);
        printf("WAITING PX IDX %d: %d\n" , i, v2[i]);
        printf("WAITING ZERO PX IDX %d: %d\n" , i, v[i]);
        printf("------------\n");

    }
    free(v);
    free(v2);
}

int getOneValue(int semid, int idx){
    int val;
    if((val = semctl(semid, idx,GETVAL)) == -1){
        throwError("SEMCTL", "getOneValue");
    }
    return val;
}
