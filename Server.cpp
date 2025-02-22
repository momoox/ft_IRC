#include "Server.hpp"

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

void	Server::acceptClient() {
	int newClientFd = accept(serverOn.serverFd, (struct sockaddr *)&serverOn.address, &addrLen);

	if (newClientFd < 0)
		std::cerr << "Client FD failed: " << strerror(errno) << std::endl;

	epollEvents.events = EPOLLIN;
	epollEvents.data.fd = newClientFd;
	epoll_ctl(epollFd, EPOLL_CTL_ADD, newClientFd, &epollEvents);
}

void	Server::receiveMessageFromClient(int clientFd) {
	std::string buffer(1024, 0);

	int readBytes = recv(clientFd, &buffer[0], 1024, 0);

	if (readBytes < 0) {
		if (newClient->data.ptr) {
			deleteUser(newClient->data.fd)
			epoll_ctl(epollFd, EPOLL_CTL_DEL, clientFd, nullptr);
		}
		else {
			close(clientFd);
			epoll_ctl(epollFd, EPOLL_CTL_DEL, clientFd, nullptr);
		}
		std::cerr << "client FD :" << clientFd << "Error while receiving client message, client disconnected." << std::endl;
	}
	message()
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
	delete _users.find(nick)->second();
	_users.erase(nick);
}
