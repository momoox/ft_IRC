/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gloms <rbrendle@student.42mulhouse.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/07 17:36:12 by gloms             #+#    #+#             */
/*   Updated: 2025/02/10 18:00:17 by gloms            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.hpp"

Server::~Server() {}

Server::Server(int port) {
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port);
	serverFd = socket(AF_INET, SOCK_STREAM, 0);
	if (bind(serverFd, (struct sockaddr *)&address, sizeof(address)) < 0) {
		throw std::runtime_error("bind failed");
	}
}

const Server &Server::operator = (const Server &rhs) {
	(void)rhs;
	return *this;
}
