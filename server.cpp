#include <bits/stdc++.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include "skiplist.h"
#include "timer.h"
#include "signal.h"
#include "configs.h"
#define FILE_PATH "./store/dump_file"
#define FILE_DIR "./store/"

#define MAX_LEVEL 6
#define	MAX_EVENT_NUMBER 1024

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

void TimeoutHandler() {
	alarm(signaler.timeout_);
}

int setnonblocking(int fd) {
	int old_option = fcntl(fd, F_GETFL);
	int new_option = old_option | O_NONBLOCK;
	fcntl(fd, F_SETFL, new_option);
	return old_option;
}

void AddFd(int epoll_fd, int fd) {
	epoll_event event;
	event.data.fd = fd;
	event.events = EPOLLIN | EPOLLET;
	epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event);
	setnonblocking(fd);
}

void UpdateMaxMemory(std::unordered_map<std::string, std::shared_ptr<SkipList<std::string, int>>>& map) {
	for(auto& it : map) {
		it.second->SetMaxMemory();
	}
	return;
}

int main(int argc, char* argv[]) {
	srand((unsigned int)(time(nullptr)));

	if(argc != 2) {
		printf("Usage: %s [port]\n", basename(argv[0]));
		return 1;
	}

	// 获取配置信息
	Configs* configs = Configs::GetInstance();

	int port = atoi(argv[1]);

	int ret = 0;
	struct sockaddr_in address;
	bzero(&address, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	// inet_pton(AF_INET, ip, &address.sin_addr);
	address.sin_port = htons(port);

	int listen_fd = socket(PF_INET, SOCK_STREAM, 0);
	assert(listen_fd >= 0);

	ret = bind(listen_fd, (struct sockaddr*)&address, sizeof(address));
	if(ret == -1) {
		printf("errno is: %d\n", errno);
		return 1;
	}

	ret = listen(listen_fd, 5);
	assert(ret != -1);

	// epoll_event设置为全局变量，防止每次调用epoll_wait都要重新分配内存
	epoll_event events[MAX_EVENT_NUMBER];
	int epoll_fd = epoll_create(5);
	assert(epoll_fd != -1);
	AddFd(epoll_fd, listen_fd);

	// 创建管道，用于处理信号
	ret = socketpair(PF_UNIX, SOCK_STREAM, 0, signaler.pipe_fd_);
	assert(ret != -1);
	setnonblocking(signaler.pipe_fd_[1]);
	AddFd(epoll_fd, signaler.pipe_fd_[0]);

	// SkipList<std::string, int> skip_list(6);
	Timer<std::string, int> timer;

	// 设置一些信号的处理函数
	// SIGALRM用于处理定时器
	AddSignal(SIGALRM);
	// SIGCHLD用于处理子进程退出
	AddSignal(SIGCHLD);
	// SIGTERM用于处理终止信号
	AddSignal(SIGTERM);
	// SIGHUP用于处理挂起信号
	AddSignal(SIGHUP);
	// SIGINT用于处理中断信号
	AddSignal(SIGINT);

	// 可能存在建立多个有序集合的情况，因此使用unordered_map
	std::unordered_map<std::string, std::shared_ptr<SkipList<std::string, int>>> map;

	// 记录超时的key
	std::vector<std::string> timeout_keys;
	
	bool stop_server = false;
	bool timeout = false;
	bool update_config = false;

	TimeoutHandler();

	while(!stop_server) {
		int number = epoll_wait(epoll_fd, events, MAX_EVENT_NUMBER, -1);
		if((number < 0) && (errno != EINTR)) {
			printf("epoll failure\n");
			break;
		}

		for(int i = 0; i < number; i++) {
			int sockfd = events[i].data.fd;
			if(sockfd == listen_fd) {
				struct sockaddr_in client_address;
				socklen_t client_addrlength = sizeof(client_address);
				int connfd = accept(listen_fd, (struct sockaddr*)&client_address, &client_addrlength);
				// std::cout << "accept a new client: " << connfd << std::endl;
				AddFd(epoll_fd, connfd);
			}
			else if(sockfd == signaler.pipe_fd_[0] && (events[i].events & EPOLLIN)) {
				int sig;
				char signals[1024];
				ret = recv(signaler.pipe_fd_[0], signals, sizeof(signals), 0);
				if(ret == -1) {
					// handle the error
					continue;
				} 
				else if(ret == 0) {
					continue;
				} 
				else {
					for(int i = 0; i < ret; i++) {
						switch(signals[i]) {
							case SIGALRM:
								timeout = true;
								break;
							case SIGCHLD:
							case SIGHUP:
								continue;
							case SIGTERM:
							case SIGINT:
								stop_server = true;
						}
					}
				}
			}
			else if(events[i].events & EPOLLIN) {
				// 处理用户输入的命令
				char buf[1024];
				ret = recv(sockfd, buf, sizeof(buf) - 1, 0);
				if(ret < 0) {
					if(errno != EAGAIN) {
						close(sockfd);
					}
				}
				else if(ret == 0) {
					close(sockfd);
				}
				else {
					buf[ret] = '\0';

					std::string cmd;
					std::vector<std::string> args;
					std::string arg;
					std::stringstream ss(buf);
					ss >> cmd;
					while(ss >> arg) {
						args.push_back(arg);
					}

					std::cout << "cmd: " << cmd << std::endl;
					for(auto& arg : args) {
						std::cout << arg << " ";
					}
					std::cout << std::endl;

					if(cmd == "EXIT" || cmd == "exit" || cmd == "QUIT" || cmd == "quit") {
						// 此处需要将所有的有序集合保存到文件中
						stop_server = true;
						break;
					}

					if(cmd == "save" || cmd == "SAVE") {
						// 将所有的有序集合保存到文件中
						for(auto& it : map) {
							std::string filename = FILE_DIR + it.first + ".txt";
							// 使用ofstream打开文件，如果文件不存在，则创建文件
							std::ofstream ofs(filename);
							if(!ofs) {
								std::cout << "open file " << filename << " failed" << std::endl;
								continue;
							}
							it.second->Save(ofs);
							ofs.close();
						}
						write(sockfd, "OK\n", 3);
						continue;
					}

					// 处理CONFIG命令
					// 在Redis中，CONFIG命令可以用于设置服务器的配置参数；但是只实现了ZSET的最大内存的设置
					// 问题：如何单独设置某个有序集合的最大内存
					// 此处使用设置跳表的最大节点数来代替
					if(cmd == "CONFIG" || cmd == "config") {
						if(args.size() != 3) {
							// std::cout << "Usage: CONFIG SET maxmemory 1000000" << std::endl;
							write(sockfd, "Usage: CONFIG SET maxmemory [number]\n", 36);
							continue;
						}
						if(args[0] == "SET" || args[0] == "set") {
							if(args[1] == "maxmemory" || args[1] == "MAXMEMORY") {
								// 设置最大内存
								int max_memory = std::stoi(args[2]);
								int ret = configs->SetMaxMemory(max_memory);
								write(sockfd, std::to_string(ret).c_str(), std::to_string(ret).size());
								write(sockfd, "\n", 1);
							}
							update_config = true;
						}
						continue;
					}

					if(cmd == "EXPIRE" || cmd == "expire") {
						// 设置过期时间
						if(args.size() != 2) {
							// std::cout << "Usage: EXPIRE key seconds" << std::endl;
							write(sockfd, "Usage: EXPIRE key seconds\n", 26);
							continue;
						}

						std::string key = args[0];
						// 将key和超时时间插入到定时器中
						// std::cout << timer.Push(key, stoi(args[0])) << std::endl;
						int ret = timer.Push(key, std::stoi(args[1]));
						write(sockfd, std::to_string(ret).c_str(), std::to_string(ret).size());
						write(sockfd, "\n", 1);
						continue;
					}

					// ZSET相关的命令
					std::string key = args[0];
					std::cout << "key: " << key << std::endl;
					// 使用shared_ptr，防止内存泄漏
					if(map.find(key) == map.end()) {
						if(cmd == "ZADD" || cmd == "zadd") {
							map.insert({key, std::make_shared<SkipList<std::string, int>>(MAX_LEVEL, configs->GetMaxMemory())});
						}
						else {
							// 对于ZREM、ZCARD、EXPIRE返回0，对于ZSCORE、ZRANK返回-1，对于ZDIS返回空字符串
							if(cmd == "ZREM" || cmd == "zrem" || cmd == "ZCARD" || cmd == "zcard") {
								// 发送到客户端
								write(sockfd, "0\n", 2);
							}
							else if(cmd == "ZSCORE" || cmd == "zscore") {
								write(sockfd, "-1\n", 3);
							}
							else if(cmd == "ZDIS" || cmd == "zdis") {
								write(sockfd, "\n", 1);
							}
							else {
								std::cout << "No such cmd" << std::endl;
								write(sockfd, "-1\n", 3);
							}
							continue;
						}
					}

					std::shared_ptr<SkipList<std::string, int>>& skip_list = map[key];

					if(cmd == "ZADD" || cmd == "zadd") {
						if(args.size() != 3) {
							// std::cout << "Usage: ZADD key score member" << std::endl;
							write(sockfd, "Usage: ZADD key score member\n", 29);
							continue;
						}
						// std::cout << skip_list->InsertElement(args[1], stoi(args[0])) << std::endl;
						// TODO: 此处的write需要换行输出，怎么做到？
						// 解答：使用\n
						// 怎么使用\n？使用write(sockfd, "\n", 1);
						// 会不会在客户端分成两行输出？不会，因为\n是一个字符
						std::string member = args[2];
						int score = std::stoi(args[1]);
						int ret = skip_list->InsertElement(member, score);
						write(sockfd, std::to_string(ret).c_str(), std::to_string(ret).size());
						write(sockfd, "\n", 1);
					} 
					else if(cmd == "ZSCORE" || cmd == "zscore") {
						if(args.size() != 2) {
							// std::cout << "Usage: ZSCORE key member" << std::endl;
							write(sockfd, "Usage: ZSCORE key member\n", 25);
							continue;
						}
						// std::cout << skip_list->SearchElement(args[0]) << std::endl;
						int score = skip_list->SearchElement(args[1]);
						write(sockfd, std::to_string(score).c_str(), std::to_string(score).size());
						write(sockfd, "\n", 1);
					}
					else if(cmd == "ZREM" || cmd == "zrem") {
						if(args.size() != 2) {
							// std::cout << "Usage: ZREM key member" << std::endl;
							write(sockfd, "Usage: ZREM key member\n", 23);
							continue;
						}
						// std::cout << skip_list->DeleteElement(args[0]) << std::endl;
						int ret = skip_list->DeleteElement(args[1]);
						write(sockfd, std::to_string(ret).c_str(), std::to_string(ret).size());
						write(sockfd, "\n", 1);
					}
					else if(cmd == "ZCARD" || cmd == "zcard") {
						if(args.size() != 1) {
							// std::cout << "Usage: ZCARD key" << std::endl;
							write(sockfd, "Usage: ZCARD key\n", 17);
							continue;
						}
						// std::cout << skip_list->Length() << std::endl;
						int len = skip_list->Length();
						write(sockfd, std::to_string(len).c_str(), std::to_string(len).size());
						write(sockfd, "\n", 1);
					} 
					else if(cmd == "ZDIS" || cmd == "zdis") {
						if(args.size() != 1) {
							// std::cout << "Usage: ZDIS key" << std::endl;
							write(sockfd, "Usage: ZDIS key\n", 16);
							continue;
						}
						skip_list->DisplayList();
						write(sockfd, "1\n", 2);
					}
					else {
						// std::cout << "Usage: ZADD(zadd) | ZSCORE(zscore) | ZREM(zrem) | ZCARD(zcard) | ZDIS(zdis) | EXPIRE(expire)" << std::endl;
						write(sockfd, "Usage: ZADD(zadd) | ZSCORE(zscore) | ZREM(zrem) | ZCARD(zcard) | ZDIS(zdis) | CONFIG(config) | EXPIRE(expire)\n", 110);
					}
				}
			}
		}
		if(timeout) {
			// 用timeout_keys返回超时的key
			timer.Tick(timeout_keys);
			// 处理超时事件
			// 是否可以在这里删除map中的key，是否会引起一次循环的时间过长？
			// 可以在这里删除，但是需要注意的是，如果在这里删除，那么在处理用户输入的命令时，需要判断map中是否存在key
			for(auto& key : timeout_keys) {
				map.erase(key);
			}
			// 重新设置定时器
			TimeoutHandler();
			timeout = false;
		}

		if(update_config) {
			// 重新加载配置文件
			// 对于已经存在的key，直接修改其max_memory
			UpdateMaxMemory(map);
			update_config = false;
		}
	}

#if 0
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

    skip_list.DeleteElement("Bibber", 8);
    skip_list.DeleteElement("Benjamin", 20);

    std::cout << "skip_list length: " << skip_list.Length() << std::endl;

    skip_list.DisplayList();
#endif

	return 0;
}
