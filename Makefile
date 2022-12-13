master : src/master.c utils/vettoriInt.c utils/vettoriString.c utils/sem_utility.c utils/msg_utility.c utils/errorHandler.c utils/shm_utility.c utils/support.c src/dump.c
	gcc -std=c89 -pedantic src/master.c utils/vettoriInt.c utils/vettoriString.c utils/sem_utility.c utils/msg_utility.c utils/errorHandler.c utils/shm_utility.c utils/support.c src/dump.c -o bin/master
test : utils/test.c utils/vettoriInt.c utils/vettoriString.c utils/sem_utility.c utils/msg_utility.c utils/shm_utility.c utils/errorHandler.c utils/support.c utils/loadShip.c src/dump.c
	gcc -std=c89 -pedantic utils/test.c utils/vettoriInt.c utils/vettoriString.c utils/sem_utility.c utils/msg_utility.c utils/errorHandler.c utils/shm_utility.c utils/support.c utils/loadShip.c src/dump.c -o utils/bin/test
porto : src/porto.c utils/vettoriInt.c utils/vettoriString.c utils/sem_utility.c utils/msg_utility.c utils/errorHandler.c utils/shm_utility.c utils/support.c src/dump.c
	gcc -std=c89 -pedantic src/porto.c utils/vettoriInt.c utils/vettoriString.c utils/sem_utility.c utils/msg_utility.c utils/errorHandler.c utils/shm_utility.c utils/support.c src/dump.c -o bin/porto
nave : src/nave.c utils/vettoriInt.c utils/vettoriString.c utils/sem_utility.c utils/msg_utility.c utils/errorHandler.c utils/shm_utility.c utils/support.c utils/loadShip.c utils/ship_utility.c src/dump.c
	gcc -std=c89 -pedantic src/nave.c utils/vettoriInt.c utils/vettoriString.c utils/sem_utility.c utils/msg_utility.c utils/errorHandler.c utils/shm_utility.c utils/support.c utils/loadShip.c utils/ship_utility.c src/dump.c -o bin/nave

