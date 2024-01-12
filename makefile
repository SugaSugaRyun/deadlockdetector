CC = gcc
CFLAGS = -g -pthread
LDFLAGS = -ldl

all: ddmon.so target ddchck

ddmon.so: ddmon.c
	$(CC) -shared -fPIC $< -o $@ -DDEBUG $(CFLAGS) $(LDFLAGS)

target: target.c
	$(CC) $< -o $@ $(CFLAGS)

ddchck: ddchck.c
	$(CC) $< -o $@ $(CFLAGS) -DDEBUG

clean:
	rm -f ddmon.so target ddchck
