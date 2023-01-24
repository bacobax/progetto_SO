#include "./errorHandler.h"
#include "./msg_utility.h"
#include <string.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <err.h>
#include <errno.h>
#include <time.h>

void msgSend(int msgqID, char text[MEXBSIZE], long type, void (*errorHandler)(int err, char* errCtx),int ipcNoWait ,char* errCtx) {
    mex m;
    int flag;
    m.mtype = type;
    strcpy(m.mtext, text);

    if (ipcNoWait) {
        flag = IPC_NOWAIT;
    }
    else {
        flag = 0;
    }

    if (msgsnd(msgqID, &m, MEXBSIZE, flag) == -1) {
        if (errorHandler != NULL) {
            errorHandler(MERRSND,errCtx);
            exit(EXIT_FAILURE);
        }
        else {
            throwError("msgSend -> msgsnd", "msgSend");
            exit(EXIT_FAILURE);
        }
    }

}

mex* msgRecv(int msgqID, long type, void (*errorHandler)(int err, char* errCtx), void (*callback)(long type, char text[MEXBSIZE]), int mod, char* errCtx) {
    mex* m = (mex*)malloc(sizeof(mex));
    if (msgrcv(msgqID, m, MEXBSIZE, type, 0) == -1) {
        
        if (errorHandler != NULL) {
            errorHandler(MERRRCV,errCtx);
            exit(EXIT_FAILURE);
        }
        else {
            throwError("msgRecv -> msgrcv", "msgRecv");
            exit(EXIT_FAILURE);
        }
    }

    if (mod == SYNC) {
        return m;
    }
    else if (mod == ASYNC) {
        int pid = fork();

        if (pid == -1) {
            if (errorHandler == NULL) {
                throwError("msgRecv -> fork" , "msgRecv");
                exit(EXIT_FAILURE);
            }
            else {
                errorHandler(errno, errCtx);
                exit(EXIT_FAILURE);
            }

        }
        if (pid == 0) {
            
            callback(m->mtype, m->mtext);
            free(m);
            exit(EXIT_SUCCESS);
        }
        else {
            free(m);
        }
        return NULL;
    }
    else {
        fprintf(stderr, "mod must be SYNC or ASYNC\n");
        exit(EXIT_FAILURE);
    }

}
mex* msgRecvPro(int msgqID, long type, void (*errorHandler)(int err, char* errCtx), void (*callback)(long type, char text[MEXBSIZE], int arg), int mod, int arg, char* errCtx) {
    mex* m = (mex*)malloc(sizeof(mex));

    if (msgrcv(msgqID, m, MEXBSIZE, type, 0) == -1) {
        if (errorHandler != NULL) {
            errorHandler(MERRRCV, errCtx);
            exit(EXIT_FAILURE);
        }
        else {
            throwError("msgRecv -> msgrcv", "msgRecvPro");
            exit(EXIT_FAILURE);
        }
    }

    if (mod == SYNC) {
        return m;
    }
    else if (mod == ASYNC) {
        int pid = fork();

        if (pid == -1) {
            if (errorHandler == NULL) {
                throwError("msgRecv -> fork", "msgRecvPro");
                exit(EXIT_FAILURE);
            }
            else {
                errorHandler(errno, errCtx);
                exit(EXIT_FAILURE);
            }

        }
        if (pid == 0) {
            
            callback(m->mtype, m->mtext, arg);
            free(m);
            exit(EXIT_SUCCESS);
        }
        else {
            free(m);
        }
        return NULL;
    }
    else {
        fprintf(stderr, "mod must be SYNC or ASYNC\n");
        exit(EXIT_FAILURE);
    }

}

int createQueue(int key, void (*errorHandler)(int err, char* errCtx), char* errCtx) {
    int qid = msgget(key, IPC_CREAT | IPC_EXCL | 0666);
    if (errno == EEXIST) {
        return EEXIST;
    }
    if (qid == -1) {
        if (errorHandler != NULL) {
            errorHandler(MERRGET, errCtx);
        }
        else {
            throwError("createQueue -> msgget", "createQueue");
            exit(EXIT_FAILURE);
        }
    }
    return qid;
}

int useQueue(int key, void (*errorHandler)(int err, char* errCtx), char* errCtx) {
    int qid = msgget(key, 0);
    if (qid == -1) {
        if (errorHandler != NULL) {
            errorHandler(MERRGET, errCtx);
        }
        else {
            throwError("useQueue -> msgget", "useQueue");
            exit(EXIT_FAILURE);
        }
    }
    return qid;
}

int getMexCount(int id, void (*errorHandler)(int err, char* errCtx), char* errCtx) {
    struct msqid_ds buf;

    if (msgctl(id, IPC_STAT, &buf) == -1) {
        if(errorHandler == NULL){
            throwError("getMexCount -> msgctl", "getMexCount");
            exit(EXIT_FAILURE);
        } else {
            errorHandler(MERRCTL, errCtx);
            exit(EXIT_FAILURE);
        }
    }
    return buf.msg_qnum;
}

void printQueueState(int id, void (*errorHandler)(int err, char* errCtx), char* errCtx) {
    struct msqid_ds buf;

    if (msgctl(id, IPC_STAT, &buf) == -1) {
        if(errorHandler == NULL){
            throwError("printQueueState -> msgctl", "printQueueState");
            exit(EXIT_FAILURE);
        } else {
            errorHandler(MERRCTL, errCtx);
            exit(EXIT_FAILURE);
        }
    }
    printf("CODA %d: {\n\tDIM: %lu bytes\n\tN_MEX: %lu\n\tTIMESTAMP msgsnd: %s\n}\n", id, buf.msg_qbytes, buf.msg_qnum, ctime(&buf.msg_stime));

}

void removeQueue(int id, void (*errorHandler)(int err, char* errCtx), char* errCtx) {
    if (msgctl(id, IPC_RMID, NULL) == -1) {
        if(errorHandler == NULL){
            throwError("removeQueue -> msgctl", "removeQueue");
            exit(EXIT_FAILURE);
        } else {
            errorHandler(MERRCTL, errCtx);
            exit(EXIT_FAILURE);
        }
    }
}
