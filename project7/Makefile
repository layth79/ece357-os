asmcat: asmcat.o
	ld -m elf_i386 asmcat.o -e start -o asmcat

asmcat.o: asmcat.S
	as --32 asmcat.S -o asmcat.o

clean:
	rm -f asmcat *.o *.stackdump *~
