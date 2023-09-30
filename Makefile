
CFLAGS = -Wall -ansi -pedantic
OBJECTS = utils.o pre_assembler.o instruction.o label.o assembler.o main.o

main: $(OBJECTS)
	gcc -g $(CFLAGS) $(OBJECTS) -o $@

utils.o: utils.c utils.h
	gcc -c $(CFLAGS) utils.c -o $@

pre_assembler.o: pre_assembler.c pre_assembler.h utils.h
	gcc -c $(CFLAGS) pre_assembler.c -o $@

instruction.o: instruction.c instruction.h utils.h
	gcc -c $(CFLAGS) instruction.c -o $@

label.o: label.c label.h utils.h
	gcc -c $(CFLAGS) label.c -o $@

assembler.o: assembler.c assembler.h pre_assembler.h utils.h
	gcc -c $(CFLAGS) assembler.c -o $@

main.o: main.c assembler.h
	gcc -c $(CFLAGS) main.c -o $@
