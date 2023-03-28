#include <stdio.h>
#include <stdlib.h>

#include "rvc.h"

uint8_t load(void *file, uint64_t addr)
{
    uint8_t data = 0;

    fread(&data, 1, 1, (FILE *)file);

    return data;
}

void stdlog(char *str)
{
    printf("%s", str);
}

int main(int argc, char *argv[])
{

    uint64_t len = 0;
    FILE *file;

    if (argc == 1)
        file = fopen("test_progs/add-addi.bin", "rb");
    else
        file = fopen(argv[1], "rb");

    if (!file)
    {
        printf("Failed to open file!\n");
        exit(1);
    }

    fseek(file, 0L, SEEK_END);
    len = ftell(file);
    rewind(file);

    RvcMemBus prog = {
        .base = 0,
        .len = len,
        .load = load,
        .store = NULL,
        .meta = (void *)file,
    };

    RvcMemBus *bus[] = {
        &prog,
        (RvcMemBus *)NULL,
    };

    RvcState state = (RvcState){
        .bus = bus,
        .pc = 0,
        .log = stdlog,
        .logFlags = {
            .abi = 0,
            .decode = 1,
            .error = 1,
            .regs = 0,
            .verbose = 1,
        },
    };

    while (1)
    {
        if (RvcStep(&state, 0) != Ok)
            break;
    }

    return 0;
}
