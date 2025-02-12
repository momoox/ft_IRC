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

};