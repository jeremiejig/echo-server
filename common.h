#ifndef _COMMON_H
#define _COMMON_H
#define _XOPEN_SOURCE 600

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <event2/event.h>
#include <errno.h>

// Global pointer

struct event_base* event_basep;

#endif
