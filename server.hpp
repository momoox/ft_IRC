/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgeisler <mgeisler@student.42mulhouse.f    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/09 19:33:39 by gloms             #+#    #+#             */
/*   Updated: 2025/02/17 17:02:04 by mgeisler         ###   ########.fr       */
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
#include "user.hpp"

#define MAX_EVENTS 10

class user;

class Server {

public :
	~Server();
	Server(int port, std::string password);
	Server(const Server &src);

	void parser(std::string buffer, int fd);
	void deleteUser(int clientFd);

	int serverFd;
	struct sockaddr_in address;
		
private :
	int _port;
	std::string _password;
	std::map<int, user *> _users;

	Server();
	const Server &operator = (const Server &rhs);

};