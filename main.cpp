/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgeisler <mgeisler@student.42mulhouse.f    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/09 19:33:07 by gloms             #+#    #+#             */
/*   Updated: 2025/02/12 17:33:17 by mgeisler         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.hpp"

int main(int ac, char **av)
{
	if (ac != 3) {
		std::cerr << "Wrong number of arguments. Should have a port and a password." << std::endl;
		return (1);
	}

	int port = atoi(av[1]);
	std::string password = av[2];

	Server serverOn(port, password);

	struct epoll_event epollEvents;
	struct epoll_event newClient[MAX_EVENTS];

	int epollFd = epoll_create1(0);
	epollEvents.events = EPOLLIN;
	epollEvents.data.fd = serverOn.serverFd;
	epoll_ctl(epollFd, EPOLL_CTL_ADD, serverOn.serverFd, &epollEvents);

	socklen_t addrLen = sizeof(serverOn.address);
	int nbEvents = 0;
	int newClientFd = 0;

	try {
		while (1) {
			nbEvents = epoll_wait(epollFd, newClient, MAX_EVENTS, -1);
			std::cout << "nbEvents: " << nbEvents << std::endl;
	
			for (int i = 0; i < nbEvents; i++) {

				if (newClient[i].data.fd == serverOn.serverFd) {
					newClientFd = accept(serverOn.serverFd, (struct sockaddr *)&serverOn.address, &addrLen);
					
					if (newClientFd < 0)
						perror("biiiiiite");
					
					epollEvents.events = EPOLLIN;
					epollEvents.data.fd = newClientFd;
					epoll_ctl(epollFd, EPOLL_CTL_ADD, newClientFd, &epollEvents);
				}

				else {
					std::string buffer(1024, 0);

					int readBytes = recv(newClient[i].data.fd, &buffer[0], 1024, 0);
					
					if (readBytes < 0)
						perror("biiiiiite");

					//parser de bytes recu
					//serverOn.parser(buffer);
					std::cout << newClientFd << ": " << buffer << std::endl;
				}
			}
		}
	}

	catch (const std::exception &e) {
		std::cerr << e.what() << std::endl;
	}
	return 0;
}
