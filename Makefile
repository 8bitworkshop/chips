
CLANG="$(EMSDK)/upstream/bin/clang"

testall: c64.test zx.test cpc.test vic20.test

%.test: wasm/%.wasm
	which node
	which nodejs
	node --experimental-modules --experimental-wasm-modules wasm/test$*.mjs
	
# from https://dassur.ma/things/c-to-webassembly/
wasm/%.wasm: wasm/%.c systems/*.h chips/*.h wasm/probe.h
	$(CLANG) -v \
	-I. \
	-Iwasm \
	--target=wasm32 \
	-O2 \
	-flto \
	-nostdlib \
	-Wl,--no-entry \
	-Wl,--export-all \
	-Wl,--lto-O2 \
	-o $@ \
	$<

