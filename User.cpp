#include "User.hpp"

User::User() {
	_nickname = "default";
	_fullname = "default";
	_userIP = "default";
	_userFD = 0;

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

void User::setFD(int fd) {
	_userFD = fd;
}

void User::setIsOp(bool state) {
	_isOp = state;
}

std::string User::validNick(const std::string& nick) {
	//pas de double du meme nickname, pas de #,@,:,' '
	if (nick.find(' ') || nick.find('@') || nick.find('#') || nick.find(':') || nick.find(',') || nick.find('*') || nick.find('?') || nick.find('!') || nick.find('.') || nick.find('&') || nick.find('+'))
		throw std::invalid_argument("Nickname invalid");
	//throw RPL error
	return (nick);
}

// std::string User::valideUsername(const std::string& username) {
// 	return
// }

std::string User::getNick() const {
	return (_nickname);
}

std::string User::getFullName() const {
	return (_fullname);
}

std::string User::getUserIP() const {
	return (_userIP);
}

int User::getFd() const {
	return (_userFD);
}

bool User::getIsOp() const {
	return (_isOp);
}