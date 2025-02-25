/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgeisler <mgeisler@student.42mulhouse.f    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/22 17:13:01 by gloms             #+#    #+#             */
/*   Updated: 2025/02/25 18:47:57 by mgeisler         ###   ########.fr       */
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
				std::cout << "i: " << i << std::endl;
				std::cout << "nbevents: " << nbEvents << " | .data.fd: " << server.newClient[i].data.fd << " | .data.ptr: " << server.newClient[i].data.ptr << std::endl; 
 				if (server.newClient[i].data.fd == server.serverFd) {
					std::cout << 'a' << std::endl;
					server.acceptClient(server.newClient[i]);
				}
				else {
					//envoyer newClient[i]
					std::cout << 'b' << std::endl;
					server.receiveMessageFromClient(server.newClient[i].data.fd, server.newClient[i]);
					// handle text or commands
				}
			}
		}
	}
	catch (const std::exception &e) {
		std::cerr << e.what() << std::endl;
	}
}
