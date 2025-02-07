/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgeisler <mgeisler@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/29 10:33:08 by mgeisler          #+#    #+#             */
/*   Updated: 2025/02/06 16:43:21 by mgeisler         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <sys/socket.h>
#include <sys/epoll.h>
#include "servData.hpp"

//epoll fonctionne pas sur mac lol

int	main(int argc, char **argv) {
	if (argc < 3) {
		std::cout << "Wrong number of arguments. Please enter the port and password of the server you want to join." << std::endl;
		return (1);
	}
	
	std::string port(argv[1]);
	std::string password(argv[2]);

	Server data(port, password);

	std::cout << "data in class for port: " << data.getPort() << std::endl;
	std::cout << "data in class for password: " << data.getPassword() << std::endl;

	int epollFD = epoll_create1(0);
	if(epollFD < 0) {
		std::cerr << "epoll fd failed." << std::endl;
		return (1);
	}

	int socketServ = socket(AF_INET, SOCK_STREAM, 0);
	if (socketServ < 0) {
		std::cerr << "Error during socket creation." << std::endl;
		return (1);
	}

	std::cout << "socket: " << socketServ << std::endl;
	
	int opt = 1;

	if (setsockopt(socketServ, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
		std::cerr << "Error setting socket options." << std::endl;
		return (1);
	}

	

	return (0);
}