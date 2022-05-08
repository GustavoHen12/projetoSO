CFLAGS = -Wall
LDFLAGS = -lm
DEBUG_FLAGS = -g -DDEBUG
COMPILER = -std=gnu99

all: ppos_core

ppos_core: queue.o
	gcc -Wall -o ppos_core_p$(P) queue.o $(CFLAGS) $(LDFLAGS) ppos_core.c testeP$(P)_$(T).c

ppos_core_debug: queue.o
	gcc -Wall -o ppos_core_p$(P) queue.o $(CFLAGS) $(LDFLAGS) ppos_core.c testeP$(P)_$(T).c $(DEBUG_FLAGS)

ppos_core_prodcons: queue.o
	gcc -Wall -o ppos_core_prodcons queue.o $(CFLAGS) $(LDFLAGS) ppos_core.c pingpong-prodcons.c

########
ppos_core_p2:
	for number in 1 2 3 ; do \
		gcc -Wall -o teste_ppos_core_$$number ppos_core.c testeP2_$$number.c ; \
	done 

# P2
ppos_core_debug_p2: queue.o
	gcc -Wall -o teste_ppos_core queue.o $(CFLAGS) $(LDFLAGS) -DDEBUG ppos_core.c testeP2_1.c $(DEBUG_FLAGS)

# P1
exemplo_context: contexts.c
	gcc contexts.c -o contexts $(CFLAGS)

# P0
testafila: testafila.o queue.o
	gcc -o testafila testafila.o queue.o $(CFLAGS) $(LDFLAGS) $(COMPILER) $(DEBUG_FLAGS)

testafila.o: testafila.c
	gcc -c testafila.c -o testafila.o $(CFLAGS)
###########

queue.o: queue.c
	gcc -c queue.c -o queue.o $(CFLAGS)

# utils
clean:
	-rm -f *~ *.o
	
purge: clean
	-rm -f testafila