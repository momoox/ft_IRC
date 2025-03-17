#include "User.hpp"

// bool containsWrongChar(const std::string& str) {
// 	for (size_t i = 0; i < str.size(); i++) {
// 		if (str[i] < 'a' && str[i] > 'z')
// 			return true;
// 		else if (str[i] < 'A' && str[i] > 'Z')
// 			return true;
// 		else if (str[i] < '0' && str[i] > '9')
// 			return true;
// 	}
// 	return false;
// }

User::User(int fd) {
	_nickname = "default";
	_fullname = "default";
	_userIP = "default";
	_buffer = "";
	_userFD = fd;
	_channelName = "default";

	_isDead = false;
	_isOp = false;
	_isRegistered = false;
	_hasPassword = false;
	_hasBeenWelcomed = false;
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

void User::setChannelName(std::string channel) {
	_channelName = channel;
}

void User::setFD(int fd) {
	_userFD = fd;
}

void User::setIsOp(bool state) {
	_isOp = state;
}

void User::setInvited(std::string channelName) {
	_invited.insert(channelName);
}

void User::setIsRegistered() {
	if (_nickname != "default" && _fullname != "default" && _hasPassword == true)
		_isRegistered = true;
}

void User::setHasBeenWelcomed(bool state) {
	_hasBeenWelcomed = state;
}

void User::setHasPassword(bool state) {
	_hasPassword = state;
}

void User::setDead(bool state) {
	_isDead = state;
}

// bool User::validNick(const std::string& nick) {
// 	//pas de double du meme nickname, pas de #,@,:,' '
// 	if (containsWrongChar(nick)) {
// 		return (false);
// 	}

// 	std::map<int, User*>::iterator it;
// 	for (it = _users.begin(); it != _users.end(); it++) {
// 		if (it->second->getNick() == nick)
// 			return (false);
// 	}

// 	return (true);
// }

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

std::string User::getChannelName() const {
	return (_channelName);
}

int User::getFd() const {
	return (_userFD);
}

bool User::getIsRegistered() const {
	return _isRegistered;
}

bool User::getHasBeenWelcomed() const {

	return _hasBeenWelcomed;
}

bool User::getIsOp() const {
	return (_isOp);
}

bool User::getIsInvited(std::string channelName) const {
	if (_invited.find(channelName) != _invited.end())
		return true;
	else
		return false;
}

bool User::getIsDead() const {
	return _isDead;
}

void User::addToBuffer(std::string str) {
	// std::cout << "str = [" << str << "]" << std::endl;
	// std::cout << "buffer = [" << _buffer << "]" << std::endl;
	_buffer.append(str);
}

void User::eraseBuffer() {
	try {	
		_buffer.erase();
	}

	catch (std::exception &e) {
		std::cerr << e.what() << std::endl;
	}
}

void User::removeChannelInvite(std::string channelName) {
	_invited.erase(channelName);
}

void User::eraseEnterInBuffer() {
	if (_buffer.find("\r\n") != std::string::npos)
		_buffer = _buffer.substr(0, _buffer.size() - 2);
	else if (_buffer.find("\n") != std::string::npos)
		_buffer = _buffer.substr(0, _buffer.size() - 1);
}
