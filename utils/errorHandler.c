#include "msg_utility.h"
#include "sem_utility.h"
#include "shm_utility.h"
#include "../config1.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

void printError(char* myerr, char* errCtx) {
    printf("ERROR: %s error handler\nERRNO: %s\nCTX: %s\n", myerr, strerror(errno), errCtx);
    printf("💥💥💥💥💥💥💥💥\n");
    
}

void errorHandler(int err, char* errCtx) {
    printf("💥💥💥💥💥💥💥💥\n");
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
