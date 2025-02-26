/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgeisler <mgeisler@student.42mulhouse.f    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/22 17:13:01 by gloms             #+#    #+#             */
/*   Updated: 2025/02/26 15:35:07 by mgeisler         ###   ########.fr       */
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
			nbEvents = epoll_wait(server.getEpollFd(), server.newClient, 2, -1);
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
