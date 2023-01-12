#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
int main() {
    for (int i = 0; i < 5; i++) {
        system("ps >> ./pxs");
        sleep(0.2);
    }
}