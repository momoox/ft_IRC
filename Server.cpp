/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gloms <rbrendle@student.42mulhouse.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/07 17:36:12 by gloms             #+#    #+#             */
/*   Updated: 2025/02/21 19:25:27 by gloms            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Server::~Server() {
	close(serverFd);
	// close(epollFd);
}

Server::Server(int port, std::string password) {
	_port = port;
	_password = password;
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

void Server::sendMessage(std::string message, int fd) {
    send(fd, message.c_str(), message.size(), 0);
}

void Server::parser(std::string buffer, int clientFD, struct epoll_event *events) {
	if (buffer.find("CAP LS") != std::string::npos) {
		User *newUser = new User();
		std::size_t pos = buffer.find("PASS");
		if (pos != std::string::npos) {
			std::size_t posOfPass = buffer.find('\r', pos + 5);
			std::string result = buffer.substr(pos + 5, posOfPass - (pos + 5));
			std::cout << "Password is " << result << std::endl;
			if (_password != result) {
				sendMessage(ERR_PASSWDMISMATCH(newUser->getFullName()), clientFD);
				delete newUser;
				//refuse access;
			}
			result.erase();
		}
		pos = buffer.find(":");
		if (pos != std::string::npos) {
			std::size_t posOfEndl = buffer.find('\r', pos);
			std::string result = buffer.substr(pos + 1, posOfEndl - (pos + 1));
			std::cout << "User is " << result << std::endl;
			try{
				newUser->setFullName(result);
			}
			catch (const std::invalid_argument& e) {
				std::cerr << e.what() << std::endl;
				delete newUser;
				return;
			}
			result.erase();
		}

		pos = buffer.find("NICK");
		if (pos != std::string::npos) {
			std::size_t posOfNick = buffer.find('\r', pos + 5);
			std::string result = buffer.substr(pos + 5, posOfNick - (pos + 5));
			std::cout << "Nick is " << result << std::endl;
			if (_users.find(result) != _users.end()) {
				send(clientFD, ERR_NICKNAMEINUSE(newUser->getFullName(), newUser->getNick()).c_str(), ERR_NICKNAMEINUSE(newUser->getFullName(), newUser->getNick()).size(), 0);
				std::cerr << "client :" << newUser->getFullName() << "Nickname : " << newUser->getNick() << "already in use, connection rejected" << std::endl;
				delete newUser;
				//return false
			}
			if (result.empty() || !newUser->validNick(result)) {
				send(clientFD, ERR_ERRONEUSNICKNAME(newUser->getFullName()).c_str(), ERR_ERRONEUSNICKNAME(newUser->getFullName()).size(), 0);
				std::cerr << "client :" << newUser->getFullName() << "Nickname : " << newUser->getNick() << "is empty, connection rejected" << std::endl;
				delete newUser;
				//return false
			}
			try {
				newUser->setNick(result);
			}
			catch (const std::invalid_argument& e) {
				std::cerr << e.what() << std::endl;
				delete newUser;
				return;
			}
			result.erase();
		}

		newUser->setFD(clientFD);
		_users.insert(std::make_pair(newUser->getNick(), newUser));
		events->data.ptr = newUser;
	}
}

void Server::deleteUser(int fd) {
	std::map<std::string, User*>::iterator it;
	std::string nick;

	for(it = _users.begin(); it != _users.end(); it++)
		if (it->second->getFd() == fd)
			nick = it->second->getNick();
	_users.erase(nick);
}

Server::Server(const Server& serv) {
	*this = serv;
}

const Server& Server::operator=(const Server &rhs) {
	(void)rhs;
	return *this;
}
