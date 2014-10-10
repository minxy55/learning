include ../config.mak

.PHONY: all clean

all: tinyweb

tinyweb: tiny.o
	gcc -o $@ $^ $(LFLAGS)

clean:
	rm *.o -rf 
	rm tinyweb -rf
