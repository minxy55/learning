
include config.mak

all: .lib .netp2


.lib: src/csapp.c
	gcc $(CFLAGS) -c -o csapp.o $^
	ar -cvr lib/libcsapp.a csapp.o

.netp2:
	make -C netp2 all
