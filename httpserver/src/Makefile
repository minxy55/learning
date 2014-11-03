
objs :=  	debug.o \
			sockets.o \
			tools.o \
  			http.o \
  			threads.o \
  			httpserver.o

image := httpserver

all: $(objs)
	gcc -O2 -o $(image) $^ -lpthread

clean:
	rm -rf $(objs) $(image)
