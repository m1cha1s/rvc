#include <stdio.h>
#include <stdlib.h>

// #define LOG_LEVEL 1

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

    RvcMemBus bus[] = {
        {.base = 0, .len = len, .load = load, .meta = (void *)file, .store = NULL},
        (RvcMemBus){NULL},
    };

    RvcState state = (RvcState){
        .bus = bus,
        .pc = 0,
        .log = stdlog,
        .log_level = 1,
    };

    for (int i = 0; i < 3; i++)
    {
        RvcStep(&state, 0);
    }

    return 0;
}
