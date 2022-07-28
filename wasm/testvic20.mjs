
import * as vic20 from './vic20.wasm';
console.log(vic20)
vic20.memory.grow(32);

import * as fs from 'fs';
const bios = fs.readFileSync('wasm/vic20generic.rom')

const cArrayPointer = vic20.malloc(0x5000);
console.log(cArrayPointer)

const cArray = new Uint8Array(
  vic20.memory.buffer,
  cArrayPointer,
  0x5000
);
cArray.set(bios);

console.log('init')
const sys = vic20.machine_init(cArrayPointer);
console.log(sys)

vic20.machine_reset(sys)

console.log(vic20.vic20_std_display_width())
console.log(vic20.vic20_std_display_height())
console.log(vic20.vic20_max_display_size());

console.log(vic20.vic20_display_width(sys));
console.log(vic20.vic20_display_height(sys));

console.log('pixels', vic20.machine_get_pixel_buffer(sys));

vic20.machine_tick(sys);
vic20.vic20_exec(sys, 1000000);

const pixels = new Uint8Array(
  vic20.memory.buffer,
  vic20.machine_get_pixel_buffer(sys),
  vic20.vic20_max_display_size()
);

fs.writeFileSync('testvic20.rgba', pixels);

console.log('probe', vic20.machine_get_probe_buffer_size());
