/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgeisler <mgeisler@student.42mulhouse.f    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/09 19:33:39 by gloms             #+#    #+#             */
/*   Updated: 2025/02/11 19:00:45 by mgeisler         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include <iostream>
#include <list>
#include "user.hpp"

#define MAX_EVENTS 10

class user;

class Server {

public :
	~Server();
	Server(int port, std::string password);
	Server(const Server &src);

	int serverFd;
	int epollFd;
	struct sockaddr_in address;
	struct epoll_event epollEvents;
	struct epoll_event newClient[MAX_EVENTS];
		
private :
	std::list<User *> _users;
	int _port;
	std::string _password;

	Server();
	const Server &operator = (const Server &rhs);

};