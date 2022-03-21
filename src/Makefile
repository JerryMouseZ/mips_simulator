OBJS = parser.o instruction.o main.o
CFLAGS = -g -Wall -Werror

simulator: $(OBJS)
	$(CC) -g -o $@ $(OBJS)

clean:
	rm -f simulator $(OBJS)
