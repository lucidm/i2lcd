CPP=g++
CFLAGS=-Wall -Wextra -Og -std=c++11
LFLAGS=-Wl,--allow-multiple-definition
OBJS=pca9535.o pots.o i2lcd.o lcdtest.o

all: lcdtest

lcdtest: $(OBJS)
	$(CPP) $(CFLAGS) -I./ -o $@ $(LFLAGS) $(OBJS)

$(OBJS): %.o: %.cpp 
	$(CPP) $(CFLAGS) -c -I./ $^ -o $@ $(LFLAGS)

clean:
	rm -f *.o lcdtest
#	$(MAKE) -C i2lcd $@

.PHONY: all clean
