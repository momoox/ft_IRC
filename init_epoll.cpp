#include "irc.hpp"

void	init_epoll(Server serverOn) {

	// int epollFd;
	

	serverOn.epollFd = epoll_create1(0);
	serverOn.epollEvents.events = EPOLLIN;
	serverOn.epollEvents.data.fd = serverOn.serverFd;
	epoll_ctl(serverOn.epollFd, EPOLL_CTL_ADD, serverOn.serverFd, &serverOn.epollEvents);

}