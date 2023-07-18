CFLAGS=`pkg-config fuse --cflags` -ggdb -O0
LIBS=`pkg-config fuse --libs`

OBJFILES=main.o diofs.o

run: all
	./diofs -s -f /tmp/diofs/ 2> errors.log

all: diofs

clean:
	rm -f *.o diofs

diofs: $(OBJFILES)
	gcc $(LIBS) -o diofs $(OBJFILES)

diofs.o: diofs.c config.h util.h diofs.h
	gcc -c -o diofs.o $(CFLAGS) diofs.c

main.o: main.c config.h util.h
	gcc -c -o main.o $(CFLAGS) main.c
