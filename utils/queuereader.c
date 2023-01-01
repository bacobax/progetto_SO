#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../config1.h"
#include "./msg_utility.h"

int main(int argc, char const* argv[]) {
    int queueID;
    int type;
    mex* res;
    queueID = atoi(argv[1]);
    type = atoi(argv[2]);

    // res = msgRecv(queueID, type, errorHandler, NULL, SYNC, "msg recv in queuereader");
    // printf("%s" , res->mtext);
    fprintf(stdout, "CIAO\n");

    return 0;
}
