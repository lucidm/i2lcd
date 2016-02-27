CC=gcc
CFLAGS=-c -Og
OBJS=pca9535.o pots.o i2lcd.o

all: $(OBJS)

i2lcd.a: $(OBJS)
	ar rcs $@ $^

$(OBJS): %.o: %.c
	$(CC) $(CFLAGS) $(LFLAGS) -I./ -c $< -o $@

clean:
	rm -f *.o *.a

.PHONY: all clean
