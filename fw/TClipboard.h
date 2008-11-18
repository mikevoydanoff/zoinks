// ========================================================================================
//	TClipboard.h			 	Copyright (C) 2001-2002 Mike Lockwood. All rights reserved.
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

#ifndef __TClipboard__
#define __TClipboard__

#include <X11/X.h>


class TClipboard
{
public:
	static bool			HasData(Atom type);
	static bool			GetData(Atom type, const unsigned char*& outData, uint32& outLength);
	static void			CopyData(Atom type, const unsigned char* data, uint32 length);
};

#endif // __TClipboard__
