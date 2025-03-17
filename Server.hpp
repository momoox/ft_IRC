/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gloms <rbrendle@student.42mulhouse.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/09 19:33:39 by gloms             #+#    #+#             */
/*   Updated: 2025/03/17 04:15:28 by gloms            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <csignal>
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
#include <set>
#include <netdb.h>
#include "User.hpp"
#include "Channel.hpp"
#include "RPL.hpp"

#define MAX_EVENTS 20

class User;
class Channel;

class Server {
private :
	std::string _ip;
	int _port;
	std::string _password;
	int _serverFd;
	int _epollFd;

	socklen_t _addrLen;

	std::map<int, User *> _users;
	std::map<std::string, Channel*> _channelInfos;

	Server();
	Server(const Server &src);
	const Server &operator = (const Server &rhs);

public :
	~Server();
	Server(int port, std::string password);


	/*INITIALISATION*/
	struct sockaddr_in address;
	struct epoll_event epollEvents;
	struct epoll_event newClient[MAX_EVENTS];

	/*CMD*/
	void	joinCmd(std::string buffer, int clientFd);
	void	inviteCmd(std::string buffer, int clientFd);
	void	kickCmd(std::string buffer, int clientFd);
	void	topicCmd(std::string buffer, int clientFd);
	void	modeCmd(std::string buffer, int clientFd);
	void	partCmd(std::string buffer, int clientFd);
	void	privmsgCmd(std::string buffer, int clientFd);
	void	passCmd(std::string buffer, int fd);
	void	nickCmd(std::string buffer, int fd);
	void	userCmd(std::string buffer, int fd);

	bool validNick(const std::string& nick);
	int getUserFromNick(std::string nickname) const;
	std::string findFittingNick(std::string buffer, int clientFd);
	std::string findFittingChan(std::string buffer, int clientFd);


	/*RUNTIME*/
	void acceptClient();
	void receiveMessageFromClient(int clientFd, User* user);
	void parserMessage(std::string message, int clientFd);
	void deleteUser(int fd);
	void sendMessage(std::string message, int fd);

	int getEpollFd() const;
	int getServerFd() const;
	socklen_t getAddrLen() const;


};
