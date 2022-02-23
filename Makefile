CFLAGS = -Wall
LDFLAGS = -lm
DEBUG_FLAGS = -g
COMPILER = -std=gnu99

all: ppos_core_debug

# P2
ppos_core_debug: 
	gcc -Wall -o teste_ppos_core -DDEBUG ppos_core.c testeP2_1.c

# P1
exemplo_context: contexts.c
	gcc contexts.c -o contexts $(CFLAGS)

# P0
testafila: testafila.o queue.o
	gcc -o testafila testafila.o queue.o $(CFLAGS) $(LDFLAGS) $(COMPILER) $(DEBUG_FLAGS)

testafila.o: testafila.c
	gcc -c testafila.c -o testafila.o $(CFLAGS)

queue.o: queue.c
	gcc -c queue.c -o queue.o $(CFLAGS)

# utils
clean:
	-rm -f *~ *.o
	
purge: clean
	-rm -f testafila