#include "Server.hpp"

Server::~Server() {
	close(serverFd);
}

Server::Server(int port, std::string password) : _port(port), _password(password)
{
	serverFd = socket(AF_INET, SOCK_STREAM, 0);
	if (serverFd == -1) {
		std::cerr << "Error creating socket: " << strerror(errno) << std::endl;
		exit(1);
	}

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port);

	int opt = 1;
    if (setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
		std::cerr << "Error setting socket options: " << strerror(errno) << std::endl;
		exit(1);
	}

	if (bind(serverFd, (struct sockaddr *)&address, sizeof(address)) == -1) {
		std::cerr << "Error binding socket: " << strerror(errno) << std::endl;
		exit(1);
	}

	if (listen(serverFd, 10) == -1) {
		std::cerr << "Error listening on socket: " << strerror(errno) << std::endl;
		exit(1);
	}

	if (fcntl(serverFd, F_SETFL, O_NONBLOCK) == -1) {
		std::cerr << "Error setting server to non-blocking: " << strerror(errno) << std::endl;
		exit(1);
	}

	epollFd = epoll_create1(0);
	if (epollFd == -1) {
		std::cerr << "Error creating epoll: " << strerror(errno) << std::endl;
		exit(1);
	}

	epollEvents.events = EPOLLIN;
	epollEvents.data.fd = serverFd;
	if (epoll_ctl(epollFd, EPOLL_CTL_ADD, serverFd, &epollEvents) == -1) {
		std::cerr << "Error adding server to epoll: " << strerror(errno) << std::endl;
		exit(1);
	}
}

void Server::parserMessage(const std::string &message, int clientFd) {
	std::string cmd;
	std::string cmdArg;
	std::size_t pos = message.find("\r\n");
	std::string buffer = message.substr(0, pos + 4);

	while (!buffer.empty()) {
		if (message.find("CAP")) {

			//creer la map et l'objet user et store les infos user
			sendMessage(":localhost 001 rbrendle :Welcome\r\n", clientFd);
		}

		else if (message.find("NICK")) {
			
		}

		else if (message.find("JOIN")) {
			joinCmd(buffer);
		}

		else if (message.find("INVITE")) {
			inviteCmd(buffer);
		}

		else if (message.find("KICK")) {
			kickCmd(buffer);
		}

		else if (message.find("TOPIC")) {
			topicCmd(buffer);
		}

		else if (message.find("MODE")) {
			modeCmd(buffer);
		}

		else if (message.find("PRIVMSG")) {
			privmsgCmd(buffer);
		}

		else {
			sendMessage("cmd existe ap wsh", clientFd);
		}
		//buffer
	}
	// std::istringstream iss(message);

	// iss >> cmd;

}

void	Server::acceptClient(struct epoll_event &client) {
	int newClientFd = accept(serverFd, (struct sockaddr *)&address, &addrLen);

	if (newClientFd < 0)
		std::cerr << "Client FD failed: " << strerror(errno) << std::endl;

	User* newClient = new User(newClientFd);
	client.data.ptr = (void *)newClient;
	epollEvents.events = EPOLLIN;
	epollEvents.data.fd = newClientFd;
	epoll_ctl(epollFd, EPOLL_CTL_ADD, newClientFd, &epollEvents);
	std::cout << "newClient: " << newClient << " (" << newClientFd << ")" << std::endl;
	std::cout << "data.ptr: " << client.data.ptr << std::endl;
}

void	Server::receiveMessageFromClient(int clientFd, struct epoll_event &client) {
	char buffer[1024];

	int readBytes = recv(clientFd, &buffer[0], 1024, 0);

	if (readBytes < 0) {
		if (client.data.ptr) {
			deleteUser(client.data.fd);
			epoll_ctl(epollFd, EPOLL_CTL_DEL, clientFd, NULL);
		}
		else {
			close(clientFd);
			epoll_ctl(epollFd, EPOLL_CTL_DEL, clientFd, NULL);
		}
		std::cerr << "client FD :" << clientFd << "Error while receiving client message, client disconnected." << std::endl;
	}
	User* user = static_cast<User*>(client.data.ptr);
	if (!user) {
        std::cerr << "Error: user pointer is null" << std::endl;
        return;
    }
	std::cout << user << std::endl;
	std::cout << "2" << client.data.ptr;

	if (readBytes > 0) {
        buffer[readBytes] = '\0'; // Null-terminate the buffer
		std::cout << "teeeeesssst" << std::endl;
		std::string buff_str = std::string(buffer);
        user->addToBuffer(buff_str);
    }
	
	if (readBytes >= 2 && strncmp(buffer + readBytes - 2, "\r\n", 2) == 0) {
		// user->addToBuffer(std::string(buffer));
		parserMessage(user->getBuffer(), clientFd);
		user->eraseBuffer();
	}
	else {
		std::string buff_str = std::string(buffer);
		user->addToBuffer(buff_str);
		perror("error:");
	}
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

void Server::sendMessage(std::string message, int fd) {
    send(fd, message.c_str(), message.size(), 0);
}

void Server::deleteUser(int fd) {
	std::map<std::string, User*>::iterator it;
	std::string nick;

	for(it = _users.begin(); it != _users.end(); it++) {
		if (it->second->getFd() == fd)
			nick = it->second->getNick();
	}
	delete _users.find(nick)->second;
	_users.erase(nick);
}

User* Server::getUserFromFd(int fd) {
	std::map<std::string, User*>::iterator it;
	std::string nick;

	for(it = _users.begin(); it != _users.end(); it++) {
		if (it->second->getFd() == fd)
			nick = it->second->getNick();
	}
	return it->second;
}
