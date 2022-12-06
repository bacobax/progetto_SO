#include <string.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <err.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h> 
#include "./shm_utility.h"


int createShm(int key, size_t shmSize, void (*errorHandler)(int err)){
    int shmid = shmget(key, shmSize, IPC_CREAT | IPC_EXCL | 0666);
    if(errno == EEXIST) return errno;
    if(shmid == -1){
        if(errorHandler == NULL){
            perror("createShm -> shmget");
            exit(EXIT_FAILURE);
        } else {
            errorHandler(SHMERRGET);
            exit(EXIT_FAILURE);
        }
    }
    return shmid;
}

int useShm(int key, size_t shmSize, void (*errorHandler)(int err)){
    int shmid = shmget(key, shmSize, 0);
    if(shmid == -1){
        if(errorHandler == NULL){
            perror("useShm -> shmget");
            exit(EXIT_FAILURE);
        } else {
            errorHandler(SHMERRGET);
            exit(EXIT_FAILURE);
        } 
    }
    return shmid;
}


void* getShmAddress(int shmid, int flag, void (*errorHandler)(int err)){
    void* addr;
    addr = shmat(shmid, NULL, flag);
    if(addr == (void*) -1){
         if(errorHandler == NULL){
            perror("getShmAddress -> shmat");
            exit(EXIT_FAILURE);
        } else {
            errorHandler(SHMERRAT);
            exit(EXIT_FAILURE);
        }
    }
    return addr;
}

void shmDetach(void* addrToRemove, void (*errorHandler)(int err)){
    if(shmdt(addrToRemove) == -1){
        if(errorHandler == NULL){
            perror("shmDetach -> shmdt");
            exit(EXIT_FAILURE);
        } else {
            errorHandler(SHMERRDT);
            exit(EXIT_FAILURE);
        }
    }
}

int removeShm(int shmid, void (*errorHandler)(int err)){
    if(shmctl(shmid, IPC_RMID, NULL) == -1){
        if(errorHandler == NULL){
            perror("removeShm -> shmctl");
            exit(EXIT_FAILURE);
        } else {
            errorHandler(SHMERRCTL);
            exit(EXIT_FAILURE);
        }
    }
}



