#ifndef RVC_H
#define RVC_H

#include <stdio.h>
#include <stdint.h>

struct MemBus
{
    uint64_t base, len;
    void *meta;
    uint8_t (*load)(void *meta, uint64_t addr);
    void (*store)(void *meta, uint64_t addr, uint8_t val);
};

struct RvcState
{
    uint64_t x[32];
    uint64_t pc;
    struct MemBus *bus; // NULL terminated
};

uint64_t load(struct RvcState *state, uint64_t addr, uint8_t size)
{
    struct MemBus *bus = state->bus;
    while (bus)
    {
        if (!(addr >= bus->base && addr < (bus->base + bus->len)))
        {
            bus++;
            continue;
        }
        switch (size)
        {
        case 8:
            return (uint64_t)bus->load(bus->meta, addr);
        case 16:
            return (uint64_t)(bus->load(bus->meta, (addr)) || (bus->load(bus->meta, (addr + 1)) << 8));
        case 32:
            return (uint64_t)(bus->load(bus->meta, (addr)) || (bus->load(bus->meta, (addr + 1)) << 8) || (bus->load(bus->meta, (addr + 2)) << 16) || (bus->load(bus->meta, (addr + 3)) << 24));
        case 64:
            return (uint64_t)(bus->load(bus->meta, (addr)) || (bus->load(bus->meta, (addr + 1)) << 8) || (bus->load(bus->meta, (addr + 2)) << 16) || (bus->load(bus->meta, (addr + 3)) << 24) || (bus->load(bus->meta, (addr + 4)) << 32) || (bus->load(bus->meta, (addr + 5)) << 40) || (bus->load(bus->meta, (addr + 6)) << 48) || (bus->load(bus->meta, (addr + 7)) << 56));
        default:
            // FIXME: This should cause a trap
            return 0;
        }
    }
    // FIXME: If we get here we trap
}

void store(struct RvcState *state, uint64_t addr, uint64_t val, uint8_t size)
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

    struct MemBus *bus - state->bus;
    while (bus)
    {
        if (!(addr >= bus->base && addr < (bus->base + bus->len)))
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

int32_t RvcStep(struct RvcState *state, uint32_t elapsed_us)
{
    state->x[0] = 0;

    uint32_t inst = load(state->pc, 32);
    printf("Inst: %#08x\n", inst);
}

#endif