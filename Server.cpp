/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgeisler <mgeisler@student.42mulhouse.f    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/07 17:36:12 by gloms             #+#    #+#             */
/*   Updated: 2025/02/22 01:44:29 by mgeisler         ###   ########.fr       */
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

//faut stocker un objet dans data.ptr et tout ca la

void Server::parser(std::string buffer, int clientFD) {
	if (buffer.find("CAP LS") != std::string::npos) {
		User *newUser = new User();
		std::size_t pos;
		if ((pos = buffer.find("NICK")) && (pos != std::string::npos)) {
			std::size_t posOfNick = buffer.find('\r', pos + 5);
			std::string result = buffer.substr(pos + 5, posOfNick - (pos + 5));
			std::cout << "Nick is " << result << std::endl;
			//checker que le nickname ne soit pas deja existant pour un autre user
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

		//repond au CAP LS du client permettant la suite de la connection
		std::string reply = ":" + std::string(SERVER_NAME) + " CAP " + newUser->getNick() + " ACK :multi-prefix\r\n";
		send(clientFD, reply.c_str(), reply.length(), 0);

		if ((pos = buffer.find("PASS")) && (pos != std::string::npos)) {
			std::size_t posOfPass = buffer.find('\r', pos + 5);
			std::string result = buffer.substr(pos + 5, posOfPass - (pos + 5));
			std::cout << "Password is " << result << std::endl;
			if (_password != result) {
				delete newUser;
				//refuse access;
			}
			result.erase();
		}
		
		if ((pos = buffer.find(":")) && (pos != std::string::npos)) {
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
		newUser->setFD(clientFD);
		_users.insert(std::make_pair(newUser->getNick(), newUser));
	}
	else {
		std::cout << "msg maybe received in multiple parts" << std::endl;
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
