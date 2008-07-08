LDFLAGS+=-lncurses -ljack -lm
all: k20

k20: ringbuffer.o k20.c

clean:
	-rm k20 ringbuffer.o
