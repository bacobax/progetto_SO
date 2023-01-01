#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../config1.h"
#include "./msg_utility.h"

int main(int argc, char* argv[]) {
    /*
    int queueID;
    int type;
    mex* res;
    queueID = atoi(argv[1]);
    type = atoi(argv[2]);
    
    printf("[%d]queue_read queueID: %d type_msg:%d\n", queueID, type);
    */
    /* res = msgRecv(queueID, type, errorHandler, NULL, SYNC, "msg recv in queuereader"); */
    /* printf("%s" , res->mtext); */
    
    /*fprintf(stdout, "CIAO\n");*/
    /*printf("a");*/
    
    /*
    char* s = "ciaoo!";
    write(STDOUT_FILENO, s, strlen(s));*/
    /*
    char c = 'a';
    fputc(c, stdout);
    */
    char text[1024];
    sprintf(text, "CIAO SONO IL PIPE\n");
    fputs(text, stdout);
    exit(EXIT_SUCCESS);
}
