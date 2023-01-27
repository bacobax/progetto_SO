#include "msg_utility.h"
#include "sem_utility.h"
#include "shm_utility.h"
#include <sys/ipc.h>
#include "./errorHandler.h"
#include "../config1.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <stdlib.h>

void initErrorHandler(){
    int semid;
    FILE *fp;
    semid = createSem(ERRFILESEMID, 1, errorHandler, "initErrorHandler");

    fclose(fopen("./logs/errorLog.log", "w"));
}

void removeErrorHandler(){
    
   removeSem(useSem(ERRFILESEMID, errorHandler, "removeErrorHandler"), errorHandler, "removeErrorHandler");
}

void throwError(char* myerr, char* errCtx) {
    int semid;
    FILE *fp;
    int hash = rand();
    semid = useSem(ERRFILESEMID, errorHandler, "throwError");
    mutex(semid, LOCK, errorHandler, "LOCK throwError");
    printf("❌❌❌❌❌❌❌❌❌❌❌❌❌ HASH %d\n", hash);
    fp = fopen("./logs/errorLog.log", "a+");
    if (fp == NULL) {
        perror("fopen throwError");
        exit(1);
    }
    fprintf(fp, "💥💥💥💥💥💥💥💥\n");
    fprintf(fp, "ERROR: %s error handler\nERRNO: %s\nCTX: %s\nHASH: %d\nPID: %d\n", myerr, strerror(errno), errCtx, hash,getpid());
    fprintf(fp, "💥💥💥💥💥💥💥💥\n");
    fclose(fp);
    mutex(semid, UNLOCK, errorHandler, "UNLOCK throwError");

    
}

void errorHandler(int err, char* errCtx) {
    switch (err) {
    case SERRCTL:
        throwError("sem ctl", errCtx);
        break;
    case SERRGET:
        throwError("sem get", errCtx);
         
        break;
    case SERROP:
        throwError("sem op", errCtx);
        
        break;
    case MERRCTL:
        throwError("msg ctl", errCtx);
             
        break;
    case MERRSND:
        throwError("msg snd", errCtx);
        
        
        break;
    case MERRRCV:
        throwError("msg rcv", errCtx);
          
        break;
    case MERRGET:
        throwError("msg get", errCtx);
        
        break;
    case SHMERRGET:
        throwError("shm get", errCtx);
        
        break;
    case SHMERRAT:
        throwError("shm at", errCtx);
        
        break;
    case SHMERRDT:
        throwError("shm dt", errCtx);
        
        break;
    case SHMERRCTL:
        throwError("shm ctl", errCtx);
        break;
    default:
        throwError("Not Handlerd error", errCtx);
    }
}
