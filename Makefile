master : src/master.c utils/vettoriInt.c utils/vettoriString.c utils/sem_utility.c utils/msg_utility.c utils/errorHandler.c utils/shm_utility.c utils/support.c
	gcc src/master.c utils/vettoriInt.c utils/vettoriString.c utils/sem_utility.c utils/msg_utility.c utils/errorHandler.c utils/shm_utility.c utils/support.c -o bin/master
test : utils/test.c utils/vettoriInt.c utils/vettoriString.c utils/sem_utility.c utils/msg_utility.c utils/shm_utility.c utils/errorHandler.c utils/support.c
	gcc utils/test.c utils/vettoriInt.c utils/vettoriString.c utils/sem_utility.c utils/msg_utility.c utils/errorHandler.c utils/shm_utility.c utils/support.c -o utils/bin/test
porto : porto.c utils/vettoriInt.c utils/vettoriString.c utils/sem_utility.c utils/msg_utility.c utils/errorHandler.c utils/shm_utility.c utils/support.c
	gcc porto.c utils/vettoriInt.c utils/vettoriString.c utils/sem_utility.c utils/msg_utility.c utils/errorHandler.c utils/shm_utility.c utils/support.c -o bin/porto
nave: 	nave.c utils/vettoriInt.c utils/vettoriString.c utils/sem_utility.c utils/msg_utility.c utils/errorHandler.c utils/shm_utility.c utils/support.c
	gcc nave.c utils/vettoriInt.c utils/vettoriString.c utils/sem_utility.c utils/msg_utility.c utils/errorHandler.c utils/shm_utility.c utils/support.c -o bin/nave

