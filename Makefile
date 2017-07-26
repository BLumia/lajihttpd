CC = gcc
CFLAGS = -g -std=c11 -D_POSIX_C_SOURCE -I../libaji/include/ -L../libaji/lib/ -laji -lrt -pthread

all: httpd.elf
	@echo "Done"

httpd.elf: main.o config.o epoll_manager.o http_utils.o socket_wrapper.o
	$(CC) main.o config.o epoll_manager.o http_utils.o socket_wrapper.o $(CFLAGS) -o $@ 

epoll_manager.o: epoll_manager.c
	$(CC) epoll_manager.c -c $(CFLAGS)

config.o: config.c
	$(CC) config.c -c $(CFLAGS)

main.o: main.c
	$(CC) main.c -c $(CFLAGS)

http_utils.o: http_utils.c
	$(CC) http_utils.c -c $(CFLAGS)

socket_wrapper.o: socket_wrapper.c
	$(CC) socket_wrapper.c -c $(CFLAGS)

clean:
	rm ./*.o
	rm ./*.elf

.PHONY: clean all
