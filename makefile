# Makefile
all: diskimage bootloader stdlib lib kernel prog

# Recipes
diskimage:
	dd if=/dev/zero of=out/system.img bs=512 count=2880

bootloader:
	nasm src/asm/bootloader.asm -o out/bootloader
	dd if=out/bootloader of=out/system.img bs=512 count=1 conv=notrunc

lib:
	bcc -ansi -c -O3 -o out/lib_textio.o src/c/lib_textio.c
	bcc -ansi -c -O3 -o out/lib_string.o src/c/lib_string.c
	bcc -ansi -c -O3 -o out/lib_program.o src/c/lib_program.c
	bcc -ansi -c -O3 -o out/lib_fileio.o src/c/lib_fileio.c

kernel:
	bcc -ansi -c -O3 -o out/kernel.o src/c/kernel.c
	nasm -f as86 src/asm/kernel.asm -o out/kernel_asm.o
	nasm -f as86 src/asm/interrupt.asm -o out/interrupt_asm.o
	ld86 -o out/kernel -d out/kernel.o out/kernel_asm.o out/std_lib.o out/interrupt_asm.o 
	dd if=out/kernel of=out/system.img bs=512 conv=notrunc seek=1

prog:
	bcc -ansi -c -O3 -o out/shell.o src/c/shell.c
	ld86 -o out/shell -d out/shell.o out/lib_textio.o out/interrupt_asm.o out/lib_string.o out/lib_program.o out/lib_fileio.o
	bcc -ansi -c -O3 -o out/ls.o src/c/ls.c
	ld86 -o out/ls -d out/ls.o out/lib_textio.o out/interrupt_asm.o out/lib_string.o out/lib_program.o out/lib_fileio.o
	bcc -ansi -c -O3 -o out/cd.o src/c/cd.c
	ld86 -o out/cd -d out/cd.o out/lib_textio.o out/interrupt_asm.o out/lib_string.o out/lib_program.o out/lib_fileio.o
	bcc -ansi -c -O3 -o out/cat.o src/c/cat.c
	ld86 -o out/cat -d out/cat.o out/lib_textio.o out/interrupt_asm.o out/lib_string.o out/lib_program.o out/lib_fileio.o
	bcc -ansi -c -O3 -o out/mkdir.o src/c/mkdir.c
	ld86 -o out/mkdir -d out/mkdir.o out/lib_textio.o out/interrupt_asm.o out/lib_string.o out/lib_program.o out/lib_fileio.o
	bcc -ansi -c -O3 -o out/cp.o src/c/cp.c
	ld86 -o out/cp -d out/cp.o out/lib_textio.o out/interrupt_asm.o out/lib_string.o out/lib_program.o out/lib_fileio.o
	bcc -ansi -c -O3 -o out/mv.o src/c/mv.c
	ld86 -o out/mv -d out/mv.o out/lib_textio.o out/interrupt_asm.o out/lib_string.o out/lib_program.o out/lib_fileio.o

stdlib:
	bcc -ansi -O3 -c -o out/std_lib.o src/c/std_lib.c

run:
	bochs -f src/config/if2230.config

build-run: all run 
