#ifndef RVC_H
#define RVC_H

#include <stdint.h>
#include <stdarg.h>

static const char *RVABI[32][2] = {
    {"x0", "zero"},
    {"x1", "ra"},
    {"x2", "sp"},
    {"x3", "gp"},
    {"x4", "tp"},
    {"x5", "t0"},
    {"x6", "t1"},
    {"x7", "t2"},
    {"x8", "s0"},
    {"x9", "s1"},
    {"x10", "a0"},
    {"x11", "a1"},
    {"x12", "a2"},
    {"x13", "a3"},
    {"x14", "a4"},
    {"x15", "a5"},
    {"x16", "a6"},
    {"x17", "a7"},
    {"x18", "s2"},
    {"x19", "s3"},
    {"x20", "s4"},
    {"x21", "s5"},
    {"x22", "s6"},
    {"x23", "s7"},
    {"x24", "s8"},
    {"x25", "s9"},
    {"x26", "s10"},
    {"x27", "s11"},
    {"x28", "t3"},
    {"x29", "t4"},
    {"x30", "t5"},
    {"x31", "t6"},
};

typedef void (*RvcLogPrint)(char *);

typedef uint8_t (*RvcMemBusLoad)(void *meta, uint64_t addr);
typedef void (*RvcMemBusStore)(void *meta, uint64_t addr, uint8_t val);

typedef struct _RvcMemBus
{
    uint64_t base, len;
    void *meta;
    RvcMemBusLoad load;
    RvcMemBusStore store;
} RvcMemBus;

typedef struct _RvcLogFlags
{
    uint8_t warning : 1;
    uint8_t error : 1;
    uint8_t decode : 1;
    uint8_t transaction : 1;
    uint8_t regs : 1;
    uint8_t abi : 1;
    uint8_t verbose : 1;
} RvcLogFlags;

#define LOG_WARNING (state->logFlags.warning)
#define LOG_ERROR (state->logFlags.error)
#define LOG_DECODE (state->logFlags.decode)
#define LOG_TRANSACTION (state->logFlags.transaction)
#define LOG_REGS (state->logFlags.regs)
#define LOG_ABI (state->logFlags.abi)
#define LOG_VERBOSE (state->logFlags.verbose)

typedef struct _RvcState
{
    uint64_t x[32];
    uint64_t pc;

    RvcMemBus *bus; // NULL terminated

    RvcLogPrint log; // NULL for disabled
    RvcLogFlags logFlags;
} RvcState;

typedef enum _RvcStatus
{
    Ok,
    UnknownInstruction,
} RvcStatus;

static void RvcLog(RvcState *state, uint8_t flags, const char *fmt, ...)
{
    if (!state->log || !flags)
        return;

    char str[256] = {0};

    va_list ap;
    va_start(ap, fmt);

    vsprintf(str, fmt, ap);

    va_end(ap);

    state->log(str);
}

static void RvcLogRegs(RvcState *state, uint8_t flags)
{
    if (!state->log || !flags)
        return;

    RvcLog(state, flags, "Registers:\n");

    for (int i = 0; i < 32; i += 2)
    {
        RvcLog(state, flags, " | %s: %#016x | %s: %#016x |\n", RVABI[i][LOG_ABI], state->x[i], RVABI[i + 1][LOG_ABI], state->x[i + 1]);
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

    RvcLog(state, LOG_DECODE && LOG_VERBOSE, "Opcode: %#02x\n", opcode);

    switch (opcode)
    {
    // lui
    case 0x36:
    {

        break;
    }
    // addi
    case 0x13:
    {
        uint64_t imm = (uint64_t)((int64_t)(inst & 0xfff00000) >> 20);
        state->x[rd] = state->x[rs1] + imm;

        RvcLog(state, LOG_DECODE, "addi %s, %s, %d\n", RVABI[rd][LOG_ABI], RVABI[rs1][LOG_ABI], imm);

        break;
    }
    // add
    case 0x33:
    {
        state->x[rd] = state->x[rs1] + state->x[rs2];

        RvcLog(state, LOG_DECODE, "add %s, %s, %s\n", RVABI[rd][LOG_ABI], RVABI[rs1][LOG_ABI], RVABI[rs2][LOG_ABI]);

        break;
    }
    default:

        RvcLog(state, LOG_ERROR, "UNKNOWN OPCODE\n");
        RvcLogRegs(state, LOG_ERROR && LOG_VERBOSE);

        return UnknownInstruction;
    }

    RvcLogRegs(state, LOG_REGS);

    return Ok;
}

#undef LOG_WARNING
#undef LOG_ERROR
#undef LOG_DECODE
#undef LOG_TRANSACTION
#undef LOG_REGS
#undef LOG_ABI
#undef LOG_VERBOSE

#endif
