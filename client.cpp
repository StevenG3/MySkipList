// client.cpp : Defines the entry point for the console application.
// 作为客户端，连接到服务器，发送命令，接收结果
#include <bits/stdc++.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/epoll.h>

#define MAX_EVENT_NUMBER 1024

void AddFd(int epoll_fd, int fd) {
	epoll_event event;
	event.data.fd = fd;
	event.events = EPOLLIN | EPOLLET;
	epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event);
}

int main(int argc, char* argv[]) {
	if(argc != 3) {
		printf("Usage: %s [ip] [port]\n", basename(argv[0]));
		return 1;
	}
	const char* ip = argv[1];
	int port = atoi(argv[2]);

	struct sockaddr_in server_addr;
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	inet_pton(AF_INET, ip, &server_addr.sin_addr);
	// 利用socket发送命令
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	assert(sockfd >= 0);
	if(connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
		printf("connection failed\n");
		close(sockfd);
		return 1;
	}

	// epoll_event events[MAX_EVENT_NUMBER];
	// int epoll_fd = epoll_create(5);
	// assert(epoll_fd != -1);
	// AddFd(epoll_fd, sockfd);

	// AddFd(epoll_fd, STDIN_FILENO);
	
	// bool stop_client = false;
	// while(!stop_client) {
	// 	// flush是为了防止缓冲区中有数据
	// 	std::cout << "[Redis]$ " << std::flush;
	// 	int number = epoll_wait(epoll_fd, events, MAX_EVENT_NUMBER, -1);
	// 	if(number < 0) {
	// 		printf("epoll failure\n");
	// 		break;
	// 	}
	// 	else if(number == 0) {
	// 		continue;
	// 	}
	// 	else {
	// 		for(int i = 0; i < number; ++i) {
	// 			int datafd = events[i].data.fd;
	// 			char buf[1024];
	// 			if(datafd == STDIN_FILENO) {
	// 				// 处理用户输入
	// 				std::cin.getline(buf, 1024);
	// 				// 直接回车的情况需要单独处理
	// 				if(strlen(buf) == 0) {
	// 					continue;
	// 				}
	// 				// 当输入quit或exit时，退出
	// 				if(strcmp(buf, "quit") == 0 || strcmp(buf, "exit") == 0) {
	// 					stop_client = true;
	// 					break;
	// 				}

	// 				write(sockfd, buf, strlen(buf));
	// 				break;
	// 			}
	// 			else if(datafd == sockfd) {
	// 				// 处理服务器返回的结果
	// 				int len = read(sockfd, buf, 1024);
	// 				write(STDOUT_FILENO, buf, len);
	// 				break;
	// 			}
	// 		}
	// 	}
	// }
	while(true) {
		std::cout << "[Redis]$ ";
		char buf[1024];
		std::cin.getline(buf, 1024);

		if(strlen(buf) == 0) {
			continue;
		}
		if(strcmp(buf, "quit") == 0 || strcmp(buf, "exit") == 0) {
			break;
		}
		// 当服务端关闭时，客户端也会关闭，如何处理？
		// 解答：客户端在发送命令时，如果服务端关闭，会返回-1，客户端也会关闭
		if(write(sockfd, buf, strlen(buf)) == -1) {
			printf("write error\n");
			break;
		}
		int len = read(sockfd, buf, 1024);
		if(len < 0) {
			printf("read error\n");
			break;
		}
		else if(len == 0) {
			break;
		}
		write(STDOUT_FILENO, buf, len);
	}

	// close(epoll_fd);
	close(sockfd);

	return 0;
}