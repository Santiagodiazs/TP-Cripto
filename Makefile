CC      = gcc
CFLAGS  = -Wall -Wextra -std=c99 -pedantic
DBGFLAGS = $(CFLAGS) -g -DDEBUG
TARGET  = sss
SRCS    = src/main.c src/cli.c src/bmp.c src/math_gf257.c src/permutation.c src/sss.c
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
	rm -f src/*.o $(TARGET) $(TARGET)_dbg tests/test_bmp tests/test_math_permutation tests/test_distribute tests/compare_bmp tests/test_recover_cat

test_bmp: src/bmp.c tests/test_bmp.c
	$(CC) $(CFLAGS) -o tests/test_bmp src/bmp.c tests/test_bmp.c

test_math_permutation: src/math_gf257.c src/permutation.c tests/test_math_permutation.c
	$(CC) $(CFLAGS) -o tests/test_math_permutation $^

test_distribute: src/math_gf257.c tests/test_distribute.c
	$(CC) $(CFLAGS) -o tests/test_distribute $^

compare_bmp: src/bmp.c tests/compare_bmp.c
	$(CC) $(CFLAGS) -o tests/compare_bmp $^

test_recover_cat: src/cli.c src/bmp.c src/math_gf257.c src/permutation.c src/sss.c tests/test_recover_cat.c
	$(CC) $(CFLAGS) -o tests/test_recover_cat $^


