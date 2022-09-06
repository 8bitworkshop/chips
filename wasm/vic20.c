
#include "malloc.h"

#define CHIPS_IMPL

#include "probe.h"

#include "chips/m6502.h"
#include "chips/m6522.h"
#include "chips/m6561.h"
#include "chips/kbd.h"
#include "chips/mem.h"
#include "chips/clk.h"
#include "systems/c1530.h"
#include "systems/vic20.h"

#define MAX_SAMPLES 4096
float sample_buffer[MAX_SAMPLES];
unsigned int sample_count = 0;
char* bios_array = 0;

typedef struct {
    uint8_t kbd_joy_mask;      /* current joystick-1 state from keyboard-joystick emulation */
    uint8_t joy_joy_mask;      /* current joystick-1 state from vic20_joystick() */
    kbd_t kbd;                  /* keyboard matrix state */
} VIC20ControlsState;

void audio_callback_fn(const float* samples, int num_samples, void* user_data) {
    if (sample_count + num_samples < MAX_SAMPLES) {
        memcpy(sample_buffer+sample_count, samples, num_samples*sizeof(float));
        sample_count += num_samples;
    }
}

void machine_hardreset(vic20_t* sys) {
    vic20_desc_t desc;
    memset(sys, 0, sizeof(vic20_t));
    memset(&desc, 0, sizeof(vic20_desc_t));
    desc.pixel_buffer_size = vic20_max_display_size();
    desc.pixel_buffer = malloc(desc.pixel_buffer_size);
    desc.rom_basic = &bios_array[0x0];
    desc.rom_char = &bios_array[0x2000];
    desc.rom_kernal = &bios_array[0x3000];
    desc.rom_basic_size = 0x2000;
    desc.rom_char_size = 0x1000;
    desc.rom_kernal_size = 0x2000;
    desc.audio_cb = audio_callback_fn;
    desc.audio_num_samples = VIC20_MAX_AUDIO_SAMPLES;
    vic20_init(sys, &desc);
    sys->pixel_buffer = desc.pixel_buffer;
}

vic20_t* machine_init(char* bios) {
    vic20_t* sys = (vic20_t*) malloc(sizeof(vic20_t));
    bios_array = bios;
    machine_hardreset(sys);
    return sys;
}

void machine_reset(vic20_t* sys) {
    vic20_reset(sys);
}

void machine_start_frame(vic20_t* sys) {
    kbd_update(&sys->kbd, 1000000/50);
}

void machine_tick(vic20_t* sys) {
    vic20_tick(sys);
}

void machine_exec(vic20_t* sys, uint32_t micro_seconds) {
    vic20_exec(sys, micro_seconds);
}

void* machine_get_pixel_buffer(const vic20_t* sys) {
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

uint8_t machine_mem_read(vic20_t* sys, uint16_t address) {
    return mem_rd(&sys->mem_cpu, address);
}
 
void machine_mem_write(vic20_t* sys, uint16_t address, uint8_t value) {
    mem_wr(&sys->mem_cpu, address, value);
}

int machine_get_state_size() {
    return sizeof(vic20_t);
}

int machine_get_cpu_state_size() {
    return sizeof(m6502_t);
}

int machine_get_controls_state_size() {
    return sizeof(VIC20ControlsState);
}

void machine_save_state(const vic20_t* sys, vic20_t* state) {
    memcpy(state, sys, sizeof(vic20_t));
}

void machine_load_state(vic20_t* sys, const vic20_t* state) {
    memcpy(sys, state, sizeof(vic20_t));
}

void machine_save_cpu_state(const vic20_t* sys, m6502_t* state) {
    memcpy(state, &sys->cpu, sizeof(m6502_t));
}

void machine_load_cpu_state(vic20_t* sys, const m6502_t* state) {
    memcpy(&sys->cpu, state, sizeof(m6502_t));
}

void machine_save_controls_state(const vic20_t* sys, VIC20ControlsState* state) {
    state->kbd_joy_mask = sys->kbd_joy_mask;
    state->joy_joy_mask = sys->joy_joy_mask;
    state->kbd = sys->kbd;
}

void machine_load_controls_state(vic20_t* sys, const VIC20ControlsState* state) {
    sys->kbd_joy_mask = state->kbd_joy_mask;
    sys->joy_joy_mask = state->joy_joy_mask;
    sys->kbd = state->kbd;
}

void machine_key_down(vic20_t* sys, int key_code) {
    vic20_key_down(sys, key_code);
}

void machine_key_up(vic20_t* sys, int key_code) {
    vic20_key_up(sys, key_code);
}

void machine_load_rom(vic20_t* sys, const uint8_t* ptr, int num_bytes) {
    // TODO: vic20_insert_rom_cartridge
    vic20_quickload(sys, ptr, num_bytes);
}

unsigned int machine_cpu_get_pc(vic20_t* sys) {
    return sys->cpu.PC;
}

unsigned int machine_cpu_get_sp(vic20_t* sys) {
    return sys->cpu.S;
}

bool machine_cpu_is_stable(vic20_t* sys) {
    return sys->cpu.PINS & M6502_SYNC;
}

int machine_get_raster_line(vic20_t* sys) {
    return sys->vic.rs.v_count;
}

void* machine_get_debug_pointer(vic20_t* sys, int index) {
    switch (index) {
        case 0: return &sys->cpu;
        case 1: return &sys->via_1;
        case 2: return &sys->via_2;
        case 3: return &sys->vic;
        case 5: return &sys->ram0;
        case 6: return &sys->color_ram;
        default: return 0;
    }
}

