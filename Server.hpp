/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgeisler <mgeisler@student.42mulhouse.f    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/09 19:33:39 by gloms             #+#    #+#             */
/*   Updated: 2025/02/18 13:27:22 by mgeisler         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <errno.h>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <cstdlib>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <sys/epoll.h>
#include <map>
#include "User.hpp"

#define MAX_EVENTS 10

class User;

class Server {

public :
	~Server();
	Server(int port, std::string password);
	Server(const Server &src);

	void parser(std::string buffer, int clientFD);
	void deleteUser(int fd);

	int serverFd;
	struct sockaddr_in address;
		
private :
	int _port;
	std::string _password;
	std::map<std::string, User *> _users;

	Server();
	const Server &operator = (const Server &rhs);

};