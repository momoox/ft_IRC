/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgeisler <mgeisler@student.42mulhouse.f    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/22 17:13:01 by gloms             #+#    #+#             */
/*   Updated: 2025/03/18 18:00:44 by mgeisler         ###   ########.fr       */
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
		std::cerr << RED << "Wrong number of arguments. Should have a port and a password." << RESET << std::endl;
		return (1);
	}

	Server server(atoi(av[1]), av[2]);
	int nbEvents = 0;
	User* user;

	std::cout << PINK << "Server started :)" << RESET << std::endl;
	
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
				}
			}
		}
	}
	catch (const std::exception &e) {
		std::cerr << e.what() << std::endl;
		server.~Server();
	}
}
