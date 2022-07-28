
#include "malloc.h"

#define CHIPS_IMPL

#include "probe.h"

#include "chips/m6502.h"
#include "chips/m6526.h"
#include "chips/m6569.h"
#include "chips/m6581.h"
#include "chips/beeper.h"
#include "chips/kbd.h"
#include "chips/mem.h"
#include "chips/clk.h"
#include "systems/c1530.h"
#include "chips/m6522.h"
#include "systems/c1541.h"
#include "systems/c64.h"

#define MAX_SAMPLES 4096
float sample_buffer[MAX_SAMPLES];
unsigned int sample_count = 0;
char* bios_array = 0;

typedef struct {
    uint8_t kbd_joy1_mask;      /* current joystick-1 state from keyboard-joystick emulation */
    uint8_t kbd_joy2_mask;      /* current joystick-2 state from keyboard-joystick emulation */
    uint8_t joy_joy1_mask;      /* current joystick-1 state from c64_joystick() */
    uint8_t joy_joy2_mask;      /* current joystick-2 state from c64_joystick() */
    kbd_t kbd;                  /* keyboard matrix state */
} C64ControlsState;

void audio_callback_fn(const float* samples, int num_samples, void* user_data) {
    if (sample_count + num_samples < MAX_SAMPLES) {
        memcpy(sample_buffer+sample_count, samples, num_samples*sizeof(float));
        sample_count += num_samples;
    }
}

void machine_hardreset(c64_t* sys) {
    c64_desc_t desc;
    memset(sys, 0, sizeof(c64_t));
    memset(&desc, 0, sizeof(c64_desc_t));
    desc.pixel_buffer_size = c64_max_display_size();
    desc.pixel_buffer = malloc(desc.pixel_buffer_size);
    desc.rom_basic = &bios_array[0x0];
    desc.rom_char = &bios_array[0x2000];
    desc.rom_kernal = &bios_array[0x3000];
    desc.rom_basic_size = 0x2000;
    desc.rom_char_size = 0x1000;
    desc.rom_kernal_size = 0x2000;
    desc.audio_cb = audio_callback_fn;
    desc.audio_num_samples = C64_MAX_AUDIO_SAMPLES;
    c64_init(sys, &desc);
    sys->pixel_buffer = desc.pixel_buffer;
}

c64_t* machine_init(char* bios) {
    c64_t* sys = (c64_t*) malloc(sizeof(c64_t));
    bios_array = bios;
    machine_hardreset(sys);
    return sys;
}

void machine_reset(c64_t* sys) {
    c64_reset(sys);
}

void machine_start_frame(c64_t* sys) {
    kbd_update(&sys->kbd, 1000000/50);
}

void machine_tick(c64_t* sys) {
    c64_tick(sys);
}

void machine_exec(c64_t* sys, uint32_t micro_seconds) {
    c64_exec(sys, micro_seconds);
}

void* machine_get_pixel_buffer(const c64_t* sys) {
    return sys->pixel_buffer;
}

float* machine_get_sample_buffer() {
    return sample_buffer;
}

unsigned int machine_get_sample_count() {
    unsigned int n = sample_count;
    sample_count = 0;
    return n;
}

uint8_t machine_mem_read(c64_t* sys, uint16_t address) {
    return mem_rd(&sys->mem_cpu, address);
}
 
void machine_mem_write(c64_t* sys, uint16_t address, uint8_t value) {
    mem_wr(&sys->mem_cpu, address, value);
}

int machine_get_state_size() {
    return sizeof(c64_t);
}

int machine_get_cpu_state_size() {
    return sizeof(m6502_t);
}

int machine_get_controls_state_size() {
    return sizeof(C64ControlsState);
}

void machine_save_state(const c64_t* sys, c64_t* state) {
    memcpy(state, sys, sizeof(c64_t));
}

void machine_load_state(c64_t* sys, const c64_t* state) {
    memcpy(sys, state, sizeof(c64_t));
}

void machine_save_cpu_state(const c64_t* sys, m6502_t* state) {
    memcpy(state, &sys->cpu, sizeof(m6502_t));
}

void machine_load_cpu_state(c64_t* sys, const m6502_t* state) {
    memcpy(&sys->cpu, state, sizeof(m6502_t));
}

void machine_save_controls_state(const c64_t* sys, C64ControlsState* state) {
    state->kbd_joy1_mask = sys->kbd_joy1_mask;
    state->kbd_joy2_mask = sys->kbd_joy2_mask;
    state->joy_joy1_mask = sys->joy_joy1_mask;
    state->joy_joy2_mask = sys->joy_joy2_mask;
    state->kbd = sys->kbd;
}

void machine_load_controls_state(c64_t* sys, const C64ControlsState* state) {
    sys->kbd_joy1_mask = state->kbd_joy1_mask;
    sys->kbd_joy2_mask = state->kbd_joy2_mask;
    sys->joy_joy1_mask = state->joy_joy1_mask;
    sys->joy_joy2_mask = state->joy_joy2_mask;
    sys->kbd = state->kbd;
}

void machine_key_down(c64_t* sys, int key_code) {
    c64_key_down(sys, key_code);
}

void machine_key_up(c64_t* sys, int key_code) {
    c64_key_up(sys, key_code);
}

void machine_load_rom(c64_t* sys, const uint8_t* ptr, int num_bytes) {
    c64_quickload(sys, ptr, num_bytes);
}

unsigned int machine_cpu_get_pc(c64_t* sys) {
    return sys->cpu.PC;
}

unsigned int machine_cpu_get_sp(c64_t* sys) {
    return sys->cpu.S;
}

bool machine_cpu_is_stable(c64_t* sys) {
    return sys->cpu.PINS & M6502_SYNC;
}

int machine_get_raster_line(c64_t* sys) {
    return sys->vic.rs.v_count;
}

void machine_set_mouse(c64_t* sys, int x, int y) {
    sys->vic.reg.lightpen_xy[0] = x;
    sys->vic.reg.lightpen_xy[1] = y;
}

void* machine_get_debug_pointer(c64_t* sys, int index) {
    switch (index) {
        case 0: return &sys->cpu;
        case 1: return &sys->cia_1;
        case 2: return &sys->cia_2;
        case 3: return &sys->vic;
        case 4: return &sys->sid;
        case 5: return &sys->ram;
        case 6: return &sys->color_ram;
        case 7: return &sys->mem_cpu;
        case 8: return &sys->mem_vic;
        case 9: return &sys->io_mapped;
        default: return 0;
    }
}

