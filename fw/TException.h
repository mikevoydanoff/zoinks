// ========================================================================================
//	TException.h			 	Copyright (C) 2001-2002 Mike Lockwood. All rights reserved.
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

#ifndef __TException__
#define __TException__

#include "TCommandID.h"
#include "TWindow.h"

#include <errno.h>


class TException
{
public:
						TException();
	virtual				~TException();

protected:	

};

class TSystemError : public TException
{
public:
							TSystemError(int error);
	virtual					~TSystemError();
	
	inline int				GetError() const { return fError; }

protected:
	int						fError;
};


class TProgramError : public TException
{
public:
							TProgramError(const char* message);
	virtual					~TProgramError();
	
	inline const TString&	GetMessage() const { return fMessage; }

protected:
	TString					fMessage;
};


class TUserCancelled : public TException
{
public:
							TUserCancelled();
	virtual					~TUserCancelled();
};


inline void ThrowSystemError()
{
	int error = errno;
	if (error != 0)
		throw (new TSystemError(error));
}


inline void ThrowSystemError(int error)
{
	if (error != 0)
		throw (new TSystemError(error));
}


inline void ThrowProgramError(const char* message)
{
	throw (new TProgramError(message));
}


inline void ThrowUserCancelled()
{
	throw(new TUserCancelled);
}

#endif // __TException__
