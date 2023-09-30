#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "assembler.h"

int main(int argc, char **argv) {
    machine *machine = initialize_machine();
    int files = 1;
    while(argc > files) {
        reset_machine(machine);
        full_assembler(machine, argv[files]);
	    printf("\n\n");
        files++;
    }
    return 0;
}