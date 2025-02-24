/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gloms <rbrendle@student.42mulhouse.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/09 19:33:39 by gloms             #+#    #+#             */
/*   Updated: 2025/02/24 19:35:36 by gloms            ###   ########.fr       */
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
#include <fcntl.h>
#include <sstream>
#include "User.hpp"
#include "RPL.hpp"

#define MAX_EVENTS 10

class User;

class Server {

public :
	~Server();
	Server(int port, std::string password);


	/*INITIALISATION*/
	struct sockaddr_in address;
	struct epoll_event epollEvents;
	struct epoll_event newClient[MAX_EVENTS];
	int serverFd;
	int epollFd;
	socklen_t addrLen;

	/*CMD*/
	void	joinCmd(std::string buffer);
	void	inviteCmd(std::string buffer);
	void	kickCmd(std::string buffer);
	void	topicCmd(std::string buffer);
	void	modeCmd(std::string buffer);
	void	privmsgCmd(std::string buffer);

	/*RUNTIME*/
	void acceptClient();
	void receiveMessageFromClient(int clientFd);
	void parserMessage(const std::string &message, int clientFd);
	void deleteUser(int fd);
	void sendMessage(std::string message, int fd);
	User* getUserFromFd(int fd);


private :
	int _port;
	std::string _password;
	std::map<std::string, User *> _users;

	Server();
	Server(const Server &src);
	const Server &operator = (const Server &rhs);

};
