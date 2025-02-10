/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gloms <rbrendle@student.42mulhouse.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/09 19:33:07 by gloms             #+#    #+#             */
/*   Updated: 2025/02/10 18:07:24 by gloms            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.hpp"

#define MAX_EVENTS 10

int main(int ac, char **av)
{
	int epollFd;
	int newClientFd;
	struct epoll_event epollEvents; //
	struct epoll_event newClient[MAX_EVENTS];

	epollFd = epoll_create1(0);
	try {
		Server serverOn(6667);
		listen(serverOn.serverFd, 10);
		epollEvents.events = EPOLLIN;
		epollEvents.data.fd = serverOn.serverFd;
		epoll_ctl(epollFd, EPOLL_CTL_ADD, serverOn.serverFd, &epollEvents);
		while (1) {
			int nbEvents = epoll_wait(epollFd, newClient, MAX_EVENTS, -1);
			for (int i = 0; i < nbEvents; i++) {
				if (newClient[i].data.fd == serverOn.serverFd) {
					newClientFd = accept(serverOn.serverFd, (struct sockaddr *)&serverOn.address, (socklen_t *)sizeof(serverOn.address));
					epollEvents.events = EPOLLIN;
					epollEvents.data.fd = newClientFd;
					std::cout << newClientFd << std::endl;
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
