#ifndef _PROBE_H
#define _PROBE_H

#include <stdint.h>

#define OP_CLOCKS	0x00000000
#define OP_EXECUTE	0x01000000
#define OP_HAS_VALUE 	0x10000000
#define OP_MEM_READ	0x12000000
#define OP_MEM_WRITE	0x13000000
#define OP_IO_READ	0x14000000
#define OP_IO_WRITE	0x15000000
#define OP_VRAM_READ	0x16000000
#define OP_VRAM_WRITE	0x17000000
#define OP_INTERRUPT	0x08000000
#define OP_ILLEGAL	0x09000000
#define OP_SP_PUSH	0x0a000000
#define OP_SP_POP	0x0b000000
#define OP_SCANLINE	0x7e000000
#define OP_FRAME	0x7f000000
#define OP_MASK		0xff000000

#define PROBE_BUFFER_MAX 500000
uint32_t* probebuf = 0;
int probebufofs = 0;

void machine_reset_probe_buffer() {
    probebufofs = 0;
    if (!probebuf) probebuf = malloc(sizeof(uint32_t) * PROBE_BUFFER_MAX);
}

uint32_t* machine_get_probe_buffer_address() {
    return probebuf;
}

uint32_t machine_get_probe_buffer_size() {
    return probebufofs;
}

inline void logProbe(uint32_t x) {
    if (probebuf && probebufofs < PROBE_BUFFER_MAX)
        probebuf[probebufofs++] = x;
}

void logClocks(uint16_t clocks) {
    if (clocks) {
        // coalesce clocks?
        if (probebufofs && probebuf[probebufofs-1] < OP_EXECUTE) {
            probebuf[probebufofs-1] += clocks;
        } else {
            logProbe(OP_CLOCKS | clocks);
        }
    }
}

void logExec(uint16_t addr) {
    logProbe(OP_EXECUTE | addr);
}

void logRead(uint16_t addr, uint8_t data) {
    logProbe(OP_MEM_READ | addr | (data<<16));
}

void logWrite(uint16_t addr, uint8_t data) {
    logProbe(OP_MEM_WRITE | addr | (data<<16));
}

void logIORead(uint16_t addr, uint8_t data) {
    logProbe(OP_IO_READ | addr | (data<<16));
}

void logIOWrite(uint16_t addr, uint8_t data) {
    logProbe(OP_IO_WRITE | addr | (data<<16));
}

void logVRAMRead(uint16_t addr, uint8_t data) {
    logProbe(OP_VRAM_READ | addr | (data<<16));
}

void logVRAMWrite(uint16_t addr, uint8_t data) {
    logProbe(OP_VRAM_WRITE | addr | (data<<16));
}

void logInterrupt(uint16_t addr) {
    logProbe(OP_INTERRUPT | addr);
}

void logNewScanline() {
    logProbe(OP_SCANLINE);
}

void logNewFrame() {
    logProbe(OP_FRAME);
}

void logStackPush(uint16_t sp) {
    logProbe(OP_SP_PUSH | sp);
}

void logStackPop(uint16_t sp) {
    logProbe(OP_SP_POP | sp);
}

#endif
