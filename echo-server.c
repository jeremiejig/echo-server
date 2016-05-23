#include "common.h"
#include "echo-server.h"

void dummy(evutil_socket_t fd, short what, void *arg);

void on_read_write_available(evutil_socket_t fd, short what, void* arg) {
}

void starting_accept(int fd) {
	struct server* serverp;
	if(fcntl(fd, F_SETFL, O_NONBLOCK) == -1) {
		perror("fcntl set NONBLOCK");
		return;
	}
	serverp = malloc(sizeof(struct server));
	serverp->event_accept = event_new(event_basep, fd, EV_READ, on_accept, serverp);
	//event_add(serverp->event_accept, NULL);
	on_accept(fd, EV_READ, serverp);
}

void on_accept(evutil_socket_t fd, short what, void* data) {
	int cfd;
	struct server* serverp = data;
	for(;;) {
		cfd = accept(fd, NULL, NULL);
		if(cfd == -1) {
			if(errno == EAGAIN || errno == EWOULDBLOCK) {
				perror("repending accept");
				event_add(serverp->event_accept, NULL);
				return;
			} else {
				perror("accept");
				event_free(serverp->event_accept);
				free(serverp);
				close(fd);
				return;
			}
		} else {
			struct client* c;
			printf("New connection ! %d\n", cfd);
			if(fcntl(cfd, F_SETFL, O_NONBLOCK) < 0) {
				perror("setting client NONBLOCK");
				close(cfd);
			} else {
				c = new_client(cfd);
				event_add(c->event_read, 0);
			}
		}
	}
}

struct client* new_client(int fd) {
	struct client* clientp = malloc(sizeof(struct client));
	clientp->fd = fd;
	clientp->closing = 0;
	clientp->closed = 0;
	clientp->read_pos = 0;
	clientp->write_pos = 0;
	clientp->event_read = event_new(event_basep, fd, EV_READ, on_read_available, clientp);
	clientp->event_write = event_new(event_basep, fd, EV_WRITE, on_write_available, clientp);

	return clientp;
}

void close_client(struct client* clientp) {
	printf("client closed %d\n", clientp->fd);
	if(close(clientp->fd) == -1)
		perror("close client");
	event_free(clientp->event_read);
	event_free(clientp->event_write);
	free(clientp);
}

// will perform write if possible.
// Return: 	0 when finish writing all buffer.
// 		-1 when an error occured (the error of write)
int do_write(struct client* clientp) {
	int count;
	while(clientp->read_pos != clientp->write_pos && 0 <
			(count = write(clientp->fd, clientp->buffer + clientp->write_pos, clientp->read_pos - clientp->write_pos))) {
		//printf("write %d bytes on fd %d\n", count, clientp->fd);
		clientp->write_pos += count;
	}
	assert(clientp->read_pos == clientp->write_pos || count == -1);
	if(clientp->read_pos == clientp->write_pos) {
		// Read Closed, closing stream.
		if(clientp->closing)
			clientp->closed = 1;
		else {
			clientp->read_pos = 0;
			clientp->write_pos = 0;
		}
		count = 0;
	}
	else if(errno == EAGAIN || errno == EWOULDBLOCK) {
		// printf("repending write %d \n", clientp->fd);
		event_add(clientp->event_write, 0);
	}
	else if(errno == ECONNRESET) {
		close_client(clientp);
	}
	else {
		perror("write");
	}
	return count;
}

void on_read_available(evutil_socket_t fd, short what, void* data) {
	struct client* clientp = data;
	int count;
	while(clientp->read_pos < BUFFER_SIZE && (count = read(fd, clientp->buffer + clientp->read_pos, BUFFER_SIZE - clientp->read_pos)) > 0) {
		//printf("read %d bytes on %d\n", count, fd);
		clientp->read_pos += count;
	}
	// nothing to read
	if(count < 0) {
		//perror("read");
		if(clientp->read_pos != clientp->write_pos)
			event_add(clientp->event_write, 0);
		else
			event_add(clientp->event_read, 0);
	}
	// buffer full
	else if(count > 0) {
		assert(clientp->read_pos == BUFFER_SIZE);
		event_add(clientp->event_write, 0);
	}
	// The stream has been closed.
	else {
		clientp->closing = 1;
		// check if nothing to write in buffer (normaly there isn't)
		if(clientp->read_pos != clientp->write_pos)
			event_add(clientp->event_write, 0);
		else
			close_client(clientp);
	}
}

void on_write_available(evutil_socket_t fd, short what, void* data) {
	struct client* clientp = data;
	int count;
	while((count = do_write(clientp)) > 0);
	// If return -1 do_write will have reenqueue event_write
	if(count == 0) {
		event_add(clientp->event_read, 0);
	}
}
