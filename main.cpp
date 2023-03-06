#include <iostream>
#include <unistd.h>
#include <signal.h>
#include "skiplist.h"
#include "timer.h"
#include "signal.h"
#define FILE_PATH "./store/dump_file"

Signal signaler;

void SignalHandler(int signal) {
  int save_errno = errno;
  int msg = signal;
  send(signaler.pipe_fd_[1], (char *)&msg, 1, 0);
  errno = save_errno;
}

void AddSignal(int signal) {
  struct sigaction sa;
  memset(&sa, '\0', sizeof(sa));
  sa.sa_handler = SignalHandler;
  sa.sa_flags |= SA_RESTART;
  sigfillset(&sa.sa_mask);
  assert(sigaction(signal, &sa, nullptr) != -1);
}

void TimeoutHandler()
{
  alarm(signaler.timeout_);
}

int main(int argc, char* argv[]) {


	srand ((unsigned int)(time(NULL)));

	SkipList<std::string, int> skip_list(6);
	Timer<std::string, int> timer;

	AddSignal(SIGALRM);
	AddSignal(SIGTERM);
	TimeoutHandler();

	while(1) {
		printf("[Redis]$ ");
	}

	skip_list.InsertElement("Steven", 1);
	skip_list.InsertElement("Jobs", 3);
	skip_list.InsertElement("Justin", 7);
	skip_list.InsertElement("Bibber", 8);
	skip_list.InsertElement("Bibber", 8);
	skip_list.InsertElement("Abraham", 9);
	skip_list.InsertElement("Lincoln", 19);
	skip_list.InsertElement("Benjamin", 19);
	skip_list.InsertElement("Franklin", 20);

	std::cout << "skip_list length: " << skip_list.Length() << std::endl;

	skip_list.DumpFile();

    skip_list.SearchElement("Bibber", 8);
    skip_list.SearchElement("Benjamin", 20);

	skip_list.DisplayList();

#if 1
    skip_list.DeleteElement("Bibber", 8);
    skip_list.DeleteElement("Benjamin", 20);

    std::cout << "skip_list length: " << skip_list.Length() << std::endl;

    skip_list.DisplayList();
#endif

	return 0;
}