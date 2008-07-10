LDFLAGS+=-lncurses -ljack -lm
PREFIX=/usr/local
objects=ringbuffer.o k20.o jack.o options.o
all: options.h k20

k20: $(objects)

options.o: options.h options.c
options.h: options.opts
	./opg $<
clean:
	rm -f k20 *.o options.[hc]

install: all
	install -d $(PREFIX)/bin
	install -t $(PREFIX)/bin k20

uninstall:
	rm -f $(PREFIX)/bin/k20

.PHONY: clean install
