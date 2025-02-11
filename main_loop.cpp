#include "irc.hpp"

void main_loop(Server serverOn) {
	std::cout << "entree main_loop" << std::endl;
	socklen_t addrLen = sizeof(serverOn.address);
	int newClientFd;
	int nbEvents = 0;
	
	while (1) {
		nbEvents = epoll_wait(serverOn.epollFd, serverOn.newClient, MAX_EVENTS, -1);

		for (int i = 0; i < nbEvents; i++) {
			std::cout << "entree for loop" << std::endl;
			if (serverOn.newClient[i].data.fd == serverOn.serverFd) {
				std::cout << "entree client trouve" << std::endl;
				newClientFd = accept(serverOn.serverFd, (struct sockaddr *)&serverOn.address, &addrLen);
				if (newClientFd < 0)
					perror("biiiiiite");
				serverOn.epollEvents.events = EPOLLIN;
				serverOn.epollEvents.data.fd = newClientFd;
			epoll_ctl(serverOn.epollFd, EPOLL_CTL_ADD, newClientFd, &serverOn.epollEvents);
			}
			else {
				std::string buffer(1024, 0);
				int readBytes = recv(serverOn.newClient[i].data.fd, &buffer[0], 1024, 0);
				if (readBytes < 0)
					perror("biiiiiite");
				std::cout << newClientFd << ": " << buffer << std::endl;
			}
		}
	}
}