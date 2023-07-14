CFLAGS=`pkg-config fuse --cflags`
LIBS=`pkg-config fuse --libs`

OBJFILES=main.o diofs.o

diofs: $(OBJFILES)
	gcc $(LIBS) -o diofs $(OBJFILES)

diofs.o: diofs.c config.h util.h
	gcc -c -o diofs.o $(CFLAGS) diofs.c

main.o: main.c config.h util.h
	gcc -c -o main.o $(CFLAGS) main.c
