#include "Channel.hpp"

Channel::Channel(std::string channelName) {
	_channelName = channelName;
	_currentUsers = 0;
	_limitUsers = 0;
	_inviteMode = false;
	_topicMode = false;
	_passwordChannel = "";
}
	
Channel::~Channel() {
	
}

Channel::Channel(Channel const &src) {
	*this = src;
}

const Channel& Channel::operator= (Channel const &rhs) {
	if (this != &rhs) {
		this->_channelName = rhs._channelName;
        this->_passwordChannel = rhs._passwordChannel;
        this->_currentUsers = rhs._currentUsers;
        this->_limitUsers = rhs._limitUsers;
        this->_inviteMode = rhs._inviteMode;
        this->_topicMode = rhs._topicMode;
	}

	return (*this);
}

void Channel::setMapUsers(int clientFd, User *user) {
	_usersInChannel.insert(std::make_pair(clientFd, user));

	std::cout << "user in channel map: " << _usersInChannel.find(clientFd)->second->getNick() << std::endl;
}

void Channel::setChannelName(std::string channelName) {
	_channelName = channelName;
}

void Channel::setPasswordChannel(std::string password) {
	_passwordChannel = password;
}

void Channel::setCurrentUsers() {
	_currentUsers += 1; 
}

void Channel::setlimitUsers(int limitOfUsers) {
	_limitUsers = limitOfUsers;
}

void Channel::setInviteMode(bool state) {
	_inviteMode = state;
}

void Channel::setTopicMode(bool state) {
	_topicMode = state;
}

std::string Channel::getChannelName() const {
	return (_channelName);
}

std::string Channel::getPasswordChannel() const {
	return (_passwordChannel);
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