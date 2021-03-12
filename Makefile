CC=vc +kick13
ASM=vasmm68k_mot -Fhunk -I$(NDK_ASMINC)
CFLAGS=-I$(NDK_INC) -c99 -O3
LDFLAGS=-lamiga -lauto
EXES=example_01 example_02

.PHONY : clean check
.SUFFIXES : .o .c .asm

all: $(EXES)

clean:
	rm -f *.o ptplayer/*.o $(EXES)

.c.o:
	$(CC) $(CFLAGS) $^ -c -o $@

.asm.o:
	$(ASM) $^ -o $@

example_01: example_01.o ptplayer/ptplayer.o
	$(CC) $^ $(LDFLAGS) -o $@

example_02: example_02.o tilesheet.o sprites.o ptplayer/ptplayer.o
	$(CC) $^ $(LDFLAGS) -o $@
