#pragma once

#include "Server.hpp"

class User;

class Channel {
private:
	std::string _channelName;
	std::string _passwordChannel;
	int			_currentUsers;
	int			_limitUsers;
	bool		_inviteMode;
	bool		_topicMode;

	std::map<int, User*> _usersInChannel;

public:
	Channel(std::string channelName);
	~Channel();
	Channel(Channel const &src);
	const Channel& operator= (Channel const &rhs);

	void setMapUsers(int clientFd, User *user);
	void setChannelName(std::string channelName);
	void setPasswordChannel(std::string password);
	void setCurrentUsers();
	void setlimitUsers(int limitOfUsers);
	void setInviteMode(bool state);
	void setTopicMode(bool state);

	std::string getChannelName() const;
	std::string getPasswordChannel() const;
	int 		getCurrentUsers() const;
	int			getlimitUsers() const;
	bool		getInviteMode() const;
	bool		getTopicMode() const;

};