/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgeisler <mgeisler@student.42mulhouse.f    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/09 19:33:07 by gloms             #+#    #+#             */
/*   Updated: 2025/02/10 20:30:26 by mgeisler         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.hpp"
#include <errno.h>
#include <stdio.h>
#include <string.h>

#define MAX_EVENTS 10

int main(int ac, char **av)
{

	(void) ac;
	(void) av;
	int epollFd;
	int newClientFd;
	struct epoll_event epollEvents; //
	struct epoll_event newClient[MAX_EVENTS];

	epollFd = epoll_create1(0);
	try {
		Server serverOn(6667);

		socklen_t addrLen = sizeof(serverOn.address);

		listen(serverOn.serverFd, 10);
		epollEvents.events = EPOLLIN;
		epollEvents.data.fd = serverOn.serverFd;
		epoll_ctl(epollFd, EPOLL_CTL_ADD, serverOn.serverFd, &epollEvents);

		while (1) {
			int nbEvents = epoll_wait(epollFd, newClient, MAX_EVENTS, -1);
			std::cout << "nb events: " << nbEvents << std::endl;
			for (int i = 0; i < nbEvents; i++) {
				if (newClient[i].data.fd == serverOn.serverFd) {
					newClientFd = accept(serverOn.serverFd, (struct sockaddr *)&serverOn.address, &addrLen);
					if (newClientFd < 0)
						perror("biiiiiite");
					epollEvents.events = EPOLLIN;
					epollEvents.data.fd = newClientFd;
					epoll_ctl(epollFd, EPOLL_CTL_ADD, newClientFd, &epollEvents);
				}
			}
		}
	}
	catch (const std::exception &e) {
		std::cerr << e.what() << std::endl;
	}
	return 0;
}
