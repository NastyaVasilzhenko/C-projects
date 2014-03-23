CC=gcc -g
CFLAGS= -pedantic -ansi -Wall -Werror
LFLAGS= -pthread

OBJS=   MsgQueue.o mem.o flags.o verbose.o Sync.o queue.o
TEST=  Ping.o Pong.o	

all: $(OBJS) $(TEST)
	$(CC) $(OBJS) Ping.o -o Ping $(LFLAGS)
	$(CC) $(OBJS) Pong.o -o Pong $(LFLAGS)

include depends
depends:
	$(CC) -MM $(OBJS:.o=.c) $(TEST:.o=.c) > depends

clean:
	rm Ping Pong $(OBJS) $(TEST)
