#ifndef _ECHO_SERVER_H
#define _ECHO_SERVER_H

#include <event2/event.h>

#define BUFFER_SIZE 4096

struct client{
	int fd;
	char buffer[BUFFER_SIZE];
	int closed;
	int closing;
	int read_pos;
	int write_pos;
	struct event* event_read_write;
	struct event* event_read;
	struct event* event_write;
};

struct server{
	int fd;
	int closed;
	struct event* event_accept;
};

struct client* new_client(int fd);

void close_client(struct client*);

void starting_accept(int fd);

void on_accept(evutil_socket_t, short, void*);

void on_read_available(evutil_socket_t, short, void*);

void on_write_available(evutil_socket_t, short, void*);

#endif
