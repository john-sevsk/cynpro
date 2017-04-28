CFLAGS=$(shell pkg-config libusb-1.0 --cflags) -Wall
LDFLAGS=$(shell pkg-config libusb-1.0 --libs)

cynpro: cynpro.c

clean:
	rm cynpro
