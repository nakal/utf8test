
CFLAGS+=-Wall -Werror -Wstrict-prototypes -Wmissing-prototypes -pedantic
BIN=utf8test

OBJS=$(BIN).o

all: $(OBJS)
	$(CC) $(LDFLAGS) -o $(BIN) $(OBJS)

$(BIN).o: $(BIN).c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS)
