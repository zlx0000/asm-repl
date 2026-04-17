asm-repl: asm-repl.c
	gcc ./asm-repl.c -o ./asm-repl

asm-repl-with-sig-handler: asm-repl-with-sig-handler.c
	gcc ./asm-repl-with-sig-handler.c -o asm-repl-with-sig-handler

asm-repl-with-sig-handler-rwx: asm-repl-with-sig-handler-rwx.c
	gcc ./asm-repl-with-sig-handler-rwx.c -o asm-repl-with-sig-handler-rwx

a.out: asm-repl.c
	gcc ./asm-repl.c -o ./a.out -O0 -g

test: asm-repl.c
	gcc ./asm-repl.c -o ./test && strip ./test


#-Wl,-T,link.ld