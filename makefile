test: getandput.o queue.o
	gcc getandput.o queue.o -pthread -o test main.c -Wall -Werror
getandput: getandput.c
	gcc -c getandput.c
queue: queue.c
	gcc -c queue.c
clean: 
	rm  *.o test