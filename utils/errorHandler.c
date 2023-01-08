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

void initErrorHandler(){
    int semid;
    FILE *fp;
    semid = createSem(ERRFILESEMID, 1, errorHandler, "initErrorHandler");

    fclose(fopen("./logs/errorLog.log", "w"));
}

void removeErrorHandler(){
    
   removeSem(useSem(ERRFILESEMID, errorHandler, "removeErrorHandler"), errorHandler, "removeErrorHandler");
}

void printError(char* myerr, char* errCtx) {
    int semid;
    FILE *fp;
    int hash = IPC_PRIVATE;
    semid = useSem(ERRFILESEMID, errorHandler, "printError");
    mutex(semid, LOCK, errorHandler, "LOCK printError");
    printf("❌❌❌❌❌❌❌❌❌❌❌❌❌ HASH %d\n", hash);
    fp = fopen("./logs/errorLog.log", "a+");
    fprintf(fp, "💥💥💥💥💥💥💥💥\n");
    fprintf(fp, "ERROR: %s error handler\nERRNO: %s\nCTX: %s\nHASH: %d\n", myerr, strerror(errno), errCtx, hash);
    fprintf(fp, "💥💥💥💥💥💥💥💥\n");
    fclose(fp);
    mutex(semid, UNLOCK, errorHandler, "UNLOCK printError");

    
}

void errorHandler(int err, char* errCtx) {
    switch (err) {
    case SERRCTL:
        printError("sem ctl", errCtx);
        break;
    case SERRGET:
        printError("sem get", errCtx);
         
        break;
    case SERROP:
        printError("sem op", errCtx);
        
        break;
    case MERRCTL:
        printError("msg ctl", errCtx);
             
        break;
    case MERRSND:
        printError("msg snd", errCtx);
        
        
        break;
    case MERRRCV:
        printError("msg rcv", errCtx);
          
        break;
    case MERRGET:
        printError("msg get", errCtx);
        
        break;
    case SHMERRGET:
        printError("shm get", errCtx);
        
        break;
    case SHMERRAT:
        printError("shm at", errCtx);
        
        break;
    case SHMERRDT:
        printError("shm dt", errCtx);
        
        break;
    case SHMERRCTL:
        printError("shm ctl", errCtx);
        break;
    default:
        perror("Not Handlerd error");
    }
}
