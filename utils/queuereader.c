#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../config1.h"
#include "./msg_utility.h"

int main(int argc, char* argv[]) {
    
    int queueID;
    int type;
    mex* res;
    FILE* fp;
    fp = fopen("./utils/bin/logQueuereader.log" , "a+");

    queueID = atoi(argv[1]);
    type = atoi(argv[2]);

    fprintf(fp, "QUEUEID: %d, TYPE: %d\n", queueID, type);
    
    
    
    fputs(res->mtext, stdout);
    exit(EXIT_SUCCESS);
}
