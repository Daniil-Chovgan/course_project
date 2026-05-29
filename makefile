CC = emcc
CFLAGS = -O3 --no-entry -sERROR_ON_UNDEFINED_SYMBOLS=0 -sFILESYSTEM=0 -sASSERTIONS=0 -sEXPORT_KEEPALIVE=1 -Wall
WASMPG = /Users/daniilchovgan/Documents/sem_4/tech_prog/project/WebAssembly-protocol-generator/target/release/wasmpg
WASI_STUB = wasi-stub

all: nelder_mead.wasm

# Генерация файлов протокола
protocol/protocol.c protocol/protocol.h protocol.typ: nelder_mead.prot
	@mkdir -p protocol
	$(WASMPG) nelder_mead.prot -t . -c protocol

# Компиляция и заглушка WASI
nelder_mead.wasm: main.c protocol/protocol.c
	$(CC) $(CFLAGS) main.c protocol/protocol.c -o nelder_mead.wasm -I"protocol" -lm
	$(WASI_STUB) --stub-function env:__syscall_unlinkat,env:__syscall_faccessat ./nelder_mead.wasm
	mv "./nelder_mead - stubbed.wasm" ./nelder_mead.wasm

clean:
	rm -rf protocol nelder_mead.wasm
