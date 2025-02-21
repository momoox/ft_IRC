/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gloms <rbrendle@student.42mulhouse.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/09 19:33:07 by gloms             #+#    #+#             */
/*   Updated: 2025/02/21 19:37:38 by gloms            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

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
						serverOn.deleteUser(newClient[i].data.fd);

					//parser de bytes recu
					// std::cout << buffer << std::endl;
					serverOn.parser(buffer, newClient[i].data.fd);
				}
			}
		}
	}

	catch (const std::exception &e) {
		std::cerr << e.what() << std::endl;
	}
	return 0;
}


//https://medium.com/@afatir.ahmedfatir/small-irc-server-ft-irc-42-network-7cee848de6f9

//if (fcntl(SerSocketFd, F_SETFL, O_NONBLOCK) == -1) //-> set the socket option (O_NONBLOCK) for non-blocking socket
//throw(std::runtime_error("faild to set option (O_NONBLOCK) on socket"));
