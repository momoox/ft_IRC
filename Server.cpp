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

	char ip[100];
	gethostname(ip, sizeof(ip));
	std::string IP = inet_ntoa(*((struct in_addr *)(gethostbyname(ip))->h_addr_list[0]));
    _ip = IP;

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

void Server::parserMessage(std::string message, int clientFd) {
	std::string cmd;
	std::string cmdArg;
	std::size_t pos = message.find("\r\n");
	std::string buffer = message.substr(0, pos + 2);


	// std::cout << "Message: " << message << std::endl;
	while (!buffer.empty()) {
		std::cout << "Buffer before check cmd: " << buffer << std::endl;
		if (buffer.find("CAP LS") != std::string::npos) {
			sendMessage(CAP_LS, clientFd);
		}

		else if (buffer.find("PRIVMSG") != std::string::npos && _users[clientFd]->getIsRegistered() == true) {
			privmsgCmd(buffer, clientFd);
		}

		else if (buffer.find("CAP END") != std::string::npos) {
			sendMessage(RPL_WELCOME(_users.find(clientFd)->second->getFullName()), clientFd);
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
			deleteUser(clientFd);
		}

		else if (buffer.find("WHOIS") != std::string::npos) {
			std::string nick = buffer.substr(buffer.find(" ") + 1, buffer.find("\r\n"));
			std::string msg = _users.find(clientFd)->second->getNick() + " :End of /WHOIS list" + "\r\n";
			sendMessage(msg, clientFd);
		}
		//! else if (is not registered)
		else {
			std::string cmd = buffer.substr(0, buffer.find(" "));
			sendMessage(ERR_UNKNOWNCOMMAND(_users.find(clientFd)->second->getNick(), cmd), clientFd);
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

}

void	Server::joinCmd(std::string buffer, int clientFd) {

	if (buffer.find("JOIN #") != std::string::npos) {
		std::size_t pos = 5;
		std::string channel = buffer.substr(pos, buffer.find("\r\n") - 5);

		if (_users.find(clientFd)->second->getChannelName() != channel && _channelInfos.find(channel) == _channelInfos.end()) {
			Channel* chan = new Channel(channel);

			_channelInfos.insert(std::make_pair(channel, chan));
			chan->setMapUsers(clientFd, _users.find(clientFd)->second);
			if (_users[clientFd]->getChannelName() != "default") {
				_channelInfos[_users[clientFd]->getChannelName()]->eraseUserInChannel(clientFd);
				_channelInfos[_users[clientFd]->getChannelName()]->setCurrentUsers("-");
				if (_channelInfos[channel]->getCurrentUsers() == 0) {
					//erase le channel
					_channelInfos[_users[clientFd]->getChannelName()]->~Channel();
					_channelInfos.erase(_users[clientFd]->getChannelName());
				}
			}
			_users.find(clientFd)->second->setChannelName(channel);

			if (chan->getCurrentUsers() == 0) {
				_users.find(clientFd)->second->setIsOp(true);
			}

			chan->setCurrentUsers("+");
			sendMessage(JOIN(_users[clientFd]->getNick(), _users[clientFd]->getFullName(), channel), clientFd);
		}

		else if (_users.find(clientFd)->second->getChannelName() == channel) {

			sendMessage(ERR_USERONCHANNEL(_users.find(clientFd)->second->getNick(), channel), clientFd);

		}


		else {
				std::cout << "current invite use : " << _users.find(clientFd)->second->isInvited(channel) << std::endl;
				std::cout << "invite mode in channel: " << _channelInfos.find(channel)->second->getInviteMode() << std::endl;

			if (_channelInfos.find(channel)->second->getInviteMode() == false) {

				// chan->setCurrentUsers();
				std::cout << "je suis dans channel invite mode false" << std::endl;

				_channelInfos.find(channel)->second->setCurrentUsers("+");
				_channelInfos.find(channel)->second->setMapUsers(clientFd, _users.find(clientFd)->second);
				if (_users[clientFd]->getChannelName() != "default") {
					_channelInfos[_users[clientFd]->getChannelName()]->eraseUserInChannel(clientFd);
					_channelInfos[_users[clientFd]->getChannelName()]->setCurrentUsers("-");
					if (_channelInfos[channel]->getCurrentUsers() == 0) {
						//erase le channel
						_channelInfos[_users[clientFd]->getChannelName()]->~Channel();
						_channelInfos.erase(_users[clientFd]->getChannelName());
					}
				}
				_users.find(clientFd)->second->setChannelName(channel);
			}

			else if (_channelInfos.find(channel)->second->getInviteMode() == true && _users.find(clientFd)->second->isInvited(channel) == true) {
				std::cout << "je suis dans channel invite mode true et user true" << std::endl;
				_channelInfos.find(channel)->second->setCurrentUsers("+");
				_channelInfos.find(channel)->second->setMapUsers(clientFd, _users.find(clientFd)->second);
				if (_users[clientFd]->getChannelName() != "default") {
					_channelInfos[_users[clientFd]->getChannelName()]->eraseUserInChannel(clientFd);
					_channelInfos[_users[clientFd]->getChannelName()]->setCurrentUsers("-");
					if (_channelInfos[channel]->getCurrentUsers() == 0) {
						//erase le channel
						_channelInfos[_users[clientFd]->getChannelName()]->~Channel();
						_channelInfos.erase(_users[clientFd]->getChannelName());
					}
				}
				_users.find(clientFd)->second->setChannelName(channel);
			}

			else if (_channelInfos.find(channel)->second->getInviteMode() == true && _users.find(clientFd)->second->isInvited(channel) == false) {
				std::cout << "je suis dans channel invite mode true et user false" << std::endl;
				std::cout << "current user: " << _users.find(clientFd)->second->getFullName() << std::endl;
				std::cout << "current user channel: " << _users.find(clientFd)->second->getChannelName() << std::endl;
				sendMessage(ERR_INVITEONLYCHAN(_users.find(clientFd)->second->getNick(), channel), clientFd);
			}
		}
		if (_channelInfos.find(channel)->second->getTopic().size() > 0) {
			sendMessage(RPL_TOPIC(_users.find(clientFd)->second->getNick(), channel, _channelInfos.find(channel)->second->getTopic()), clientFd);
		}
		else {
			sendMessage(RPL_NOTOPIC(_users.find(clientFd)->second->getNick(), channel), clientFd);
		}
	}

}


void	Server::inviteCmd(std::string buffer, int clientFd) {
// /invite <nickname> #channel
	int targetFd;
	//std::size_t pos = 7;
	std::string channel;

	if (buffer.find("#") != std::string::npos)
		channel = findFittingChan(buffer);
	else {
		sendMessage(ERR_NOSUCHCHANNEL(_users.find(clientFd)->second->getNick(), "NO_NAME"), clientFd);
		return ;
	}
	std::string nickname = findFittingNick(buffer);
	std::cout << "nickname: " << nickname << std::endl;
	std::cout << "channel: " << channel << std::endl;
	targetFd = getUserFromNick(nickname);
	if (targetFd < 0) {
		std::cout << "no user found1" << std::endl;
		sendMessage(ERR_NOSUCHCHANNEL(_users.find(clientFd)->second->getNick(), nickname), clientFd);
		return ;
	}

	if (_channelInfos.find(channel) != _channelInfos.end()) {

		if (_users.find(clientFd)->second->getChannelName() != channel) {
			sendMessage(ERR_NOTONCHANNEL(_users.find(clientFd)->second->getNick(), channel), clientFd);
		}

		else if (_users.find(clientFd)->second->getIsOp() == false) {
			sendMessage(ERR_CHANOPRIVSNEEDED(_users.find(clientFd)->second->getNick(), channel), clientFd);
		}

		sendMessage(RPL_INVITING(_users.find(clientFd)->second->getNick(), nickname, channel), targetFd);
		_users.find(targetFd)->second->setInvited(channel);
	}

	else {
		std::cout << "no channel found" << std::endl;
		sendMessage(ERR_NOSUCHCHANNEL(_users.find(clientFd)->second->getNick(), channel), clientFd);
	}

}

void	Server::kickCmd(std::string buffer, int clientFd) {
	//tous les users du channel recoivent un message leur indiquant qu'un user a ete kick

	//seuls les op peuvent kick, sinon -> ERR_CHANOPRIVSNEEDED
	//si le nom du channel donne ne contient pas le # -> ERR_BADCHANMASK (ou un simple ERR_NOSUCHCHANNEL) ou s'il n'existe pas -> ERR_NOSUCHCHANNEL
	//si le user a kick n'existe pas -> ERR_NOSUCHNICK
	//si le user a kick n'est pas sur le channel -> ERR_USERNOTINCHANNEL
	int targetFd;
	std::size_t pos = 5;
	std::string channel;
	std::string userToKick;

	userToKick = buffer.substr(buffer.find(" ", pos) + 1, buffer.find(" :") - buffer.find(" ", pos) - 1);
	channel = buffer.substr(buffer.find("#"), buffer.find(" ", 5) - buffer.find("#"));
	std::cout << "user to kick: " << userToKick << std::endl;
	std::cout << "channel: " << channel << std::endl;


	if (_users.find(clientFd)->second->getIsOp() == true) {
		//si le nom du channel donne ne comporte pas de # -> error
		if (channel.find("#") == std::string::npos) {
			sendMessage(ERR_BADCHANMASK(_users.find(clientFd)->second->getNick(), ("#" + channel)), clientFd);

		}

		else if ((targetFd = getUserFromNick(userToKick)) == 0) {
			std::cout << "no user to kick found" << std::endl;
			sendMessage(ERR_NOSUCHNICK(_users.find(clientFd)->second->getNick(), userToKick), clientFd);

		}

		else if (_users.find(targetFd)->second->getChannelName() != channel) {
			std::cout << "no user to kick in channel" << std::endl;
			std::cout << "channelname in user: " << _users.find(targetFd)->second->getChannelName() << std::endl;
			sendMessage(ERR_USERNOTINCHANNEL(_users.find(clientFd)->second->getNick(), userToKick, channel), clientFd);

		}

		else {
			std::cout << "user being kick" << std::endl;
			_channelInfos.find(channel)->second->sendAllUsers(KICK(_users.find(clientFd)->second->getNick(), _users.find(targetFd)->second->getNick(), channel, ""), 0);
			_channelInfos.find(channel)->second->kickUserFromChannel(targetFd);
			_channelInfos[_users[clientFd]->getChannelName()]->setCurrentUsers("-");
			_users.find(targetFd)->second->setChannelName("default");
			_users.find(targetFd)->second->setIsOp(false);
			_users.find(targetFd)->second->removeChannelInvite(channel);
		}

	}

	else {

		sendMessage(ERR_CHANOPRIVSNEEDED(_users.find(clientFd)->second->getNick(), channel), clientFd);

	}
}

void	Server::topicCmd(std::string buffer, int clientFd) {
	// (void)buffer;
	// (void)clientFd;
	//recuperer le nom du channel
	//recuperer le nouveau topic

	//channel doit exister -> ERR_NOSUCHCHANNEL ou BADchannelmescouilles
	//user doit etre op sinon -> ERR_CHANPRIVI
	//tout le monde dans le channel recoit un message pour indiquer le nouveau topic

	//c'est bizarre parce qu'il faut ecrire </topic #channel > avec l'espace pour que irssi envoie le message au serveur la premiere fois que le topic est change, on peut ensuite juste ecrire </topic #test> ??!!
	//et les sendMessage print r dans le client
	std::cout << "buffer dans topic: " << buffer << std::endl;

	std::size_t pos = 6;
	std::string channel;

	if (buffer.find(":") != std::string::npos) {
		channel = buffer.substr(pos, buffer.find(" :") - pos);
		std::cout << "dans : pour trouver le channel" << std::endl;
		std::cout << "channel: "  << channel << std::endl;
	}

	else {
		channel = buffer.substr(pos, buffer.find("\r\n") - pos);
		std::cout << "dans le channel sans :" << std::endl;
	}

	std::cout << "user op: " << _users.find(clientFd)->second->getIsOp() << std::endl;

	if (_users.find(clientFd)->second->getIsOp() == true && channel == _users.find(clientFd)->second->getChannelName()) {
		if (buffer.find(": ") != std::string::npos) {
			//remove -> TOPIC #channel :  -> la commande est /topic #channel   (2 espaces)
			std::cout << "dnas le reset de topic" << std::endl;
			_channelInfos.find(channel)->second->setTopic(" ");
			std::cout << "RESET TOPIC IS: " << _channelInfos.find(channel)->second->getTopic() << std::endl;
			_channelInfos.find(channel)->second->sendAllUsers(RPL_NOTOPIC(_users.find(clientFd)->second->getNick(), channel), 0);
		}

		else if (buffer.find(":") != std::string::npos) {
			//set a topic -> TOPIC #channel :blablabla
			std::string topic = buffer.substr(pos + channel.size() + 2, buffer.find("\r\n") - pos);
			std::cout << "Topic change: " << topic << std::endl;
			_channelInfos.find(channel)->second->setTopic(topic);
			_channelInfos.find(channel)->second->sendAllUsers(RPL_TOPIC(_users.find(clientFd)->second->getNick(), channel, topic), 0);
		}

		else {
			//view the topic -> TOPIC #channel
			std::cout << "dans le print de topic" << std::endl;
			_channelInfos.find(channel)->second->getTopic();
			_channelInfos.find(channel)->second->sendAllUsers(RPL_TOPIC(_users.find(clientFd)->second->getNick(), channel, _channelInfos.find(channel)->second->getTopic()), 0);
		}

	}

	else {
		sendMessage(ERR_NOSUCHCHANNEL(_users.find(clientFd)->second->getNick(), ""), clientFd);
		std::cout << "channel n'existe pas" << std::endl;
	}

	std::cout << "Bien arrive dans TOPIC :)" << std::endl;
}

void	Server::modeCmd(std::string buffer, int clientFd) {
	// checker la structure de la commande -> MODE #channelname <param>
	// i -> user needs to be invited to join a channel
	//  t : Définir/supprimer les restrictions de la commande TOPIC pour les opérateurs de canaux
	// k : Définir/supprimer la clé du canal (mot de passe)
	// o : Donner/retirer le privilège de l’opérateur de canal
	// l : Définir/supprimer la limite d’utilisateurs pour le canal
	std::size_t pos = 5;
	std::string channel = buffer.substr(pos, buffer.find("\r\n") - (pos + 3));
	buffer = buffer.substr(pos, buffer.find("\r\n") - 5);

	std::cout << "string channel: " << channel << std::endl;
	std::cout << "buffer in mode: " << buffer << std::endl;

	if (buffer.find("#") == std::string::npos) {
		sendMessage(RPL_UMODEIS(_users.find(clientFd)->second->getFullName(), "+i"), clientFd);
	}

	else if (buffer.find("+i") != std::string::npos) {
		//set channel to invite only
		std::cout << "inside +i " << _channelInfos.find(channel)->second->getChannelName() << std::endl;
		if (_users.find(clientFd)->second->getChannelName() != "default")
			_channelInfos.find(channel)->second->setInviteMode(true);
		_channelInfos.find(channel)->second->sendAllUsers(MODE(_users.find(clientFd)->second->getNick(), channel, "+i", ""), clientFd);
		// std::cout << "channel for mode +i: " << _channelInfos.find(channel)->second->getChannelName() << std::endl;
		// std::cout << "current mode is " << _channelInfos.find(channel)->second->getInviteMode() << std::endl;
	}

	else if (buffer.find("-i") != std::string::npos) {
		//set channel to non invite
		_channelInfos.find(channel)->second->setInviteMode(false);
		_channelInfos.find(channel)->second->sendAllUsers(MODE(_users.find(clientFd)->second->getNick(), channel, "-i", ""), clientFd);
	}

	else if (buffer.find("+t") != std::string::npos) {
		//only op can change topic of channel
		_channelInfos.find(channel)->second->setTopicMode(true);
		_channelInfos.find(channel)->second->sendAllUsers(MODE(_users.find(clientFd)->second->getNick(), channel, "+t", ""), clientFd);
	}

	else if (buffer.find("-t") != std::string::npos) {
		//everyone can change topic of channel
		_channelInfos.find(channel)->second->setTopicMode(false);
		_channelInfos.find(channel)->second->sendAllUsers(MODE(_users.find(clientFd)->second->getNick(), channel, "-t", ""), clientFd);
	}

	else if (buffer.find("+k") != std::string::npos) {
		//met un mdp sur le channel, les users devront du coup ecrire le mdp en parametre pour pouvoir join, ->  /mode #chan +k <key>
		std::string password = buffer.substr(pos + channel.size() + 4, buffer.find("\r\n") - pos - channel.size() - 4);
		// std::cout << "password for channel: " << channel << std::endl;
		//! modifier join pour prendre en compte une connexion avec mdp

		_channelInfos.find(channel)->second->setPasswordChannel(password);
		_channelInfos.find(channel)->second->sendAllUsers(MODE(_users.find(clientFd)->second->getNick(), channel, "+k", ""), clientFd);
	}

	else if (buffer.find("-k") != std::string::npos) {
		//supprime l'acces via un mdp
		_channelInfos.find(channel)->second->setPasswordChannel("");
		_channelInfos.find(channel)->second->sendAllUsers(MODE(_users.find(clientFd)->second->getNick(), channel, "-k", ""), clientFd);
	}

	else if (buffer.find("+o") != std::string::npos) {
		//le user devient op -> /mode $chan +o nick
		std::size_t pos = buffer.find("+o") + 1;
		std::string nick = buffer.substr(pos, buffer.find("\r\n") - pos);
		int targetFd = getUserFromNick(nick);

		_channelInfos.find(channel)->second->setUserOp(targetFd, true);
		_channelInfos.find(channel)->second->sendAllUsers(MODE(_users.find(clientFd)->second->getNick(), channel, "+o", ""), clientFd);
	}

	else if (buffer.find("-o") != std::string::npos) {
		//le user n'est plus op -> /mode $chan -o nick
		std::size_t pos = buffer.find("+o") + 1;
		std::string nick = buffer.substr(pos, buffer.find("\r\n") - pos);
		int targetFd = getUserFromNick(nick);

		_channelInfos.find(channel)->second->setUserOp(targetFd, false);
		_channelInfos.find(channel)->second->sendAllUsers(MODE(_users.find(clientFd)->second->getNick(), channel, "-o", ""), clientFd);
	}

	else if (buffer.find("+l") != std::string::npos) {
		//met une limite sur le nombre de users qui ont le droit d'etre sur le channel -> /mode $chan +l n
		std::size_t pos = buffer.find("+l") + 2;
		std::string limit = buffer.substr(pos, buffer.find("\r\n") - pos);
		int limitOfUsers = atoi(limit.c_str());

		_channelInfos.find(channel)->second->setLimitUsers(limitOfUsers);
		_channelInfos.find(channel)->second->sendAllUsers(MODE(_users.find(clientFd)->second->getNick(), channel, "+l", ""), clientFd);
	}

	else if (buffer.find("-l") != std::string::npos) {
		//retire la limite de users sur le channel
		_channelInfos.find(channel)->second->setLimitUsers(0);
		_channelInfos.find(channel)->second->sendAllUsers(MODE(_users.find(clientFd)->second->getNick(), channel, "-l", ""), clientFd);
	}
	std::cout << "Bien arrive dans MODE :)" << std::endl;
}

void	Server::partCmd(std::string buffer, int clientFd) {
	std::size_t pos = 5;
	std::string channel = buffer.substr(pos, buffer.find("\r\n") - pos);
	std::cout << "channel: " << channel << std::endl;

	if (_users.find(clientFd)->second->getChannelName() == channel) {
		_channelInfos[channel]->eraseUserInChannel(clientFd);
		_users.find(clientFd)->second->setChannelName("default");
		_users.find(clientFd)->second->setIsOp(false);
		_channelInfos[channel]->setCurrentUsers("-");
		if (_channelInfos[channel]->getCurrentUsers() == 0) {
			_channelInfos[channel]->~Channel();
			_channelInfos.erase(channel);
		}
	}

	else {
		sendMessage(ERR_NOTONCHANNEL(_users.find(clientFd)->second->getNick(), channel), clientFd);
	}
}

void	Server::privmsgCmd(std::string buffer, int clientFd) {
	//(void)buffer;
	size_t posChannel = buffer.find("#");
	size_t posMsg = buffer.find(":") + 1;
	std::string channel = buffer.substr(posChannel, buffer.find(" ", posChannel) - posChannel);
	std::string msg = buffer.substr(posMsg, buffer.find("\r\n", posMsg) - posMsg);

	std::cout << "channel : " << channel << std::endl;
	std::cout << "msg : " << msg << std::endl;

	std::cout << "channel in map: " << _channelInfos.find(channel)->second->getChannelName() << std::endl;
	if (_users[clientFd]->getChannelName() == channel) {
	_channelInfos.find(channel)->second->sendAllUsers(PRIVMSG(_users.find(clientFd)->second->getNick(), channel, msg), clientFd);
	}
	//_channelInfos.find(channel)->second->sendAllUsers(":" + _users[clientFd]->getNick() + ":" + msg + "\r\n", clientFd);

	std::cout << "number of users: " << _channelInfos.find(channel)->second->getCurrentUsers() << std::endl;
	std::cout << "Bien arrive dans PRIVMSG :)" << std::endl;
}
// /connect localhost 6667 mdp

void Server::passCmd(std::string buffer, int fd) {
	std::size_t pos = buffer.find("PASS");
	std::string pwd = buffer.substr(pos + 5, buffer.find("\r\n") - (pos + 5));
	std::cout << "password: " << pwd << std::endl;
	std::cout << "server password: " << _password << std::endl;

	if (pwd != _password) {
		sendMessage(ERR_PASSWDMISMATCH(_users.find(fd)->second->getNick()), fd);
		//deleteUser(fd);
	}
	else {
		_users[fd]->setIsRegistered(true);
		//sendMessage(RPL_WELCOME(_users.find(fd)->second->getFullName()), fd);
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

// void Server::sendMessage(std::string message, int fd) {

//     send(fd, message.c_str(), message.size(), 0);
// }

void Server::sendMessage(std::string message, int fd) {
    if (send(fd, message.c_str(), message.size(), 0) == -1) {
        std::cerr << "Error sending message to client FD " << fd << ": " << strerror(errno) << std::endl;
    }
}

void Server::deleteUser(int fd) {

	_channelInfos[_users[fd]->getChannelName()]->eraseUserInChannel(fd);

	delete _users.find(fd)->second;
	_users.erase(fd);

}

int Server::getUserFromNick(std::string nickname) const{
	int targetFd;

	for(std::map<int, User*>::const_iterator it = _users.begin(); it != _users.end(); it++) {
		if (it->second->getNick() == nickname)
			targetFd = it->second->getFd();
		else if (it == _users.end())
			return -1;
	}
	return (targetFd);
}

std::string Server::findFittingNick(std::string buffer) {
	std::map<int, User*>::iterator it = _users.begin();
	while(buffer.find(it->second->getNick()) == std::string::npos && it != _users.end()) {
		it++;
	}
	return it->second->getNick();
}

std::string Server::findFittingChan(std::string buffer) {
	std::map<std::string, Channel*>::iterator it = _channelInfos.begin();
	while(buffer.find(it->first) == std::string::npos && it != _channelInfos.end()) {
		it++;
	}
	return it->first;
}
