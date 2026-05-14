CC      = gcc
CFLAGS  = -Wall -Wextra -std=c99 -pedantic
DBGFLAGS = $(CFLAGS) -g -DDEBUG
TARGET  = sss
SRCS    = src/main.c src/cli.c src/bmp.c
OBJS    = $(SRCS:.c=.o)

.PHONY: all clean debug

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

debug: $(SRCS)
	$(CC) $(DBGFLAGS) -o $(TARGET)_dbg $^

clean:
	rm -f src/*.o $(TARGET) $(TARGET)_dbg tests/test_bmp

test_bmp: src/bmp.c tests/test_bmp.c
	$(CC) $(CFLAGS) -o tests/test_bmp src/bmp.c tests/test_bmp.c
