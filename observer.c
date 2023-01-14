#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
int main(int argc, char const* argv[]) {
    for (int i = 0; i < atoi(argv[1]); i++) {
        system("ps aux>> ./pxs");

        sleep(0.2);

    }
}