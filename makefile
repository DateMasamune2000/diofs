CFLAGS=`pkg-config fuse --cflags` -ggdb -O0
LIBS=`pkg-config fuse --libs`

OBJFILES=main.o diofs.o util.o dentry.o

run: all
	./diofs -s -f /tmp/diofs/

all: diofs

clean:
	rm -f *.o diofs

diofs: $(OBJFILES)
	gcc $(LIBS) -o diofs $(OBJFILES)

util.o: util.h util.c
	gcc -c -o util.o $(CFLAGS) util.c

diofs.o: diofs.c config.h util.h diofs.h
	gcc -c -o diofs.o $(CFLAGS) diofs.c

dentry.o: dentry.c dentry.h util.h
	gcc -c -o dentry.o $(CFLAGS) dentry.c

main.o: main.c config.h util.h
	gcc -c -o main.o $(CFLAGS) main.c
