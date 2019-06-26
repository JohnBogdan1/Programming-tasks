CC = gcc
CPPFLAGS = -Iutils
CFLAGS = -Wall -Wextra -g -fPIC
LDLIBS = -lpthread

.PHONY: all clean

all: build

build: libscheduler.so

libscheduler.so: so_scheduler.o priority_table.o t_list.o
	$(CC) -shared -o $@ $^

so_scheduler.o: so_scheduler.c

priority_table.o: priority_table.c

t_list.o: t_list.c

clean:
	-rm -f *~ *.o *.so

