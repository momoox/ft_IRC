/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgeisler <mgeisler@student.42mulhouse.f    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/07 17:36:12 by gloms             #+#    #+#             */
/*   Updated: 2025/02/13 14:34:25 by mgeisler         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.hpp"

Server::~Server() {
	close(serverFd);
	// close(epollFd);
}

Server::Server(int port, std::string password) {
	_port = port;
	_password = password;
	std::cout << port << std::endl;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port);
	serverFd = socket(AF_INET, SOCK_STREAM, 0);

	if (serverFd == -1) {
        perror("socket creation failed");
        throw std::runtime_error("socket creation failed");
    }

	int opt = 1;
    if (setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        throw std::runtime_error("setsockopt failed");
	}

	if (bind(serverFd, (struct sockaddr *)&address, (socklen_t)sizeof(address)) == -1) {
		perror ("bind");
		throw std::runtime_error("bind failed");
	}
	listen(serverFd, 10);
}

void Server::parser(std::string buffer) {
	if (buffer.find("CAP LS"))
		
		
}

void Server::deleteUser(int clientFd) {
	_users.erase(clientFd);
}

Server::Server(const Server& serv) {
	*this = serv;
}

const Server& Server::operator=(const Server &rhs) {
	(void)rhs;
	return *this;
}
