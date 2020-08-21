P = pusherman
OBJS = main.o
CFLAGS = -g -Wall -Wstrict-prototypes
LDLIBS = -lb -lwiringPi

$(P): $(OBJS)
	$(CC) -o $(P) $(LDFLAGS) $(OBJS) $(LDLIBS)

clean:
	rm -f *o; rm -f $(P)

