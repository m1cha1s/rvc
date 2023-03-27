#ifndef RVC_H
#define RVC_H

#include <stdint.h>
#include <stdarg.h>

static const char *RVABI[32] = {
    "zero",
    "ra",
    "sp",
    "gp",
    "tp",
    "t0",
    "t1",
    "t2",
    "s0",
    "s1",
    "a0",
    "a1",
    "a2",
    "a3",
    "a4",
    "a5",
    "a6",
    "a7",
    "s2",
    "s3",
    "s4",
    "s5",
    "s6",
    "s7",
    "s8",
    "s9",
    "s10",
    "s11",
    "t3",
    "t4",
    "t5",
    "t6",
};

typedef void (*RvcLogPrint)(char *);

typedef uint8_t (*RvcMemBusLoad)(void *meta, uint64_t addr);
typedef void (*RvcMemBusStore)(void *meta, uint64_t addr, uint8_t val);

typedef struct RvcMemBus
{
    uint64_t base, len;
    void *meta;
    RvcMemBusLoad load;
    RvcMemBusStore store;
} RvcMemBus;

typedef struct RvcState
{
    uint64_t x[32];
    uint64_t pc;
    RvcMemBus *bus;  // NULL terminated
    RvcLogPrint log; // NULL for disabled
    int log_level;
} RvcState;

static void RvcLog(RvcState *state, int min_log_level, const char *fmt, ...)
{
    if (!state->log || state->log_level < min_log_level)
        return;

    char str[256] = {0};

    va_list ap;
    va_start(ap, fmt);

    vsprintf(str, fmt, ap);

    va_end(ap);

    state->log(str);
}

static void RvcLogRegs(RvcState *state, int min_log_level)
{
    if (!state->log || state->log_level < min_log_level)
        return;

    RvcLog(state, min_log_level, "Registers:\n");

    for (int i = 0; i < 32; i += 2)
    {
        RvcLog(state, min_log_level, " | %s: %#016x | %s: %#016x |\n", RVABI[i], state->x[i], RVABI[i + 1], state->x[i + 1]);
    }
}

static uint64_t RvcLoad(RvcState *state, uint64_t addr, uint8_t size)
{
    RvcMemBus *bus = state->bus;

    while (bus)
    {
        if (!((addr >= bus->base && addr < (bus->base + bus->len)) || !bus->load))
        {
            bus++;
            continue;
        }
        // Please don't look at the nightmare below :(
        switch (size)
        {
        case 8:
            return (uint64_t)bus->load(bus->meta, addr);
        case 16:
            return (uint64_t)((uint64_t)bus->load(bus->meta, (addr)) | ((uint64_t)bus->load(bus->meta, (addr + 1)) << 8));
        case 32:
            return (uint64_t)((uint64_t)bus->load(bus->meta, (addr)) | ((uint64_t)bus->load(bus->meta, (addr + 1)) << 8) | ((uint64_t)bus->load(bus->meta, (addr + 2)) << 16) | ((uint64_t)bus->load(bus->meta, (addr + 3)) << 24));
        case 64:
            return (uint64_t)((uint64_t)bus->load(bus->meta, (addr)) | ((uint64_t)bus->load(bus->meta, (addr + 1)) << 8) | ((uint64_t)bus->load(bus->meta, (addr + 2)) << 16) | ((uint64_t)bus->load(bus->meta, (addr + 3)) << 24) | ((uint64_t)bus->load(bus->meta, (addr + 4)) << 32) | ((uint64_t)bus->load(bus->meta, (addr + 5)) << 40) | ((uint64_t)bus->load(bus->meta, (addr + 6)) << 48) | ((uint64_t)bus->load(bus->meta, (addr + 7)) << 56));
        default:
            // FIXME: This should cause a trap
            return 0;
        }
    }
    // FIXME: If we get here we trap
    return 0;
}

static void RvcStore(RvcState *state, uint64_t addr, uint64_t val, uint8_t size)
{
    uint8_t b0, b1, b2, b3, b4, b5, b6, b7;

    b0 = val & 0xff;
    b1 = (val >> 8) & 0xff;
    b2 = (val >> 16) & 0xff;
    b3 = (val >> 24) & 0xff;
    b4 = (val >> 32) & 0xff;
    b5 = (val >> 40) & 0xff;
    b6 = (val >> 48) & 0xff;
    b7 = (val >> 56) & 0xff;

    RvcMemBus *bus = state->bus;
    while (bus)
    {
        if (!((addr >= bus->base && addr < (bus->base + bus->len)) || !bus->store))
        {
            bus++;
            continue;
        }
        switch (size)
        {
        case 8:
            bus->store(bus->meta, addr, b0);
            break;
        case 16:
            bus->store(bus->meta, addr, b0);
            bus->store(bus->meta, addr + 1, b1);
            break;
        case 32:
            bus->store(bus->meta, addr, b0);
            bus->store(bus->meta, addr + 1, b1);
            bus->store(bus->meta, addr + 2, b2);
            bus->store(bus->meta, addr + 3, b3);
            break;
        case 64:
            bus->store(bus->meta, addr, b0);
            bus->store(bus->meta, addr + 1, b1);
            bus->store(bus->meta, addr + 2, b2);
            bus->store(bus->meta, addr + 3, b3);
            bus->store(bus->meta, addr + 4, b4);
            bus->store(bus->meta, addr + 5, b5);
            bus->store(bus->meta, addr + 6, b6);
            bus->store(bus->meta, addr + 7, b7);
            break;
        default:
            // FIXME: This should cause a trap
            break;
        }
    }
    // FIXME: If we get here we trap
}

typedef enum RvcStatus
{
    Ok,
    UnknownInstruction,
} RvcStatus;

RvcStatus RvcStep(RvcState *state, uint32_t elapsed_us)
{
    // Fetch
    uint32_t inst = RvcLoad(state, state->pc, 32);

    // Zero the zero register
    state->x[0] = 0;

    // Split the instruction
    uint8_t opcode = inst & 0x7f;
    uint8_t rd = (inst >> 7) & 0x1f;
    uint8_t rs1 = (inst >> 15) & 0x1f;
    uint8_t rs2 = (inst >> 20) & 0x1f;
    uint8_t func4 = (inst >> 12) & 0x7;
    uint8_t func7 = (inst >> 25) & 0x7f;

    RvcLog(state, 2, "Opcode: %#02x\n", opcode);

    switch (opcode)
    {
    // addi
    case 0x13:
    {
        uint64_t imm = (uint64_t)((int64_t)(inst & 0xfff00000) >> 20);
        state->x[rd] = state->x[rs1] + imm;

        RvcLog(state, 1, "addi %s, %s, %d\n", RVABI[rd], RVABI[rs1], imm);

        break;
    }
    // add
    case 0x33:
    {
        state->x[rd] = state->x[rs1] + state->x[rs2];

        RvcLog(state, 1, "add %s, %s, %s\n", RVABI[rd], RVABI[rs1], RVABI[rs2]);

        break;
    }
    default:

        RvcLog(state, 1, "UNKNOWN OPCODE\n");

        return UnknownInstruction;
    }

    RvcLogRegs(state, 3);

    return Ok;
}

#endif
