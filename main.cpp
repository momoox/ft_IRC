/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gloms <rbrendle@student.42mulhouse.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/22 17:13:01 by gloms             #+#    #+#             */
/*   Updated: 2025/02/22 20:13:48 by gloms            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

int main(int ac, char **av)
{
	if (ac != 3) {
		std::cerr << "Wrong number of arguments. Should have a port and a password." << std::endl;
		return (1);
	}

	Server server(atoi(av[1]), av[2]);
	int nbEvents = 0;

	try {
		while (1) {
			nbEvents = epoll_wait(server.epollFd, server.newClient, 2, -1);
			for (int i = 0; i < nbEvents; i++) {
				if (newClient[i].data.fd == server.serverFd) {
					server.acceptClient();
				}
				else {
					server.receiveMessageFromClient(server.newClient[i].data.fd);
					// handle text or commands
				}
			}
		}
	}
	catch {}
}
