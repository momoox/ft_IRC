/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gloms <rbrendle@student.42mulhouse.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/09 19:33:39 by gloms             #+#    #+#             */
/*   Updated: 2025/02/21 19:22:46 by gloms            ###   ########.fr       */
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
#include "RPL.hpp"

#define MAX_EVENTS 10

class User;

class Server {

public :
	~Server();
	Server(int port, std::string password);
	Server(const Server &src);

	void parser(std::string buffer, int clientFD, struct epoll_event *events);
	void deleteUser(int fd);
	void sendMessage(std::string message, int fd);

	int serverFd;
	struct sockaddr_in address;

private :
	int _port;
	std::string _password;
	std::map<std::string, User *> _users;

	Server();
	const Server &operator = (const Server &rhs);

};
