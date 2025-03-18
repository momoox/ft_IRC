#include "Channel.hpp"

Channel::Channel(std::string channelName) {

	_channelName = channelName;
	_currentUsers = 0;
	_limitUsers = 100;
	_inviteMode = false;
	_topicMode = false;
	_hasLimitedUsers = false;
	_passwordChannel = "";
}

Channel::~Channel() {}

Channel::Channel(Channel const &src) {
	*this = src;
}

const Channel& Channel::operator = (Channel const &rhs) {

	if (this != &rhs) {

		this->_channelName = rhs._channelName;
        this->_passwordChannel = rhs._passwordChannel;
        this->_currentUsers = rhs._currentUsers;
        this->_limitUsers = rhs._limitUsers;
        this->_inviteMode = rhs._inviteMode;
        this->_topicMode = rhs._topicMode;
		this->_hasLimitedUsers = rhs._hasLimitedUsers;
	}

	return (*this);
}

void Channel::setMapUsers(int clientFd, User *user) {

	_usersInChannel.insert(std::make_pair(clientFd, user));
}

void Channel::setUserOp(int clientFd, bool state) {

	if (state == true)
		_usersInChannel.find(clientFd)->second->setIsOp(true);

	else
		_usersInChannel.find(clientFd)->second->setIsOp(false);
}

void Channel::setChannelName(std::string channelName) {
	_channelName = channelName;
}

void Channel::setPasswordChannel(std::string password) {
	_passwordChannel = password;
}

void Channel::setTopic(std::string topic) {
	_topic = topic;
}

void Channel::setCurrentUsers(std::string set) {

	if (set == "+")
		_currentUsers += 1;

	else
		_currentUsers -= 1;
}

void Channel::setHasLimitedUsers(bool state) {
	_hasLimitedUsers = state;
}

void Channel::setInviteMode(bool state) {
	_inviteMode = state;
}

void Channel::setTopicMode(bool state) {
	_topicMode = state;
}

void Channel::setLimitUsers(int limitOfUsers) {
	_limitUsers = limitOfUsers;
}

std::string Channel::getChannelName() const {
	return (_channelName);
}

std::string Channel::getPasswordChannel() const {
	return (_passwordChannel);
}

std::string Channel::getTopic() const{
	return (_topic);
}

std::string Channel::allUsersInChannel() const {

	std::string allUsers;
	std::string opUsers;

	for (std::map<int, User*>::const_iterator it = _usersInChannel.begin(); it != _usersInChannel.end(); it++) {
		if (it->second->getIsOp() == true)
			opUsers += "@" + it->second->getNick() + " ";

		else
			allUsers += it->second->getNick() + " ";
	}

	allUsers += opUsers;

	return (allUsers);
}

std::string Channel::allModesInChannel() const {

	std::string allModes = "";

	if (_inviteMode == true)
		allModes += "i";

	if (_topicMode == true)
		allModes += "t";

	if (_limitUsers != 100)
		allModes += "l";

	if (_passwordChannel != "")
		allModes += "k";

	return (allModes);
}

int Channel::getCurrentUsers() const {
	return (_currentUsers);
}

int	Channel::getlimitUsers() const {
	return (_limitUsers);
}

bool Channel::getInviteMode() const {
	return (_inviteMode);
}

bool Channel::getTopicMode() const {
	return (_topicMode);
}

bool Channel::hasLimitedUsers() const {

	if (_hasLimitedUsers == true)
		return (true);

	return (false);
}

void Channel::kickUserFromChannel(int clientFd) {

    std::map<int, User*>::iterator it = _usersInChannel.find(clientFd);

    if (it != _usersInChannel.end()) {
        _usersInChannel.erase(it);
    }
}

void	Channel::sendAllUsers(std::string message, int clientFd) {

	for (std::map<int, User*>::iterator it = _usersInChannel.begin(); it != _usersInChannel.end(); it++) {
		if (it->first != clientFd) {
			send(it->first, message.c_str(), message.size(), 0);
		}
	}
}

void Channel::eraseUserInChannel(int clientFd) {

	if (_usersInChannel.find(clientFd) != _usersInChannel.end()) {
		_usersInChannel.erase(clientFd);
	}
}
