/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgeisler <mgeisler@student.42mulhouse.f    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/09 19:33:07 by gloms             #+#    #+#             */
/*   Updated: 2025/02/11 19:03:45 by mgeisler         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "irc.hpp"

int main(int ac, char **av)
{
	if (ac != 3) {
		std::cerr << "Wrong number of arguments. Should have a port and a password." << std::endl;
		return (1);
	}

	int port = atoi(av[1]);
	std::string password = av[2];
	
	try {
		Server serverOn(port, password);
		std::cout << "creation obj serverOn" << std::endl;
		init_epoll(serverOn);
		std::cout << "epoll init" << std::endl;
		main_loop(serverOn);
	}

	catch (const std::exception &e) {
		std::cerr << e.what() << std::endl;
	}
	return 0;
}
