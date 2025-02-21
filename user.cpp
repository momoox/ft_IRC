#include "user.hpp"

User::User() {
	_nickname = "default";
	_fullname = "default";
	_userIP = "default";

	_isOp = false;

}

User::~User() {
	
}

void User::setNick(std::string nickname) {
	_nickname = nickname;
}

void User::setFullName(std::string fullname) {
	_fullname = fullname;
}

void User::setUserIP(std::string IP) {
	_userIP = IP;
}

void User::setIsOp(bool state) {
	_isOp = state;
}

std::string User::getNick() const {
	return (_nickname);
}

std::string User::getFullName() const {
	return (_fullname);
}

std::string User::getUserIP() const {
	return (_userIP);
}


bool User::getIsOp() const {
	return (_isOp);
}