# $@ = target file
# $< = first dependency
# $^ = all dependencies

# First rule is the one executed when no parameters are fed to the Makefile
all: fat16info fat32info fat16dir fat32dir fatdir

# Notice how dependencies are built as needed
fat16info: fat16info.o
	gcc -o $@ $^
	
fat32info: fat32info.o
	gcc -o $@ $^
	
fatdir: fatdir.o
	gcc -o $@ $^
	
fat16dir: fat16dir.o
	gcc -o $@ $^
	
fat32dir: fat32dir.o
	gcc -o $@ $^
	
fat16info.o: fat16info.c
	gcc -c $< -o $@

fat32info.o: fat32info.c
	gcc -c $< -o $@
	
fat16dir.o: fat16dir.c
	gcc -c $< -o $@

fat32dir.o: fat32dir.c
	gcc -c $< -o $@

fatdir.o: fatdir.c
	gcc -c $< -o $@

clean:
	rm -f *.o fat16info fat32info fat16dir fat32dir fatdir


