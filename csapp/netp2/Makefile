include ../config.mak

.PHONY: all clean

all: echoserveri echoclient echoclientx

echoserveri: echoserveri.o echo.o
	gcc -o $@ $^ $(LFLAGS)

echoclient: echoclient.o
	gcc -o $@ $^ $(LFLAGS)

echoclientx: echoclientx.o
	gcc -o $@ $^ $(LFLAGS)

clean:
	rm *.o -rf
	rm echoserveri -rf
	rm echoclient -rf
