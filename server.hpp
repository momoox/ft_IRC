/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgeisler <mgeisler@student.42mulhouse.f    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/09 19:33:39 by gloms             #+#    #+#             */
/*   Updated: 2025/02/12 17:20:18 by mgeisler         ###   ########.fr       */
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
#include <list>
#include "user.hpp"

#define MAX_EVENTS 10

class user;

class Server {

public :
	~Server();
	Server(int port, std::string password);
	Server(const Server &src);

	void parser(std::string buffer);

	int serverFd;
	struct sockaddr_in address;
		
private :
	std::list<User *> _users;
	int _port;
	std::string _password;

	Server();
	const Server &operator = (const Server &rhs);

};