/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   servData.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgeisler <mgeisler@student.42mulhouse.f    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/29 10:45:03 by mgeisler          #+#    #+#             */
/*   Updated: 2025/01/29 11:04:18 by mgeisler         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>

class Server {
private:
	std::string _port;
	std::string _password;

public:
	Server(const std::string port, const std::string password);
	~Server();

	const std::string getPort() const;
	const std::string getPassword() const;

};