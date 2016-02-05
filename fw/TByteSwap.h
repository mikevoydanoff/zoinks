// ========================================================================================
//	TByteSwap.h		 		   Copyright (C) 2001-2002 Mike Voydanoff. All rights reserved.
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

#ifndef __TByteSwap__
#define __TByteSwap__


inline uint16 Swap2(uint16 x)
{
        return (x << 8) | (x >> 8);
}

inline uint32 Swap4(uint32 x)
{
        return (x << 24) | ((x & 0x0000FF00) << 8) | ((x & 0x00FF0000) >> 8) | (x >> 24);
}

#endif // __TByteSwap__
