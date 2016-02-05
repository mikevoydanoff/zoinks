// ========================================================================================
//	TClipboard.cpp			   Copyright (C) 2001-2002 Mike Voydanoff. All rights reserved.
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

#include "TClipboard.h"

#include <stdlib.h>
#include <string.h>


static Atom 			sDataType = None;
static unsigned char*	sData = NULL;
static uint32 			sDataLength = 0;


bool TClipboard::HasData(Atom type)
{
	return (type == sDataType && sData != NULL);
}


bool TClipboard::GetData(Atom type, const unsigned char*& outData, uint32& outLength)
{
	if (type == sDataType && sDataLength > 0)
	{
		outData = sData;
		outLength = sDataLength;
		return true;
	}
	else
		return false;
}


void TClipboard::CopyData(Atom type, const unsigned char* data, uint32 length)
{
	if (sData)
	{
		free(sData);
		sData = NULL;
	}

	if (data)
	{
		sDataType = type;
		sData = (unsigned char *)malloc(length);
		if (sData)
			memcpy(sData, data, length);
		sDataLength = length;
	}	
}
