#include "msg_utility.h"
#include "sem_utility.h"
#include "shm_utility.h"
#include "../config1.h"
#include <stdio.h>

void errorHandler(int err) {
    switch (err) {
    case SERRCTL:
        perror("sem ctl error handler");
        break;
    case SERRGET:
        perror("sem get error handler");
        break;
    case SERROP:
        perror("sem op error handler");
        break;
    case MERRCTL:
        perror("msg ctl error handler");
        break;
    case MERRSND:
        perror("msg snd error handler");
        break;
    case MERRRCV:
        perror("msg recv error handler");
        break;
    case MERRGET:
        perror("msg get error handler");
        break;
    case SHMERRGET:
        perror("shm get error handler");
        break;
    case SHMERRAT:
        perror("shm at error handler");
        break;
    case SHMERRDT:
        perror("shm dt error handler");
        break;           
    default:
        perror("Not Handlerd error");
    }
}