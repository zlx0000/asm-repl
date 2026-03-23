asm-repl: asm-repl.c
	gcc ./asm-repl.c -o ./asm-repl

a.out: asm-repl.c
	gcc ./asm-repl.c -o ./a.out -O0 -g

test: asm-repl.c
	gcc ./asm-repl.c -o ./test && strip ./test


#-Wl,-T,link.ld