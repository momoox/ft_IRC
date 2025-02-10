/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgeisler <mgeisler@student.42mulhouse.f    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/07 17:36:12 by gloms             #+#    #+#             */
/*   Updated: 2025/02/10 19:13:19 by mgeisler         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.hpp"

Server::~Server() {}

Server::Server(int port) {
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port);
	serverFd = socket(AF_INET, SOCK_STREAM, 0);

	int opt = 1;
    if (setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        throw std::runtime_error("setsockopt failed");
	}

	if (bind(serverFd, (struct sockaddr *)&address, (socklen_t)sizeof(address)) < 0) {
		throw std::runtime_error("bind failed");
	}
}

const Server& Server::operator=(const Server &rhs) {
	(void)rhs;
	return *this;
}
