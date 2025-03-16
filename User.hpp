#pragma once

#include "Server.hpp"

class User {
	private:
		std::string _nickname;
		std::string _fullname;
		std::string	_userIP;
		std::string _buffer;
		std::string _channelName;
		bool		_isRegistered;
		bool 		_hasPassword;
		bool		_hasBeenWelcomed;

		int			_userFD;
		bool		_isOp;
		std::set<std::string>	_invited;

	public:
		User(int fd);
		~User();

		void setNick(std::string nickname);
		void setFullName(std::string fullname);
		void setUserIP(std::string IP);
		void setChannelName(std::string channel);
		void setFD(int fd);
		void setIsOp(bool state);
		void setInvited(std::string channelName);
		void setIsRegistered();
		void setHasBeenWelcomed(bool state);
		void setHasPassword(bool state);
		void removeChannelInvite(std::string channelName);

		bool validNick(const std::string& nick);

		std::string getNick() const;
		std::string getFullName() const;
		std::string getUserIP() const;
		std::string getBuffer() const;
		std::string getChannelName() const;


		int getFd() const;
		bool getIsRegistered() const;
		bool getHasBeenWelcomed() const;
		bool getIsOp() const;
		bool isInvited(std::string channelName) const;

		void addToBuffer(std::string str);
		void eraseBuffer();

};
