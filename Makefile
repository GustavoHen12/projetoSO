CFLAGS = -Wall
LDFLAGS = -lm
DEBUG_FLAGS = -g
COMPILER = -std=gnu99

all: ppos_core

ppos_core_p4: queue.o
	gcc -Wall -o ppos_core_p4 queue.o $(CFLAGS) $(LDFLAGS) ppos_core.c testeP4_1.c $(DEBUG_FLAGS)

ppos_core_p4_debug: queue.o
	gcc -Wall -o ppos_core_p4 queue.o $(CFLAGS) $(LDFLAGS) -DDEBUG ppos_core.c testeP4_1.c $(DEBUG_FLAGS)

ppos_core_p3_clean: queue.o
	gcc -Wall -o ppos_core_p3 queue.o $(CFLAGS) $(LDFLAGS) ppos_core.c testeP3_1.c $(DEBUG_FLAGS)

ppos_core_p3: queue.o
	gcc -Wall -o ppos_core_p3 queue.o $(CFLAGS) $(LDFLAGS) -DDEBUG ppos_core.c testeP3_1.c $(DEBUG_FLAGS)

ppos_core:
	for number in 1 2 3 ; do \
		gcc -Wall -o teste_ppos_core_$$number ppos_core.c testeP2_$$number.c ; \
	done 

# P2
ppos_core_debug: queue.o
	gcc -Wall -o teste_ppos_core queue.o $(CFLAGS) $(LDFLAGS) -DDEBUG ppos_core.c testeP2_1.c $(DEBUG_FLAGS)

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