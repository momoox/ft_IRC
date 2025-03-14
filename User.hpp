#pragma once 

#include "Server.hpp"

class User {
	private:
		std::string _nickname;
		std::string _fullname;
		std::string	_userIP;

		int			_userFD;
		bool		_isOp;

	public:
		User();
		~User();

		void setNick(std::string nickname);
		void setFullName(std::string fullname);
		void setUserIP(std::string IP);
		void setFD(int fd);
		void setIsOp(bool state);

		std::string validNick(const std::string& nick);
		//je sais pas encore s'il y des trucs a check pour le fullname
		// std::string valideUsername(const std::string& username);

		std::string getNick() const;
		std::string getFullName() const;
		std::string getUserIP() const;
		int getFd() const;
		bool getIsOp() const;

};