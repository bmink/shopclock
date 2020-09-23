P = shopclock
OBJS = main.o ht16k33.o digit.o
CFLAGS = -g -Wall -Wstrict-prototypes
LDLIBS = -lb

$(P): $(OBJS)
	$(CC) -o $(P) $(LDFLAGS) $(OBJS) $(LDLIBS)

clean:
	rm -f *o; rm -f $(P)

