#detecta o compilador disponivel (pgcc/nvc ou gcc/cc). pode sobrescrever com make CC=...
CC ?= $(firstword $(shell command -v pgcc || command -v nvc || command -v gcc || command -v cc))

#pgcc/nvc usam -mp, gcc/cc usam -fopenmp
ifneq (,$(or $(findstring pgcc,$(CC)),$(findstring nvc,$(CC))))
OMPFLAG = -mp
else
OMPFLAG = -fopenmp
endif

OFLAGS=-O2
FLAGS=$(OFLAGS) $(OMPFLAG)
LDLIBS=-lm

RM=rm -f

EXEC=polygon_cut
SEQ_EXEC=polygon_cut_seq

all: $(EXEC)

$(EXEC): $(EXEC).c
	$(CC) $(FLAGS) $(EXEC).c -c -o $(EXEC).o
	$(CC) $(FLAGS) $(EXEC).o -o $(EXEC) $(LDLIBS)

#versao sequencial para comparar speedup, mesmo -O2 da versao paralela
seq: $(SEQ_EXEC)

$(SEQ_EXEC): old_$(EXEC).c
	$(CC) $(OFLAGS) old_$(EXEC).c -o $(SEQ_EXEC) $(LDLIBS)

run:
	./$(EXEC)

clean:
	$(RM) $(EXEC).o $(EXEC) $(SEQ_EXEC)
