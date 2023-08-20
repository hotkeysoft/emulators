#Assembler using main symbol
AS=vasmm68k_mot #Assembly compiler

SOURCES=$(wildcard *.asm)
OBJECTS=$(SOURCES:.asm=.o)

.PHONY: all
all: $(OBJECTS)

$(OBJECTS): %.o: %.asm
	$(AS) -nowarn=2069 -no-opt $< -o $@ -Lbpl=12 -L $@.lst
 
.PHONY: clean
clean:
	rm -f *.o *.lst *.tst *~ *.bak