CFLAGS=$(shell pkg-config libusb-1.0 --cflags) -std=c99 -Wall
LDLIBS=$(shell pkg-config libusb-1.0 --libs)

cynpro: cynpro.c

clean:
	rm cynpro
