// ========================================================================================
//	TSocket.cpp				 	Copyright (C) 2001-2002 Mike Lockwood. All rights reserved.
// ========================================================================================
/*
	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.
	
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
	
	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "FWCommon.h"

#include "TSocket.h"

#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/socket.h>

#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

#if !HAVE_IN_ADDR_T
typedef int in_addr_t;
#endif


TSocket::TSocket()
	:	fSocket(0)
{
}


TSocket::~TSocket()
{
	Close();
}


void TSocket::Connect(const char* hostName, int portNumber)
{
	struct sockaddr_in		address;
	memset(&address, 0, sizeof(address));
	
	hostent* host = gethostbyname(hostName);
	
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = (host ? *(in_addr_t*)host->h_addr : 0);
	address.sin_port = htons(portNumber);
	
	fSocket = socket(AF_INET, SOCK_STREAM, 0);
	ASSERT(fSocket > 0);

	int result = connect(fSocket, (sockaddr *)&address, sizeof(address));
	ASSERT(result == 0);
}


int TSocket::Read(void* buffer, int length)
{
	return recv(fSocket, buffer, length, 0);
}


int TSocket::Write(const void* buffer, int length)
{
	return send(fSocket, buffer, length, 0);
}


void TSocket::Close()
{
	if (fSocket > 0)
		close(fSocket);
}
