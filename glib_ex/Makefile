
CFLAGS := -I/usr/local/include/glib-2.0 -I/usr/lib/i386-linux-gnu/glib-2.0/include 
LDFLAGS := -L/usr/local/lib/ -lglib-2.0

all: glist_ex ghash_ex gtree_ex

glist_ex: glist_ex.o
	$(CC) -o $@ $^ $(LDFLAGS)

ghash_ex: ghash_ex.o
	$(CC) -o $@ $^ $(LDFLAGS)

gtree_ex: gtree_ex.o
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	rm *.o -rf
	rm glist_ex ghash_ex gtree_ex -rf
