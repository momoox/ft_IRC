/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgeisler <mgeisler@student.42mulhouse.f    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/22 17:13:01 by gloms             #+#    #+#             */
/*   Updated: 2025/03/17 19:39:00 by mgeisler         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

void signal_hand(int signum)
{
    (void)signum;
    throw std::logic_error(" \033[1;31m\nNeed to stop server\033[0m");
}

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
		signal(SIGINT, signal_hand);
		while (1) {
			nbEvents = epoll_wait(server.getEpollFd(), server.newClient, MAX_EVENTS, -1);
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
		server.~Server();
	}
}
