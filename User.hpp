#pragma once

#include "Server.hpp"

class User {
	private:
		std::string _nickname;
		std::string _fullname;
		std::string	_userIP;
		std::string _buffer;
		std::string _channelName;

		int			_userFD;
		bool		_isOp;
		bool		_invited;

	public:
		User(int fd);
		~User();

		void setNick(std::string nickname);
		void setFullName(std::string fullname);
		void setUserIP(std::string IP);
		void setChannelName(std::string channel);
		void setFD(int fd);
		void setIsOp(bool state);
		void setInvited(bool state);

		bool validNick(const std::string& nick);
		//je sais pas encore s'il y des trucs a check pour le fullname
		// std::string valideUsername(const std::string& username);

		std::string getNick() const;
		std::string getFullName() const;
		std::string getUserIP() const;
		std::string getBuffer() const;
		std::string getChannelName() const;
		int getFd() const;
		bool getIsOp() const;
		bool getInvite() const;

		void addToBuffer(std::string str);
		void eraseBuffer();

};
