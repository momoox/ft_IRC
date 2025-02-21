/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gloms <rbrendle@student.42mulhouse.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/21 17:17:47 by gloms             #+#    #+#             */
/*   Updated: 2025/02/21 17:27:35 by gloms            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <map>
#include "User.hpp"

class Channel {
public:
	Channel(std::string channelName);
	~Channel();
	Channel(Channel const &src);
	const Channel& operator = (Channel const &rhs);
private:
	std::string _channelName;
	std::map<std::string, User*> _usersConnected;
};
