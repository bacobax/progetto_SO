all : master nave porto queuereader
	make master
	make nave
	make porto
	make queuereader


master : src/master.c src/dump.c utils/vettoriInt.c utils/vettoriString.c utils/sem_utility.c utils/msg_utility.c utils/errorHandler.c utils/shm_utility.c utils/support.c utils/master_utility.c utils/supplies.c utils/ship_utility.c
	gcc -std=c89 -pedantic src/master.c src/dump.c utils/vettoriInt.c utils/vettoriString.c utils/sem_utility.c utils/msg_utility.c utils/errorHandler.c utils/shm_utility.c utils/support.c utils/master_utility.c utils/supplies.c utils/ship_utility.c -lm -o bin/master
test : utils/test.c src/dump.c utils/vettoriInt.c utils/vettoriString.c utils/sem_utility.c utils/msg_utility.c utils/shm_utility.c utils/errorHandler.c utils/support.c  utils/ship_utility.c
	gcc -std=c89 -pedantic utils/test.c utils/vettoriInt.c utils/vettoriString.c utils/sem_utility.c utils/msg_utility.c utils/errorHandler.c utils/shm_utility.c utils/support.c utils/ship_utility.c src/dump.c -lm -o utils/bin/test
porto : src/porto.c src/dump.c utils/vettoriInt.c utils/vettoriString.c utils/sem_utility.c utils/msg_utility.c utils/errorHandler.c utils/shm_utility.c utils/support.c utils/supplies.c utils/port_utility.c
	gcc -std=c89 -pedantic src/porto.c src/dump.c utils/vettoriInt.c utils/vettoriString.c utils/sem_utility.c utils/msg_utility.c utils/errorHandler.c utils/shm_utility.c utils/support.c utils/supplies.c utils/port_utility.c -o bin/porto
nave : src/nave.c src/dump.c utils/vettoriInt.c utils/vettoriString.c utils/sem_utility.c utils/msg_utility.c utils/errorHandler.c utils/shm_utility.c utils/support.c  utils/ship_utility.c
	gcc -std=c89 -pedantic src/nave.c src/dump.c utils/vettoriInt.c utils/vettoriString.c utils/sem_utility.c utils/msg_utility.c utils/errorHandler.c utils/shm_utility.c utils/support.c utils/ship_utility.c -lm -o bin/nave 
queuereader : utils/queuereader.c utils/msg_utility.c utils/errorHandler.c
	gcc -std=c89 -pedantic utils/queuereader.c utils/errorHandler.c utils/msg_utility.c -o utils/bin/queuereader

masterL : src/master.c src/dump.c utils/vettoriInt.c utils/vettoriString.c utils/sem_utility.c utils/msg_utility.c utils/errorHandler.c utils/shm_utility.c utils/support.c utils/master_utility.c utils/supplies.c utils/ship_utility.c
	gcc -std=c89 -pedantic src/master.c src/dump.c utils/vettoriInt.c utils/vettoriString.c utils/sem_utility.c utils/msg_utility.c utils/errorHandler.c utils/shm_utility.c utils/support.c utils/master_utility.c utils/supplies.c utils/ship_utility.c -o binLinux/master
testL : utils/test.c src/dump.c utils/vettoriInt.c utils/vettoriString.c utils/sem_utility.c utils/msg_utility.c utils/shm_utility.c utils/errorHandler.c utils/support.c  utils/ship_utility.c
	gcc -std=c89 -pedantic utils/test.c utils/vettoriInt.c utils/vettoriString.c utils/sem_utility.c utils/msg_utility.c utils/errorHandler.c utils/shm_utility.c utils/support.c utils/ship_utility.c src/dump.c -o binLinux/test
portoL : src/porto.c src/dump.c utils/vettoriInt.c utils/vettoriString.c utils/sem_utility.c utils/msg_utility.c utils/errorHandler.c utils/shm_utility.c utils/support.c utils/supplies.c utils/port_utility.c
	gcc -std=c89 -pedantic src/porto.c src/dump.c utils/vettoriInt.c utils/vettoriString.c utils/sem_utility.c utils/msg_utility.c utils/errorHandler.c utils/shm_utility.c utils/support.c utils/supplies.c utils/port_utility.c -o binLinux/porto
naveL : src/nave.c src/dump.c utils/vettoriInt.c utils/vettoriString.c utils/sem_utility.c utils/msg_utility.c utils/errorHandler.c utils/shm_utility.c utils/support.c  utils/ship_utility.c
	gcc -std=c89 -pedantic src/nave.c src/dump.c utils/vettoriInt.c utils/vettoriString.c utils/sem_utility.c utils/msg_utility.c utils/errorHandler.c utils/shm_utility.c utils/support.c utils/ship_utility.c -o binLinux/nave

