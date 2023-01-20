
failL : all
	make all
	ipcrm -a
	bin/master
failM : all
	make all
	./removeIPCS
	bin/master

success : all
	make all
	make start

start : all 
	bin/master

all : master nave porto meteo
	make master
	make nave
	make porto
	make meteo


master : src/master.c src/dump.c utils/vettoriInt.c utils/vettoriString.c utils/sem_utility.c utils/msg_utility.c utils/errorHandler.c utils/shm_utility.c utils/support.c utils/master_utility.c utils/supplies.c utils/ship_utility.c utils/port_utility.c utils/loadShip.c
	gcc -std=c89 -pedantic src/master.c src/dump.c utils/vettoriInt.c utils/vettoriString.c utils/sem_utility.c utils/msg_utility.c utils/errorHandler.c utils/shm_utility.c utils/support.c utils/master_utility.c utils/supplies.c utils/ship_utility.c utils/port_utility.c utils/loadShip.c -lm -D_XOPEN_SOURCE=700 -o bin/master
test : utils/test.c  src/dump.c utils/vettoriInt.c utils/vettoriString.c utils/sem_utility.c utils/msg_utility.c utils/shm_utility.c utils/errorHandler.c utils/support.c utils/loadShip.c utils/port_utility.c utils/ship_utility.c utils/supplies.c utils/master_utility.c
	gcc -std=c89 -pedantic utils/test.c src/dump.c utils/vettoriInt.c utils/vettoriString.c utils/sem_utility.c utils/msg_utility.c utils/errorHandler.c utils/shm_utility.c utils/support.c utils/loadShip.c utils/port_utility.c utils/ship_utility.c utils/supplies.c utils/master_utility.c -lm -D_XOPEN_SOURCE=700 -o utils/bin/test
porto : src/porto.c src/dump.c utils/vettoriInt.c utils/vettoriString.c utils/sem_utility.c utils/msg_utility.c utils/errorHandler.c utils/shm_utility.c utils/support.c utils/supplies.c utils/port_utility.c utils/ship_utility.c utils/loadShip.c
	gcc -std=c89 -pedantic src/porto.c src/dump.c utils/vettoriInt.c utils/vettoriString.c utils/sem_utility.c utils/msg_utility.c utils/errorHandler.c utils/shm_utility.c utils/support.c utils/supplies.c utils/port_utility.c utils/ship_utility.c utils/loadShip.c -lm -D_XOPEN_SOURCE=700 -o bin/porto
nave : src/nave.c src/dump.c utils/vettoriInt.c utils/vettoriString.c utils/sem_utility.c utils/msg_utility.c utils/errorHandler.c utils/shm_utility.c utils/support.c  utils/ship_utility.c utils/port_utility.c utils/supplies.c utils/loadShip.c
	gcc -std=c89 -pedantic src/nave.c src/dump.c utils/vettoriInt.c utils/vettoriString.c utils/sem_utility.c utils/msg_utility.c utils/errorHandler.c utils/shm_utility.c utils/support.c utils/ship_utility.c utils/port_utility.c utils/supplies.c utils/loadShip.c -lm -D_XOPEN_SOURCE=700 -o bin/nave 

meteo : src/meteo.c src/dump.c utils/vettoriInt.c utils/vettoriString.c utils/sem_utility.c utils/msg_utility.c utils/errorHandler.c utils/shm_utility.c utils/support.c utils/master_utility.c utils/supplies.c utils/ship_utility.c utils/port_utility.c utils/loadShip.c
	gcc -std=c89 -pedantic src/meteo.c src/dump.c utils/vettoriInt.c utils/vettoriString.c utils/sem_utility.c utils/msg_utility.c utils/errorHandler.c utils/shm_utility.c utils/support.c utils/master_utility.c utils/supplies.c utils/ship_utility.c utils/port_utility.c utils/loadShip.c -lm -D_XOPEN_SOURCE=700 -o bin/meteo
