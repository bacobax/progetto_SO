#include "./msg_utility.h"
#include <string.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <err.h>
#include <errno.h>
#include <time.h>

void msgSend(int msgqID, char text[MEXBSIZE], long type, void (*errorHandler)(int err)) {
    mex m;
    m.mtype = type;
    strcpy(m.mtext, text);

    if (msgsnd(msgqID, &m, MEXBSIZE, 0) == -1) {
        if (errorHandler != NULL) {
            errorHandler(MERRSND);
            exit(EXIT_FAILURE);
        }
        else {
            perror("msgSend -> msgsnd");
            exit(EXIT_FAILURE);
        }
    }

}

mex* msgRecv(int msgqID, long type, void (*errorHandler)(int err), void (*callback)(long type, char text[MEXBSIZE]), int mod) {
    mex* m = (mex*)malloc(sizeof(mex));

    if (msgrcv(msgqID, m, MEXBSIZE, type, 0) == -1) {
        if (errorHandler != NULL) {
            errorHandler(MERRRCV);
            exit(EXIT_FAILURE);
        }
        else {
            perror("msgRecv -> msgrcv");
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
                perror("msgRecv -> fork");
                exit(EXIT_FAILURE);
            }
            else {
                errorHandler(errno);
                exit(EXIT_FAILURE);
            }

        }
        if (pid == 0) {
            callback(m->mtype, m->mtext);
            free(m);
            exit(EXIT_SUCCESS);
        }
        return NULL;
    }
    else {
        fprintf(stderr, "mod must be SYNC or ASYNC\n");
        exit(EXIT_FAILURE);
    }

}

int createQueue(int key, void (*errorHandler)(int err)) {
    int qid = msgget(key, IPC_CREAT | IPC_EXCL | 0666);
    if (errno == EEXIST) {
        return EEXIST;
    }
    if (qid == -1) {
        if (errorHandler != NULL) {
            errorHandler(MERRGET);
        }
        else {
            perror("createQueue -> msgget");
            exit(EXIT_FAILURE);
        }
    }
    return qid;
}

int useQueue(int key, void (*errorHandler)(int err)) {
    int qid = msgget(key, 0);
    if (qid == -1) {
        if (errorHandler != NULL) {
            errorHandler(MERRGET);
        }
        else {
            perror("useQueue -> msgget");
            exit(EXIT_FAILURE);
        }
    }
    return qid;
}

int getMexCount(int id) {
    struct msqid_ds buf;

    if (msgctl(id, IPC_STAT, &buf) == -1) {
        perror("getMexCount -> msgctl");
        exit(1);
    }
    return buf.msg_qnum;
}

void printQueueState(int id) {
    struct msqid_ds buf;

    if (msgctl(id, IPC_STAT, &buf) == -1) {
        perror("printQueueState -> msgctl");
        exit(1);
    }
    printf("CODA %d: {\n\tDIM: %lu bytes\n\tN_MEX: %lu\n\tTIMESTAMP msgsnd: %s\n}\n", id, buf.msg_qbytes, buf.msg_qnum, ctime(&buf.msg_stime));

}

void removeQueue(int id) {
    if (msgctl(id, IPC_RMID, NULL) == -1) {
        perror("removeQueue -> msgctl");
        exit(1);
    }
}
