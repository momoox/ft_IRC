/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgeisler <mgeisler@student.42mulhouse.f    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/09 19:33:39 by gloms             #+#    #+#             */
/*   Updated: 2025/02/26 17:04:15 by mgeisler         ###   ########.fr       */
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
	
	/*CMD*/
	void	joinCmd(std::string buffer);
	void	inviteCmd(std::string buffer);
	void	kickCmd(std::string buffer);
	void	topicCmd(std::string buffer);
	void	modeCmd(std::string buffer);
	void	privmsgCmd(std::string buffer);
	void	passCmd(std::string buffer, int fd);
	void	nickCmd(std::string buffer, int fd);
	void	userCmd(std::string buffer, int fd);
	
	/*RUNTIME*/
	void acceptClient();
	void receiveMessageFromClient(int clientFd, User* user);
	void parserMessage(std::string message, int clientFd);
	void registerUser(std::string buffer, int fd);
	void deleteUser(int fd);
	void sendMessage(std::string message, int fd);
	// User* getUserFromFd(int fd);
	
	int getEpollFd() const;
	int getServerFd() const;
	socklen_t getAddrLen() const;
	
private :
int _port;
	std::string _password;
	std::map<int, User *> _users;
	int _serverFd;
	int _epollFd;
	socklen_t _addrLen;

	Server();
	Server(const Server &src);
	const Server &operator = (const Server &rhs);

};
