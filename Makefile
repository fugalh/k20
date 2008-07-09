LDFLAGS+=-lncurses -ljack -lm
objects=ringbuffer.o k20.o jack.o options.o
all: options.h k20

k20: $(objects)

options.o: options.h options.c
options.h: options.opts
	opg $<
clean:
	rm -f k20 *.o options.[hc]

.PHONY: clean
