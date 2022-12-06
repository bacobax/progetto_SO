master : master.c utils/vettoriInt.c utils/vettoriString.c utils/sem_utility.c utils/msg_utility.c utils/errorHandler.c
	gcc master.c utils/vettoriInt.c utils/vettoriString.c utils/sem_utility.c utils/msg_utility.c utils/errorHandler.c -o bin/master
