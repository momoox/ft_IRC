#include "Server.hpp"

Server::~Server() {
	close(_serverFd);
}

Server::Server(int port, std::string password) : _port(port), _password(password)
{
	_serverFd = socket(AF_INET, SOCK_STREAM, 0);
	if (_serverFd == -1) {
		std::cerr << "Error creating socket: " << strerror(errno) << std::endl;
		exit(1);
	}

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port);

	int opt = 1;
    if (setsockopt(_serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
		std::cerr << "Error setting socket options: " << strerror(errno) << std::endl;
		exit(1);
	}

	if (bind(_serverFd, (struct sockaddr *)&address, sizeof(address)) == -1) {
		std::cerr << "Error binding socket: " << strerror(errno) << std::endl;
		exit(1);
	}

	if (listen(_serverFd, 10) == -1) {
		std::cerr << "Error listening on socket: " << strerror(errno) << std::endl;
		exit(1);
	}

	if (fcntl(_serverFd, F_SETFL, O_NONBLOCK) == -1) {
		std::cerr << "Error setting server to non-blocking: " << strerror(errno) << std::endl;
		exit(1);
	}

	_epollFd = epoll_create1(0);
	if (_epollFd == -1) {
		std::cerr << "Error creating epoll: " << strerror(errno) << std::endl;
		exit(1);
	}

	epollEvents.events = EPOLLIN;
	epollEvents.data.fd = _serverFd;
	if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, _serverFd, &epollEvents) == -1) {
		std::cerr << "Error adding server to epoll: " << strerror(errno) << std::endl;
		exit(1);
	}
}

int Server::getEpollFd() const {
	return (_epollFd);
}
int Server::getServerFd() const {
	return (_serverFd);
}

socklen_t Server::getAddrLen() const {
	return (_addrLen);
}

// void Server::registerUser(std::string buffer, int fd) {
// 	if (buffer.find("PASS") != std::string::npos) {
// 		std::size_t pos = buffer.find("PASS");
// 		std::string pwd = buffer.substr(pos + 5, buffer.find("\r\n") - 7);
// 		std::cout << "password: " << pwd << std::endl;

// 		if (pwd != _password) {
// 			deleteUser(fd);
// 		}
// 	}

// 	if (buffer.find("NICK") != std::string::npos) {
// 		nickCmd(buffer, fd);
// 	}

// 	if (buffer.find("USER") != std::string::npos) {
// 		std::size_t pos = buffer.find("USER");
// 		std::string fullname = buffer.substr(pos, buffer.find("\r\n") - 7);
// 		std::cout << "user: " << fullname << std::endl;
// 		_users.find(fd)->second->setFullName(fullname);
// 	}
// }

void Server::parserMessage(std::string message, int clientFd) {
	std::string cmd;
	std::string cmdArg;
	std::size_t pos = message.find("\r\n");
	std::string buffer = message.substr(0, pos + 2);
	// std::string nextBuffer;

	std::cout << "Buffer: " << buffer << std::endl;
	std::cout << "Message: " << message << std::endl;
	while (!buffer.empty()) {
		if (buffer.find("CAP LS") != std::string::npos) {
			// registerUser(buffer, clientFd);
			sendMessage(CAP_LS, clientFd);
		}
		
		else if (buffer.find("JOIN") != std::string::npos) {
			joinCmd(buffer);
		}
		
		else if (buffer.find("INVITE") != std::string::npos) {
			inviteCmd(buffer);
		}

		else if (buffer.find("KICK") != std::string::npos) {
			kickCmd(buffer);
		}

		else if (buffer.find("TOPIC") != std::string::npos) {
			topicCmd(buffer);
		}

		else if (buffer.find("MODE") != std::string::npos) {
			modeCmd(buffer);
		}

		else if (buffer.find("PRIVMSG") != std::string::npos) {
			privmsgCmd(buffer);
		}

		else if (buffer.find("PASS") != std::string::npos) {
			passCmd(buffer, clientFd);
		}

		else if (buffer.find("NICK") != std::string::npos) {
			nickCmd(buffer, clientFd);
		}

		else if (buffer.find("USER") != std::string::npos) {
			userCmd(buffer, clientFd);
		}
		else if (buffer.find("CAP END") != std::string::npos) {
			sendMessage(":localhost 001 rbrendle :Welcome\r\n", clientFd);
		}
		else if (buffer.find("PING") != std::string::npos) {
			sendMessage("PONG", clientFd);
		}

		else {
			sendMessage("cmd existe ap wsh", clientFd);
		}
		buffer = message.substr(pos + 2, message.find("\r\n", pos + 2) - (pos + 2));
		message = message.substr(pos + 2, message.size() - (pos + 2));
		pos = message.find("\r\n");
		std::cout << "buffer after substr: " << buffer << std::endl;
		std::cout << "message after substr: " << message << std::endl;
	}
}

void	Server::acceptClient() {
	struct epoll_event event;
	struct sockaddr_in client;
	socklen_t len = sizeof(client);

	int newClientFd = accept(_serverFd, (struct sockaddr *)&client, &len);

	if (newClientFd < 0)
		std::cerr << "Client FD failed: " << strerror(errno) << std::endl;

	User* user = new User(newClientFd);
	event.events = EPOLLIN;
	event.data.ptr = user;
	_users.insert(std::make_pair(newClientFd, user));
	epoll_ctl(_epollFd, EPOLL_CTL_ADD, newClientFd, &event);
}

void	Server::receiveMessageFromClient(int clientFd, User* user) {
	char buffer[1024];

	int readBytes = recv(clientFd, buffer, 1024, 0);

	if (readBytes < 0) {
		if (user) {
			deleteUser(user->getFd());
			epoll_ctl(_epollFd, EPOLL_CTL_DEL, clientFd, NULL);
		}
		else {
			close(clientFd);
			epoll_ctl(_epollFd, EPOLL_CTL_DEL, clientFd, NULL);
		}
		std::cerr << "client FD :" << clientFd << "Error while receiving client message, client disconnected." << std::endl;
	}

	// std::cout << "Buffer after receive: " << buffer << std::endl;

	if (readBytes > 0) {
        buffer[readBytes] = '\0'; // Null-terminate the buffer
        std::string buff_str = std::string(buffer);
        user->addToBuffer(buff_str);
    }

    if (readBytes >= 2 && strncmp(buffer + readBytes - 2, "\r\n", 2) == 0) {
        //user->addToBuffer(std::string(buffer));
        parserMessage(user->getBuffer(), clientFd);
        user->eraseBuffer();
    }

    else {
        std::string buff_str = std::string(buffer);
        user->addToBuffer(buff_str);
    }

	// std::cout << getUserFromFd(clientFd)->getBuffer() << std::endl;
}

void	Server::joinCmd(std::string buffer) {
	(void)buffer;
	std::cout << "Bien arrive dans JOIN :)" << std::endl;
}

void	Server::inviteCmd(std::string buffer) {
	(void)buffer;
	std::cout << "Bien arrive dans INVITE :)" << std::endl;
}

void	Server::kickCmd(std::string buffer) {
	(void)buffer;
	std::cout << "Bien arrive dans KICK :)" << std::endl;
}

void	Server::topicCmd(std::string buffer) {
	(void)buffer;
	std::cout << "Bien arrive dans TOPIC :)" << std::endl;
}

void	Server::modeCmd(std::string buffer) {
	(void)buffer;
	std::cout << "Bien arrive dans MODE :)" << std::endl;
}

void	Server::privmsgCmd(std::string buffer) {
	(void)buffer;
	std::cout << "Bien arrive dans PRIVMSG :)" << std::endl;
}

void Server::passCmd(std::string buffer, int fd) {
	std::size_t pos = buffer.find("PASS");
	std::string pwd = buffer.substr(pos + 5, buffer.find("\r\n") - (pos + 5));
	std::cout << "password: " << pwd << std::endl;
	std::cout << "server password: " << _password << std::endl;

	if (pwd != _password) {
		std::cout << "oh nooo" << std::endl;
		deleteUser(fd);
	}
}

void Server::nickCmd(std::string buffer, int fd) {
	std::size_t pos = buffer.find("NICK");
	std::string newNick = buffer.substr(pos + 5, buffer.find("\r\n") - (pos + 5));
	std::cout << "nick: " << newNick << std::endl;

	if (_users.find(fd)->second->validNick(newNick)) {
		_users.find(fd)->second->setNick(newNick);
	}
}

void Server::userCmd(std::string buffer, int fd) {
	std::size_t pos = buffer.find(":");
	std::string fullname = buffer.substr(pos + 1, buffer.find("\r\n") - (pos + 1));
	std::cout << "user: " << fullname << std::endl;
	_users.find(fd)->second->setFullName(fullname);
}

void Server::sendMessage(std::string message, int fd) {
    send(fd, message.c_str(), message.size(), 0);
}

void Server::deleteUser(int fd) {
	delete _users.find(fd)->second;
	_users.erase(fd);
}

// User* Server::getUserFromFd(int fd) {
// 	std::map<std::string, User*>::iterator it;
// 	std::string nick;

// 	for(it = _users.begin(); it != _users.end(); it++) {
// 		if (it->second->getFd() == fd)
// 			nick = it->second->getNick();
// 	}
// 	return it->second;
// }
