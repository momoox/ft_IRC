/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gloms <rbrendle@student.42mulhouse.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/22 17:13:01 by gloms             #+#    #+#             */
/*   Updated: 2025/03/16 22:59:58 by gloms            ###   ########.fr       */
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
	User* user;
	try {
		while (1) {
			nbEvents = epoll_wait(server.getEpollFd(), server.newClient, MAX_EVENTS, -1);
			std::cout << "new event entry" << std::endl;
			for (int i = 0; i < nbEvents; i++) {
				user = (User*)server.newClient[i].data.ptr;
				if (server.newClient[i].data.fd == server.getServerFd()) {
					server.acceptClient();
				}
				else {
					server.receiveMessageFromClient(user->getFd(), user);
					// handle text or commands
				}
			}
		}
	}
	catch (const std::exception &e) {
		std::cerr << e.what() << std::endl;
	}
}
