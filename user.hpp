#pragma once 

#include "server.hpp"

class User {
	private:
		std::string _nickname;
		std::string _fullname;
		std::string	_userIP;

		bool		_isOp;

	public:
		User();
		~User();

		void setNick(std::string nickname);
		void setFullName(std::string fullname);
		void setUserIP(std::string IP);
		void setIsOp(bool state);

		std::string getNick() const;
		std::string getFullName() const;
		std::string getUserIP() const;
		bool getIsOp() const;

};