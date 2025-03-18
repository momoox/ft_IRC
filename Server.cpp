#include "Server.hpp"

Server::~Server() {

	std::map<int, User *>::iterator it = _users.begin();

	while (it != _users.end()) {
		sendMessage("\033[38;5;219mServer is shutting down. Goodbye :(\r\n\033[0m", it->first);
		it++;
	}

	close(_serverFd);
	close(_epollFd);

	for (std::map<int, User *>::iterator it = _users.begin(); it != _users.end();) {

		delete it->second;
		_users.erase(it);
		it = _users.begin();
	}

	for (std::map<std::string, Channel *>::iterator it = _channelInfos.begin(); it != _channelInfos.end();) {

		delete it->second;
		_channelInfos.erase(it);
		it = _channelInfos.begin();
	}

}

Server::Server(int port, std::string password) : _port(port), _password(password)
{
	_serverFd = socket(AF_INET, SOCK_STREAM, 0);

	if (_serverFd == -1) {

		std::cerr << RED << "Error creating socket: " << strerror(errno) << RESET << std::endl;
		exit(1);
	}

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port);

	char ip[100];
	gethostname(ip, sizeof(ip));
	std::string IP = inet_ntoa(*((struct in_addr *)(gethostbyname(ip))->h_addr_list[0]));
    _ip = IP;

	int opt = 1;
    if (setsockopt(_serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {

		std::cerr << RED << "Error setting socket options: " << strerror(errno) << RESET << std::endl;
		exit(1);
	}

	if (bind(_serverFd, (struct sockaddr *)&address, sizeof(address)) == -1) {

		std::cerr << RED << "Error binding socket: " << strerror(errno) << RESET << std::endl;
		exit(1);
	}

	if (listen(_serverFd, 10) == -1) {

		std::cerr << RED << "Error listening on socket: " << strerror(errno) << RESET << std::endl;
		exit(1);
	}

	if (fcntl(_serverFd, F_SETFL, O_NONBLOCK) == -1) {

		std::cerr << RED << "Error setting server to non-blocking: " << strerror(errno) << RESET << std::endl;
		exit(1);
	}

	_epollFd = epoll_create1(0);

	if (_epollFd == -1) {

		std::cerr << RED << "Error creating epoll: " << strerror(errno) << RESET << std::endl;
		exit(1);
	}

	epollEvents.events = EPOLLIN;
	epollEvents.data.fd = _serverFd;

	if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, _serverFd, &epollEvents) == -1) {

		std::cerr << RED << "Error adding server to epoll: " << strerror(errno) << RESET << std::endl;
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

void Server::parserMessage(std::string message, int clientFd) {
	std::string cmd;
	std::string cmdArg;
	std::size_t pos = message.find("\r\n");
	std::string buffer = message.substr(0, pos + 2);

	while (!buffer.empty()) {

		if (buffer.find("CAP LS") != std::string::npos) {

			sendMessage(CAP_LS, clientFd);
		}

		else if (buffer.find("PRIVMSG") != std::string::npos && _users[clientFd]->getIsRegistered() == true) {

			privmsgCmd(buffer, clientFd);
		}

		else if (buffer.find("JOIN") != std::string::npos && _users[clientFd]->getIsRegistered() == true) {

			joinCmd(buffer, clientFd);
		}

		else if (buffer.find("INVITE") != std::string::npos && _users[clientFd]->getIsRegistered() == true) {

			inviteCmd(buffer, clientFd);
		}

		else if (buffer.find("KICK") != std::string::npos && _users[clientFd]->getIsRegistered() == true) {

			kickCmd(buffer, clientFd);
		}

		else if (buffer.find("TOPIC") != std::string::npos && _users[clientFd]->getIsRegistered() == true) {

			topicCmd(buffer, clientFd);
		}

		else if (buffer.find("MODE") != std::string::npos) {

			modeCmd(buffer, clientFd);
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

		else if (buffer.find("PING") != std::string::npos) {

			std::string msg ="PONG " + _ip + " " + "localhost" + "\r\n";
			sendMessage(msg, clientFd);
		}

		else if (buffer.find("PART") != std::string::npos && _users[clientFd]->getIsRegistered() == true) {

			partCmd(buffer, clientFd);
		}

		else if (buffer.find("QUIT") != std::string::npos && _users[clientFd]->getIsRegistered() == true) {

			_users[clientFd]->setDead(true);
		}

		else if (buffer.find("WHOIS") != std::string::npos) {

			std::string nick = buffer.substr(buffer.find(" ") + 1, buffer.find("\r\n"));
			std::string msg = _users.find(clientFd)->second->getNick() + " :End of /WHOIS list" + "\r\n";
			sendMessage(msg, clientFd);
		}

		else if (_users[clientFd]->getIsRegistered() == false) {

			sendMessage(NEEDPASS, clientFd);
		}

		else {

			std::string cmd = buffer.substr(0, buffer.find(" "));
			sendMessage(ERR_UNKNOWNCOMMAND(_users.find(clientFd)->second->getNick(), cmd), clientFd);
		}

		buffer = message.substr(pos + 2, message.find("\r\n", pos + 2) - (pos + 2));
		message = message.substr(pos + 2, message.size() - (pos + 2));
		pos = message.find("\r\n");
	}
}

void	Server::acceptClient() {

	struct epoll_event event;
	struct sockaddr_in client;
	socklen_t len = sizeof(client);

	int newClientFd = accept(_serverFd, (struct sockaddr *)&client, &len);

	if (newClientFd < 0)
		std::cerr << RED << "Client FD failed: " << strerror(errno) << RESET << std::endl;

	User* user = new User(newClientFd);

	event.events = EPOLLIN;
	event.data.ptr = user;

	_users.insert(std::make_pair(newClientFd, user));

	epoll_ctl(_epollFd, EPOLL_CTL_ADD, newClientFd, &event);
}

void	Server::receiveMessageFromClient(int clientFd, User* user) {

	char buffer[1025] = {0};

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

		std::cerr << "client FD :" << clientFd << RED << "Error while receiving client message, client disconnected." << RESET << std::endl;
	}

	if (_users.find(clientFd)->second->getIsDead() == true) {

		deleteUser(clientFd);
		return ;
	}

	if (readBytes > 0) {

        std::string buff_str = std::string(buffer);
        user->addToBuffer(buff_str);
    }

	if (user->getBuffer().find("\n") != std::string::npos) {

		user->eraseEnterInBuffer();
		user->addToBuffer("\r\n");
	}

    if (readBytes >= 2 && user->getBuffer().find("\r\n") != std::string::npos) {

        parserMessage(user->getBuffer(), clientFd);
		if (user)
        	user->eraseBuffer();
    }
}


/*
────────────────────────────────────────────────────────────────────
─────────██████──██████████████──██████████──██████──────────██████─
─────────██░░██──██░░░░░░░░░░██──██░░░░░░██──██░░██████████──██░░██─
─────────██░░██──██░░██████░░██──████░░████──██░░░░░░░░░░██──██░░██─
─────────██░░██──██░░██──██░░██────██░░██────██░░██████░░██──██░░██─
─────────██░░██──██░░██──██░░██────██░░██────██░░██──██░░██──██░░██─
─────────██░░██──██░░██──██░░██────██░░██────██░░██──██░░██──██░░██─
─██████──██░░██──██░░██──██░░██────██░░██────██░░██──██░░██──██░░██─
─██░░██──██░░██──██░░██──██░░██────██░░██────██░░██──██░░██████░░██─
─██░░██████░░██──██░░██████░░██──████░░████──██░░██──██░░░░░░░░░░██─
─██░░░░░░░░░░██──██░░░░░░░░░░██──██░░░░░░██──██░░██──██████████░░██─
─██████████████──██████████████──██████████──██████──────────██████─
────────────────────────────────────────────────────────────────────
*/

void	Server::joinCmd(std::string buffer, int clientFd) {

	if (buffer.find("JOIN #") != std::string::npos) {

		std::size_t pos = 5;
		std::string channel = buffer.substr(pos, (buffer.find(" ", pos) != std::string::npos ? buffer.find(" ", pos) - pos : buffer.find("\r\n") - pos));


		if (_users.find(clientFd)->second->getChannelName() != channel && _channelInfos.find(channel) == _channelInfos.end()) {

			Channel* chan = new Channel(channel);

			_channelInfos.insert(std::make_pair(channel, chan));
			chan->setMapUsers(clientFd, _users.find(clientFd)->second);

			if (_users[clientFd]->getChannelName() != "default") {

				_channelInfos[_users[clientFd]->getChannelName()]->eraseUserInChannel(clientFd);
				_channelInfos[_users[clientFd]->getChannelName()]->setCurrentUsers("-");

				// if (_channelInfos[channel]->getCurrentUsers() == 0) {
				// 	//erase le channel
				// 	_channelInfos[_users[clientFd]->getChannelName()]->~Channel();
				// 	_channelInfos.erase(_users[clientFd]->getChannelName());
				// }

			}

			_users.find(clientFd)->second->setChannelName(channel);

			if (chan->getCurrentUsers() == 0) {
				_users.find(clientFd)->second->setIsOp(true);
			}

			chan->setCurrentUsers("+");
			sendMessage(JOIN(_users[clientFd]->getNick(), _users[clientFd]->getFullName(), channel), clientFd);
			sendMessage(RPL_NOTOPIC(_users[clientFd]->getNick(), channel), clientFd);
			sendMessage(RPL_NAMREPLY(_users[clientFd]->getNick(), channel) + _channelInfos[channel]->allUsersInChannel() + "\r\n" , clientFd);
			sendMessage(":localhost 366 " + _users[clientFd]->getNick() + " " + channel + " :End of /NAMES list\r\n", clientFd);
			sendMessage(RPL_CHANNELMODEIS(_users[clientFd]->getNick(), channel, "+" + _channelInfos[channel]->allModesInChannel() + "\r\n"), clientFd);
		}

		else if (_users.find(clientFd)->second->getChannelName() == channel) {

			sendMessage(ERR_USERONCHANNEL(_users.find(clientFd)->second->getNick(), channel), clientFd);
		}


		else {
			
			if ((_channelInfos[channel]->getPasswordChannel() == "" || buffer.find(_channelInfos[channel]->getPasswordChannel()) != std::string::npos) && ((_channelInfos.find(channel)->second->hasLimitedUsers() && _channelInfos.find(channel)->second->getCurrentUsers() < _channelInfos.find(channel)->second->getlimitUsers() && _channelInfos.find(channel)->second->getlimitUsers() != _channelInfos.find(channel)->second->getCurrentUsers()) || _channelInfos.find(channel)->second->hasLimitedUsers() == false)) {
				
				if (_channelInfos.find(channel)->second->getInviteMode() == false) {

					_channelInfos.find(channel)->second->setCurrentUsers("+");
					_channelInfos.find(channel)->second->setMapUsers(clientFd, _users.find(clientFd)->second);

					if (_users[clientFd]->getChannelName() != "default") {

						_channelInfos[_users[clientFd]->getChannelName()]->eraseUserInChannel(clientFd);
						_channelInfos[_users[clientFd]->getChannelName()]->setCurrentUsers("-");

						// if (_channelInfos[channel]->getCurrentUsers() == 0) {
						// 	//erase le channel
						// 	_channelInfos[_users[clientFd]->getChannelName()]->~Channel();
						// 	_channelInfos.erase(_users[clientFd]->getChannelName());
						// }

					}
					_users.find(clientFd)->second->setChannelName(channel);
					_channelInfos.find(channel)->second->sendAllUsers(JOIN(_users[clientFd]->getNick(), _users[clientFd]->getFullName(), channel), 0);
					sendMessage(RPL_NAMREPLY(_users[clientFd]->getNick(), channel) + _channelInfos[channel]->allUsersInChannel() + "\r\n" , clientFd);
					sendMessage(":localhost 366 " + _users[clientFd]->getNick() + " " + channel + " :End of /NAMES list\r\n", clientFd);
					sendMessage(RPL_CHANNELMODEIS(_users[clientFd]->getNick(), channel, + "+" +_channelInfos[channel]->allModesInChannel() + "\r\n"), clientFd);
				}

				else if (_channelInfos.find(channel)->second->getInviteMode() == true && _users.find(clientFd)->second->getIsInvited(channel) == true) {

					_channelInfos.find(channel)->second->setCurrentUsers("+");
					_channelInfos.find(channel)->second->setMapUsers(clientFd, _users.find(clientFd)->second);

					if (_users[clientFd]->getChannelName() != "default") {
						_channelInfos[_users[clientFd]->getChannelName()]->eraseUserInChannel(clientFd);
						_channelInfos[_users[clientFd]->getChannelName()]->setCurrentUsers("-");

						// if (_channelInfos[channel]->getCurrentUsers() == 0) {

						// 	_channelInfos[_users[clientFd]->getChannelName()]->~Channel();
						// 	_channelInfos.erase(_users[clientFd]->getChannelName());

						// }

					}
					_users.find(clientFd)->second->setChannelName(channel);
					_channelInfos.find(channel)->second->sendAllUsers(JOIN(_users[clientFd]->getNick(), _users[clientFd]->getFullName(), channel), 0);
					sendMessage(RPL_NAMREPLY(_users[clientFd]->getNick(), channel) + _channelInfos[channel]->allUsersInChannel() + "\r\n" , clientFd);
					sendMessage(":localhost 366 " + _users[clientFd]->getNick() + " " + channel + " :End of /NAMES list\r\n", clientFd);
				}

				else if (_channelInfos.find(channel)->second->getInviteMode() == true && _users.find(clientFd)->second->getIsInvited(channel) == false) {

					sendMessage(ERR_INVITEONLYCHAN(_users.find(clientFd)->second->getFullName(), channel), clientFd);
				}
			}

			else if (_channelInfos[channel]->getPasswordChannel() != "" && buffer.find(_channelInfos[channel]->getPasswordChannel()) == std::string::npos) {

				sendMessage(ERR_BADCHANNELKEY(_users.find(clientFd)->second->getFullName(), channel), clientFd);
				return ;
			}

			else if (_channelInfos.find(channel)->second->getlimitUsers() >= _channelInfos.find(channel)->second->getCurrentUsers()) {
				sendMessage(ERR_CHANNELISFULL(_users.find(clientFd)->second->getFullName(), channel), clientFd);
			}

			if (_channelInfos.find(channel)->second->getTopic().size() > 0) {
				sendMessage(RPL_TOPIC(_users.find(clientFd)->second->getNick(), channel, _channelInfos.find(channel)->second->getTopic()), clientFd);
			}

			else {
				sendMessage(RPL_NOTOPIC(_users.find(clientFd)->second->getNick(), channel), clientFd);
			}
		}
	}
}

/*
────────────────────────────────────────────────────────────────────────────────────────────────
─██████████──██████──────────██████──██████──██████──██████████──██████████████──██████████████─
─██░░░░░░██──██░░██████████──██░░██──██░░██──██░░██──██░░░░░░██──██░░░░░░░░░░██──██░░░░░░░░░░██─
─████░░████──██░░░░░░░░░░██──██░░██──██░░██──██░░██──████░░████──██████░░██████──██░░██████████─
───██░░██────██░░██████░░██──██░░██──██░░██──██░░██────██░░██────────██░░██──────██░░██─────────
───██░░██────██░░██──██░░██──██░░██──██░░██──██░░██────██░░██────────██░░██──────██░░██████████─
───██░░██────██░░██──██░░██──██░░██──██░░██──██░░██────██░░██────────██░░██──────██░░░░░░░░░░██─
───██░░██────██░░██──██░░██──██░░██──██░░██──██░░██────██░░██────────██░░██──────██░░██████████─
───██░░██────██░░██──██░░██████░░██──██░░░░██░░░░██────██░░██────────██░░██──────██░░██─────────
─████░░████──██░░██──██░░░░░░░░░░██──████░░░░░░████──████░░████──────██░░██──────██░░██████████─
─██░░░░░░██──██░░██──██████████░░██────████░░████────██░░░░░░██──────██░░██──────██░░░░░░░░░░██─
─██████████──██████──────────██████──────██████──────██████████──────██████──────██████████████─
────────────────────────────────────────────────────────────────────────────────────────────────
*/


void	Server::inviteCmd(std::string buffer, int clientFd) {

	int targetFd;
	std::string channel = findFittingChan(buffer, clientFd);
	std::string nickname = findFittingNick(buffer, clientFd);

	if (channel == " " || nickname == " ") {
		return ;
	}

	targetFd = getUserFromNick(nickname);

	if (targetFd < 0) {

		sendMessage(ERR_NOSUCHCHANNEL(_users.find(clientFd)->second->getNick(), nickname), clientFd);
		return ;
	}

	if (_channelInfos.find(channel) != _channelInfos.end()) {

		if (_users.find(clientFd)->second->getChannelName() != channel) {

			sendMessage(ERR_NOTONCHANNEL(_users.find(clientFd)->second->getNick(), channel), clientFd);
		}

		else if (_users.find(clientFd)->second->getIsOp() == false) {

			sendMessage(ERR_CHANOPRIVSNEEDED(_users.find(clientFd)->second->getNick(), channel), clientFd);
			return ;
		}

		sendMessage(RPL_INVITING(_users.find(clientFd)->second->getNick(), nickname, channel), targetFd);
		_users.find(targetFd)->second->setInvited(channel);
	}

	else {

		sendMessage(ERR_NOSUCHCHANNEL(_users.find(clientFd)->second->getNick(), channel), clientFd);
	}
}

/*
────────────────────────────────────────────────────────────────
─██████──████████──██████████──██████████████──██████──████████─
─██░░██──██░░░░██──██░░░░░░██──██░░░░░░░░░░██──██░░██──██░░░░██─
─██░░██──██░░████──████░░████──██░░██████████──██░░██──██░░████─
─██░░██──██░░██──────██░░██────██░░██──────────██░░██──██░░██───
─██░░██████░░██──────██░░██────██░░██──────────██░░██████░░██───
─██░░░░░░░░░░██──────██░░██────██░░██──────────██░░░░░░░░░░██───
─██░░██████░░██──────██░░██────██░░██──────────██░░██████░░██───
─██░░██──██░░██──────██░░██────██░░██──────────██░░██──██░░██───
─██░░██──██░░████──████░░████──██░░██████████──██░░██──██░░████─
─██░░██──██░░░░██──██░░░░░░██──██░░░░░░░░░░██──██░░██──██░░░░██─
─██████──████████──██████████──██████████████──██████──████████─
────────────────────────────────────────────────────────────────
*/

void	Server::kickCmd(std::string buffer, int clientFd) {

	int targetFd;

	std::string channel = findFittingChan(buffer, clientFd);
	std::string userToKick = findFittingNick(buffer, clientFd);

	if (channel == " " | userToKick == " ") {
		return ;
	}

	if (_users.find(clientFd)->second->getIsOp() == true) {

		if (channel.find("#") == std::string::npos) {

			sendMessage(ERR_BADCHANMASK(_users.find(clientFd)->second->getNick(), ("#" + channel)), clientFd);
		}

		else if ((targetFd = getUserFromNick(userToKick)) == 0) {

			sendMessage(ERR_NOSUCHNICK(_users.find(clientFd)->second->getNick(), userToKick), clientFd);
		}

		else if (_users.find(targetFd)->second->getChannelName() != channel) {

			sendMessage(ERR_USERNOTINCHANNEL(_users.find(clientFd)->second->getNick(), userToKick, channel), clientFd);
		}

		else {

			_channelInfos.find(channel)->second->sendAllUsers(KICK(_users.find(clientFd)->second->getNick(), _users.find(targetFd)->second->getNick(), channel, ""), 0);
			_channelInfos.find(channel)->second->kickUserFromChannel(targetFd);
			_channelInfos[_users[clientFd]->getChannelName()]->setCurrentUsers("-");
			_users.find(targetFd)->second->setChannelName("default");
			_users.find(targetFd)->second->setIsOp(false);
			_users.find(targetFd)->second->removeChannelInvite(channel);
			sendMessage(KICK(_users.find(clientFd)->second->getNick(), _users.find(targetFd)->second->getNick(), channel, ""), 0);
		}
	}

	else {

		sendMessage(ERR_CHANOPRIVSNEEDED(_users.find(clientFd)->second->getNick(), channel), clientFd);
	}
}

/*
────────────────────────────────────────────────────────────────────────────
─██████████████──██████████████──██████████████──██████████──██████████████─
─██░░░░░░░░░░██──██░░░░░░░░░░██──██░░░░░░░░░░██──██░░░░░░██──██░░░░░░░░░░██─
─██████░░██████──██░░██████░░██──██░░██████░░██──████░░████──██░░██████████─
─────██░░██──────██░░██──██░░██──██░░██──██░░██────██░░██────██░░██─────────
─────██░░██──────██░░██──██░░██──██░░██████░░██────██░░██────██░░██─────────
─────██░░██──────██░░██──██░░██──██░░░░░░░░░░██────██░░██────██░░██─────────
─────██░░██──────██░░██──██░░██──██░░██████████────██░░██────██░░██─────────
─────██░░██──────██░░██──██░░██──██░░██────────────██░░██────██░░██─────────
─────██░░██──────██░░██████░░██──██░░██──────────████░░████──██░░██████████─
─────██░░██──────██░░░░░░░░░░██──██░░██──────────██░░░░░░██──██░░░░░░░░░░██─
─────██████──────██████████████──██████──────────██████████──██████████████─
────────────────────────────────────────────────────────────────────────────
*/

void	Server::topicCmd(std::string buffer, int clientFd) {

	std::size_t pos = 6;
	std::string channel = findFittingChan(buffer, clientFd);

	if (channel == " ") {
		return ;
	}

	if ((_users.find(clientFd)->second->getIsOp() == true && channel == _users.find(clientFd)->second->getChannelName()) || _channelInfos.find(channel)->second->getTopicMode() == false) {

		if (buffer.find(": ") != std::string::npos) {

			_channelInfos.find(channel)->second->setTopic("");
			_channelInfos.find(channel)->second->sendAllUsers(RPL_TOPIC(_users.find(clientFd)->second->getNick(), channel, _channelInfos.find(channel)->second->getTopic()), 0);
			sendMessage(RPL_NOTOPIC(_users.find(clientFd)->second->getNick(), channel), clientFd);
		}

		else if (buffer.find(":") != std::string::npos) {

			std::string topic = buffer.substr(pos + channel.size() + 2, buffer.find("\r\n") - pos);
			_channelInfos.find(channel)->second->setTopic(topic);
			_channelInfos.find(channel)->second->sendAllUsers(RPL_TOPIC(_users.find(clientFd)->second->getNick(), channel, topic), 0);
		}

		else {

			_channelInfos.find(channel)->second->getTopic();
			_channelInfos.find(channel)->second->sendAllUsers(RPL_TOPIC(_users.find(clientFd)->second->getNick(), channel, _channelInfos.find(channel)->second->getTopic()), 0);
		}
	}

	else {

		sendMessage(ERR_NOSUCHCHANNEL(_users.find(clientFd)->second->getNick(), ""), clientFd);
	}
}

/*
────────────────────────────────────────────────────────────────────────
─██████──────────██████──██████████████──████████████────██████████████─
─██░░██████████████░░██──██░░░░░░░░░░██──██░░░░░░░░████──██░░░░░░░░░░██─
─██░░░░░░░░░░░░░░░░░░██──██░░██████░░██──██░░████░░░░██──██░░██████████─
─██░░██████░░██████░░██──██░░██──██░░██──██░░██──██░░██──██░░██─────────
─██░░██──██░░██──██░░██──██░░██──██░░██──██░░██──██░░██──██░░██████████─
─██░░██──██░░██──██░░██──██░░██──██░░██──██░░██──██░░██──██░░░░░░░░░░██─
─██░░██──██████──██░░██──██░░██──██░░██──██░░██──██░░██──██░░██████████─
─██░░██──────────██░░██──██░░██──██░░██──██░░██──██░░██──██░░██─────────
─██░░██──────────██░░██──██░░██████░░██──██░░████░░░░██──██░░██████████─
─██░░██──────────██░░██──██░░░░░░░░░░██──██░░░░░░░░████──██░░░░░░░░░░██─
─██████──────────██████──██████████████──████████████────██████████████─
────────────────────────────────────────────────────────────────────────
*/

void	Server::modeCmd(std::string buffer, int clientFd) {

	std::size_t pos = 5;

	if (buffer.find("#") == std::string::npos && buffer.find("+i") != std::string::npos) {

		sendMessage(RPL_UMODEIS(_users.find(clientFd)->second->getFullName(), "+i"), clientFd);
		return ;
	}

	else if (buffer.find("#") == std::string::npos) {

		sendMessage(ERR_UNKNOWNCOMMAND(_users.find(clientFd)->second->getNick(), buffer), clientFd);
		return ;
	}

	std::string channel = findFittingChan(buffer, clientFd);
	buffer = buffer.substr(pos, buffer.find("\r\n") - 5);

	if (channel == " ") {
		return ;
	}

	if (buffer.find("MODE " + channel + "\r\n") != std::string::npos) {
		sendMessage(RPL_CHANNELMODEIS(_users[clientFd]->getNick(), channel, + "+" +_channelInfos[channel]->allModesInChannel() + "\r\n"), clientFd);
		return ;
	}

	if ( _users.find(clientFd)->second->getIsOp() == false) {
		sendMessage(ERR_CHANOPRIVSNEEDED(_users.find(clientFd)->second->getNick(), channel), clientFd);
		return ;
	}


	if (buffer.find("+i") != std::string::npos) {

		if (_users.find(clientFd)->second->getChannelName() != "default")
			_channelInfos.find(channel)->second->setInviteMode(true);
		_channelInfos.find(channel)->second->sendAllUsers(MODE(_users.find(clientFd)->second->getNick(), channel, "+i", ""), clientFd);
		sendMessage(MODE(_users.find(clientFd)->second->getNick(), channel, "+i", ""), clientFd);
	}

	else if (buffer.find("-i") != std::string::npos) {

		_channelInfos.find(channel)->second->setInviteMode(false);
		_channelInfos.find(channel)->second->sendAllUsers(MODE(_users.find(clientFd)->second->getNick(), channel, "-i", ""), clientFd);
		sendMessage(MODE(_users.find(clientFd)->second->getNick(), channel, "-i", ""), clientFd);
	}

	else if (buffer.find("+t") != std::string::npos) {

		_channelInfos.find(channel)->second->setTopicMode(true);
		_channelInfos.find(channel)->second->sendAllUsers(MODE(_users.find(clientFd)->second->getNick(), channel, "+t", ""), clientFd);
		sendMessage(MODE(_users.find(clientFd)->second->getNick(), channel, "+t", ""), clientFd);
	}

	else if (buffer.find("-t") != std::string::npos) {

		_channelInfos.find(channel)->second->setTopicMode(false);
		_channelInfos.find(channel)->second->sendAllUsers(MODE(_users.find(clientFd)->second->getNick(), channel, "-t", ""), clientFd);
		sendMessage(MODE(_users.find(clientFd)->second->getNick(), channel, "-t", ""), clientFd);
	}

	else if (buffer.find("+k") != std::string::npos) {

		std::stringstream ss;
		ss << buffer;
		std::string password;

		while (ss >> password) {}
		password.erase(0, password.find("+k") + 1);

		_channelInfos.find(channel)->second->setPasswordChannel(password);
		_channelInfos.find(channel)->second->sendAllUsers(MODE(_users.find(clientFd)->second->getNick(), channel, "+k", ""), clientFd);
		sendMessage(MODE(_users.find(clientFd)->second->getNick(), channel, "+k", ""), clientFd);
	}

	else if (buffer.find("-k") != std::string::npos) {

		_channelInfos.find(channel)->second->setPasswordChannel("");
		_channelInfos.find(channel)->second->sendAllUsers(MODE(_users.find(clientFd)->second->getNick(), channel, "-k", ""), clientFd);
		sendMessage(MODE(_users.find(clientFd)->second->getNick(), channel, "-k", ""), clientFd);
	}

	else if (buffer.find("+o") != std::string::npos) {

		std::string nick = findFittingNick(buffer, clientFd);
		int targetFd = getUserFromNick(nick);

		_channelInfos.find(channel)->second->setUserOp(targetFd, true);
		_channelInfos.find(channel)->second->sendAllUsers(MODE(_users.find(clientFd)->second->getNick(), channel, "+o", ""), clientFd);
		sendMessage(MODE(_users.find(clientFd)->second->getNick(), channel, "+o", ""), clientFd);
	}

	else if (buffer.find("-o") != std::string::npos) {

		std::string nick = findFittingNick(buffer, clientFd);
		int targetFd = getUserFromNick(nick);

		_channelInfos.find(channel)->second->setUserOp(targetFd, false);
		_channelInfos.find(channel)->second->sendAllUsers(MODE(_users.find(clientFd)->second->getNick(), channel, "-o", ""), clientFd);
		sendMessage(MODE(_users.find(clientFd)->second->getNick(), channel, "-o", ""), clientFd);
	}

	else if (buffer.find("+l") != std::string::npos) {

		std::stringstream ss;
		ss << buffer;
		std::string limit;

		while (ss >> limit) {}
		limit.erase(0, limit.find("+l") + 1);

		int limitOfUsers = atoi(limit.c_str());

		_channelInfos.find(channel)->second->setHasLimitedUsers(true);
		_channelInfos.find(channel)->second->setLimitUsers(limitOfUsers);
		_channelInfos.find(channel)->second->sendAllUsers(MODE(_users.find(clientFd)->second->getNick(), channel, "+l", ""), clientFd);
		sendMessage(MODE(_users.find(clientFd)->second->getNick(), channel, "+l", ""), clientFd);
	}

	else if (buffer.find("-l") != std::string::npos) {

		std::cout << "dans le -l" << std::endl;
		_channelInfos.find(channel)->second->setHasLimitedUsers(false);
		_channelInfos.find(channel)->second->setLimitUsers(100);
		_channelInfos.find(channel)->second->sendAllUsers(MODE(_users.find(clientFd)->second->getNick(), channel, "-l", ""), clientFd);
		sendMessage(MODE(_users.find(clientFd)->second->getNick(), channel, "-l", ""), clientFd);
	}

}

/*
────────────────────────────────────────────────────────────────────
─██████████████──██████████████──████████████████────██████████████─
─██░░░░░░░░░░██──██░░░░░░░░░░██──██░░░░░░░░░░░░██────██░░░░░░░░░░██─
─██░░██████░░██──██░░██████░░██──██░░████████░░██────██████░░██████─
─██░░██──██░░██──██░░██──██░░██──██░░██────██░░██────────██░░██─────
─██░░██████░░██──██░░██████░░██──██░░████████░░██────────██░░██─────
─██░░░░░░░░░░██──██░░░░░░░░░░██──██░░░░░░░░░░░░██────────██░░██─────
─██░░██████████──██░░██████░░██──██░░██████░░████────────██░░██─────
─██░░██──────────██░░██──██░░██──██░░██──██░░██──────────██░░██─────
─██░░██──────────██░░██──██░░██──██░░██──██░░██████──────██░░██─────
─██░░██──────────██░░██──██░░██──██░░██──██░░░░░░██──────██░░██─────
─██████──────────██████──██████──██████──██████████──────██████─────
────────────────────────────────────────────────────────────────────
*/

void	Server::partCmd(std::string buffer, int clientFd) {

	std::string channel = findFittingChan(buffer, clientFd);
	std::cout << "channel partCmd: " << channel << std::endl;

	if (channel == " ") {
		return ;
	}

	if (_users.find(clientFd)->second->getChannelName() == channel) {
		_channelInfos[channel]->eraseUserInChannel(clientFd);
		_users.find(clientFd)->second->setChannelName("default");
		_users.find(clientFd)->second->setIsOp(false);
		_channelInfos[channel]->setCurrentUsers("-");
		_channelInfos.find(channel)->second->sendAllUsers(PART(_users.find(clientFd)->second->getNick(), _users.find(clientFd)->second->getFullName(), channel), clientFd);
		sendMessage(PART(_users.find(clientFd)->second->getNick(), _users.find(clientFd)->second->getFullName(), channel), clientFd);

		// if (_channelInfos[channel]->getCurrentUsers() == 0) {
		// 	_channelInfos[channel]->~Channel();
		// 	_channelInfos.erase(channel);
		// }
	}

	else {
		sendMessage(ERR_NOTONCHANNEL(_users.find(clientFd)->second->getFullName(), channel), clientFd);
	}
}

/*
────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────
─██████████████──████████████████────██████████──██████──██████──██████──────────██████──██████████████──██████████████─
─██░░░░░░░░░░██──██░░░░░░░░░░░░██────██░░░░░░██──██░░██──██░░██──██░░██████████████░░██──██░░░░░░░░░░██──██░░░░░░░░░░██─
─██░░██████░░██──██░░████████░░██────████░░████──██░░██──██░░██──██░░░░░░░░░░░░░░░░░░██──██░░██████████──██░░██████████─
─██░░██──██░░██──██░░██────██░░██──────██░░██────██░░██──██░░██──██░░██████░░██████░░██──██░░██──────────██░░██─────────
─██░░██████░░██──██░░████████░░██──────██░░██────██░░██──██░░██──██░░██──██░░██──██░░██──██░░██████████──██░░██─────────
─██░░░░░░░░░░██──██░░░░░░░░░░░░██──────██░░██────██░░██──██░░██──██░░██──██░░██──██░░██──██░░░░░░░░░░██──██░░██──██████─
─██░░██████████──██░░██████░░████──────██░░██────██░░██──██░░██──██░░██──██████──██░░██──██████████░░██──██░░██──██░░██─
─██░░██──────────██░░██──██░░██────────██░░██────██░░░░██░░░░██──██░░██──────────██░░██──────────██░░██──██░░██──██░░██─
─██░░██──────────██░░██──██░░██████──████░░████──████░░░░░░████──██░░██──────────██░░██──██████████░░██──██░░██████░░██─
─██░░██──────────██░░██──██░░░░░░██──██░░░░░░██────████░░████────██░░██──────────██░░██──██░░░░░░░░░░██──██░░░░░░░░░░██─
─██████──────────██████──██████████──██████████──────██████──────██████──────────██████──██████████████──██████████████─
────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────
*/


void	Server::privmsgCmd(std::string buffer, int clientFd) {

	size_t posMsg = buffer.find(":") + 1;
	std::string channel = findFittingChan(buffer, clientFd);
	std::string msg = buffer.substr(posMsg, buffer.find("\r\n", posMsg) - posMsg);

	if (channel == " " || msg == " ") {
		return ;
	}

	if (_users[clientFd]->getChannelName() == channel) {
		_channelInfos.find(channel)->second->sendAllUsers(PRIVMSG(_users.find(clientFd)->second->getNick(), channel, msg), clientFd);
	}

	_channelInfos.find(channel)->second->sendAllUsers(":" + _users[clientFd]->getNick() + ":" + msg + "\r\n", clientFd);
}

void Server::passCmd(std::string buffer, int fd) {

	std::size_t pos = buffer.find("PASS");
	std::string pwd = buffer.substr(pos + 5, buffer.find("\r\n") - (pos + 5));

	if (pwd != _password) {
		sendMessage(ERR_PASSWDMISMATCH(_users.find(fd)->second->getNick()), fd);
	}

	else {
		_users[fd]->setHasPassword(true);
		_users[fd]->setIsRegistered();
	}

	if (_users[fd]->getIsRegistered() == true && _users[fd]->getHasBeenWelcomed() == false) {

		sendMessage(RPL_WELCOME(_users.find(fd)->second->getNick(), _users.find(fd)->second->getFullName()), fd);
		sendMessage(RPL_YOURHOST(_users.find(fd)->second->getNick()), fd);
		sendMessage(RPL_CREATED(_users.find(fd)->second->getNick(), "2025/03/16 11:49:08"), fd);
		sendMessage(RPL_MYINFO(_users.find(fd)->second->getNick()), fd);
		_users.find(fd)->second->setHasBeenWelcomed(true);
	}
}

void Server::nickCmd(std::string buffer, int fd) {

	std::size_t pos = buffer.find("NICK");
	std::string newNick = buffer.substr(pos + 5, buffer.find("\r\n") - (pos + 5));

	if (validNick(newNick)) {

		_users.find(fd)->second->setNick(newNick);
		_users.find(fd)->second->setIsRegistered();
	}

	else{

		sendMessage(ERR_ERRONEUSNICKNAME(_users.find(fd)->second->getNick()), fd);
	}

	if (_users[fd]->getIsRegistered() == true && _users[fd]->getHasBeenWelcomed() == false) {

		sendMessage(RPL_WELCOME(_users.find(fd)->second->getNick(), _users.find(fd)->second->getFullName()), fd);
		sendMessage(RPL_YOURHOST(_users.find(fd)->second->getNick()), fd);
		sendMessage(RPL_CREATED(_users.find(fd)->second->getNick(), "2025/03/16 11:49:08"), fd);
		sendMessage(RPL_MYINFO(_users.find(fd)->second->getNick()), fd);
		_users.find(fd)->second->setHasBeenWelcomed(true);
	}
}

void Server::userCmd(std::string buffer, int fd) {

	std::size_t pos = 5;
	std::string fullname = buffer.substr(pos, buffer.find(" ", pos) - pos);

	_users.find(fd)->second->setFullName(fullname);
	_users.find(fd)->second->setIsRegistered();

	if (_users[fd]->getIsRegistered() == true && _users[fd]->getHasBeenWelcomed() == false) {

		sendMessage(RPL_WELCOME(_users.find(fd)->second->getNick(), _users.find(fd)->second->getFullName()), fd);
		sendMessage(RPL_YOURHOST(_users.find(fd)->second->getNick()), fd);
		sendMessage(RPL_CREATED(_users.find(fd)->second->getNick(), "2025/03/16 11:49:08"), fd);
		sendMessage(RPL_MYINFO(_users.find(fd)->second->getNick()), fd);
		_users.find(fd)->second->setHasBeenWelcomed(true);
	}
}

void Server::sendMessage(std::string message, int fd) {

    if (send(fd, message.c_str(), message.size(), 0) == -1) {
        std::cerr << RED << "Error sending message to client FD " << fd << ": " << strerror(errno) << RESET << std::endl;
    }
}

void Server::deleteUser(int fd) {

	if (_users[fd]->getChannelName() != "default")
		_channelInfos[_users[fd]->getChannelName()]->eraseUserInChannel(fd);

	delete _users.find(fd)->second;
	_users.erase(fd);

}

int Server::getUserFromNick(std::string nickname) const {
	int targetFd;

	for(std::map<int, User*>::const_iterator it = _users.begin(); it != _users.end(); it++) {
		if (it->second->getNick() == nickname)
			targetFd = it->second->getFd();

		else if (it == _users.end())
			return -1;
	}

	return (targetFd);
}

std::string Server::findFittingNick(std::string buffer, int clientFd) {
	std::map<int, User*>::iterator it = _users.begin();

	while(buffer.find(it->second->getNick()) == std::string::npos) {
		it++;
		if (it == _users.end()) {
			sendMessage(ERR_NOSUCHNICK(_users.find(clientFd)->second->getNick(), ""), clientFd);
			return " ";
		}
	}

	return it->second->getNick();
}

std::string Server::findFittingChan(std::string buffer, int clientFd) {

	std::stringstream ss(buffer);
	std::string token;



	if (_users.find(clientFd)->second->getChannelName() != "default") {

		while (ss >> token) {
			if (token.find("#") != std::string::npos) {
				break;
			}
			token.erase();
		}

		return token;
	}

	else {

		sendMessage(ERR_NOTONCHANNEL(_users.find(clientFd)->second->getNick(), ""), clientFd);
		return " ";
	}
}

bool containsWrongChar(const std::string& str) {

	for (size_t i = 0; i < str.size(); i++) {

		if (str[i] < 'a' && str[i] > 'z')
			return true;

		else if (str[i] < 'A' && str[i] > 'Z')
			return true;

		else if (str[i] < '0' && str[i] > '9')
			return true;
	}

	return false;
}

bool Server::validNick(const std::string& nick) {

	if (containsWrongChar(nick)) {
		return (false);
	}

	std::map<int, User*>::iterator it;
	for (it = _users.begin(); it != _users.end(); it++) {

		if (it->second->getNick() == nick)
			return (false);
	}

	return (true);
}
