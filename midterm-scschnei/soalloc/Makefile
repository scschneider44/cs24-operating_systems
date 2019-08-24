OBJS = smallobj.o tester.o

CFLAGS = -Wall -Werror -g -Og

all: tester

tester: $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

clean:
	$(RM) *.o tester

.PHONY: all clean

