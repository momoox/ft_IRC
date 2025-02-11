#pragma once

#include "irc.hpp"

class User {
	private:
		std::string _nickname;
		std::string _fullname;
		bool		_isOp;
		int			_userFD;

	public:
		User();
		~User();

};