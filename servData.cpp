/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   servData.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgeisler <mgeisler@student.42mulhouse.f    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/29 10:59:42 by mgeisler          #+#    #+#             */
/*   Updated: 2025/01/29 11:05:16 by mgeisler         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "servData.hpp"

Server::Server(const std::string port, const std::string password) {
	_port = port;
	_password = password;
}

Server::~Server() {}

const std::string Server::getPort() const {
	return (_port);
}
const std::string Server::getPassword() const {
	return (_password);
}