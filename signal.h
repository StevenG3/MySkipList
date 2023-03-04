#ifndef __SIGNAL_H__
#define __SIGNAL_H__

#include "non_copyable.h"

#include <unistd.h>
#include <sys/socket.h>
#include <cassert>

class Signal: NonCopyable {
public:
	Signal(int timeout = 5);

	~Signal();

public:
	int pipe_fd_[2];
	int timeout_;
};

Signal::Signal(int timeout): timeout_(timeout) {
	int res = socketpair(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0, pipe_fd_);
	assert(res != -1);
}

Signal::~Signal() {
	close(pipe_fd_[0]);
	close(pipe_fd_[1]);
}

#endif