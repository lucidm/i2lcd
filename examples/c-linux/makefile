CC=gcc
CFLAGS=-Og
LFLAGS=
OBJS=i2lcd/pca9535.o i2lcd/pots.o i2lcd/i2lcd.o


all: $(OBJS) lcdtest editlcd

editlcd: editlcd.c $(OBJS)
	$(CC) $(CFLAGS) -lform -lpanel -lncurses -D_XOPEN_SOURCE_EXTENDED -I./ -I./i2lcd $^ -o $@ $(LFLAGS)

lcdtest: lcdtest.c $(OBJS)
	$(CC) $(CFLAGS) -lm -I./ -I./i2lcd $^ -o $@ $(LFLAGS)

$(OBJS): i2lcd/%.o: i2lcd/%.c
	$(MAKE) -C i2lcd

clean:
	rm -f *.o lcdtest editlcd
	$(MAKE) -C i2lcd $@

.PHONY: all clean
