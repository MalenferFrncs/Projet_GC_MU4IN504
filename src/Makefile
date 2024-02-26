CC = clang
CFLAGS = -Wall -Wextra -g
OBJS = alloc.o interp.o minizam.o mlvalues.o parser.o primitives.o domain.o

all: minizam

minizam: $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	$(RM) $(OBJS)
	$(RM) minizam
