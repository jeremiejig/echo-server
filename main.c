
#include "common.h"
#include "echo-server.h"
#include <stdio.h>
#include <signal.h>

#define DEFAULT_PORT 8007

void dummy(evutil_socket_t fd, short what, void *arg) {
	const char *data = arg;
	printf("Got an event on socket %d:%s%s%s%s [%s]\n",
			(int) fd,
			(what&EV_TIMEOUT) ? " timeout" : "",
			(what&EV_READ) ? " read" : "",
			(what&EV_WRITE) ? " write" : "",
			(what&EV_SIGNAL) ? " signal" : "",
			data);
	close(fd);
	return;
}

void handle_signal() {
	event_base_loopbreak(event_basep);
}

void set_signal() {
	struct sigaction action;
	action.sa_handler = handle_signal;
	action.sa_flags = 0;
	sigemptyset(&(action.sa_mask));
	sigaction(SIGINT, &action, NULL);
	sigaction(SIGQUIT, &action, NULL);
	sigaction(SIGTERM, &action, NULL);
}


int main() {
	int status = 0;
	int sfd;
	struct sockaddr_in listen_addr;
	socklen_t listen_addr_len = sizeof(listen_addr);
	event_basep = event_base_new();

	/* Initialisation */
	set_signal();
	listen_addr.sin_family = AF_INET;
	listen_addr.sin_port = htons(DEFAULT_PORT);
	listen_addr.sin_addr.s_addr = 0x0;
	sfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sfd == -1) {
		perror("in socket");
		exit(1);
	}
	if(bind(sfd, (struct sockaddr*) &listen_addr, listen_addr_len) == -1) {
		perror("bind"); exit(1);
	}
	if(listen(sfd, 64) == -1) {
		perror("listen"); exit(1);
	}
	printf("socket fd %d listening\n",sfd);
	/* We bind to 0.0.0.0:DEFAULT_PORT */
	starting_accept(sfd);

	status = event_base_loop(event_basep,0);
	printf("Hello world !\n");
	return status;
}
