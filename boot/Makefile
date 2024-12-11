# $@ = target file
# $< = first dependency
# $^ = all dependencies

# First rule is the one executed when no parameters are fed to the Makefile
all: writeboot boot-fat32.bin boot-fat16.bin boot-fat32-nombr.bin boot-fat16-nombr.bin boot.bin

# Notice how dependencies are built as needed

boot.bin: boot.asm
	nasm $< -f bin -o $@
	./writeboot harddisk.img $@

boot-fat32.bin: boot-fat32.asm
	nasm $< -f bin -o $@

boot-fat16.bin: boot-fat16.asm
	nasm $< -f bin -o $@

boot-fat32-nombr.bin: boot-fat32-nombr.asm
	nasm $< -f bin -o $@

boot-fat16-nombr.bin: boot-fat16-nombr.asm
	nasm $< -f bin -o $@

writeboot: writeboot.o
	gcc -o $@ $^
	
writeboot.o: writeboot.c
	gcc -c $< -o $@

clean:
	rm -f writeboot.o writeboot boot.bin boot-fat32.bin boot-fat16.bin boot-fat32-nombr.bin boot-fat16-nombr.bin

