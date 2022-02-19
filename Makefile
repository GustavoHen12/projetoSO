CFLAGS = -Wall
LDFLAGS = -lm
DEBUG_FLAGS = -g
COMPILER = -std=gnu99

all: testafila

exemplo_context: contexts.c
	gcc contexts.c -o contexts $(CFLAGS)

testafila: testafila.o queue.o
	gcc -o testafila testafila.o queue.o $(CFLAGS) $(LDFLAGS) $(COMPILER) $(DEBUG_FLAGS)

testafila.o: testafila.c
	gcc -c testafila.c -o testafila.o $(CFLAGS)

queue.o: queue.c
	gcc -c queue.c -o queue.o $(CFLAGS)

clean:
	-rm -f *~ *.o
	
purge: clean
	-rm -f testafila