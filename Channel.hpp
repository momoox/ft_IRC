#pragma once

#include "Server.hpp"

class User;
class Server;

class Channel {
private:
	std::string _channelName;
	std::string _passwordChannel;
	std::string _topic;
	int			_currentUsers;
	int			_limitUsers;
	bool		_inviteMode;
	bool		_topicMode;

	std::map<int, User*> _usersInChannel;

public:
	Channel(std::string channelName);
	~Channel();
	Channel(Channel const &src);
	const Channel& operator = (Channel const &rhs);

	void setMapUsers(int clientFd, User *user);
	void setUserOp(int clientFd, bool state);
	void setChannelName(std::string channelName);
	void setTopic(std::string topic);
	void setPasswordChannel(std::string password);
	void setCurrentUsers(std::string set);
	void setLimitUsers(int limitOfUsers);
	void setInviteMode(bool state);
	void setTopicMode(bool state);
	void eraseUserInChannel(int clientFd);

	std::string getChannelName() const;
	std::string getPasswordChannel() const;
	std::string getTopic() const;
	std::map<int, User*>&	getUsersMap() const;
	int 		getCurrentUsers() const;
	int			getlimitUsers() const;
	bool		getInviteMode() const;
	bool		getTopicMode() const;

	void	kickUserFromChannel(int clientFd);
	void	sendAllUsers(std::string message, int clientFd);

};
