#include "User.hpp"

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

User::User(int fd) {
	_nickname = "default";
	_fullname = "default";
	_userIP = "default";
	_userFD = fd;

	_isOp = false;
}

User::~User() {
	//send RPL goodbye
	close(_userFD);
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

bool User::validNick(const std::string& nick) {
	//pas de double du meme nickname, pas de #,@,:,' '
	if (containsWrongChar(nick)) {
		return (false);
	}
	return (true);
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

std::string User::getBuffer() const {
	return (_buffer);
}

int User::getFd() const {
	return (_userFD);
}

bool User::getIsOp() const {
	return (_isOp);
}

void User::addToBuffer(std::string str) {
	std::cout << "str = [" << str << "]" << std::endl;
	std::cout << "buffer = [" << _buffer << "]" << std::endl;
	_buffer += str;
}

void User::eraseBuffer() {
	_buffer.erase();
}
