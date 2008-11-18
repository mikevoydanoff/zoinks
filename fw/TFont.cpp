// ========================================================================================
//	TFont.cpp				 	Copyright (C) 2001-2002 Mike Lockwood. All rights reserved.
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

#include "TFont.h"
#include "TApplication.h"

#include <stdlib.h>


TFont::TFont(const char* fontNameList)
	:	fFontSet(0)
{
	char**			missing;
	int				missingCount;
	char*			defString;

	fFontSet = XCreateFontSet(gApplication->GetDisplay(), fontNameList, &missing, &missingCount, &defString);
	ASSERT(fFontSet);
}


TFont::~TFont()
{
	XFreeFontSet(gApplication->GetDisplay(), fFontSet);
}


TCoord TFont::MeasureText(const TChar* text, int length) const
{
	if (length == 0)
		length = Tstrlen(text);

	const TChar* start = text;
	TCoord result = 0;
	
	// avoid invalid characters
	while (length > 0)
	{
		int charLength = mblen(text, length);
		if (charLength > 0)
		{
			text += charLength;
			length -= charLength;
		}
		else
		{
			result += XmbTextExtents(fFontSet, (const char *)start, text - start, NULL, NULL);
			result += XmbTextExtents(fFontSet, " ", 1, NULL, NULL);
			text += 1;
			start = text;
			length -= 1;
		}
	}
	
	if (text > start)
		result += XmbTextExtents(fFontSet, (const char *)start, text - start, NULL, NULL);

	return result;
}


TCoord TFont::MeasureText(const TChar* text, int length, TCoord& ascent, TCoord& height) const
{
	if (length == 0)
		length = Tstrlen(text);
	
	const TChar* start = text;
	TCoord result = 0;

	ascent = 0;
	height = 0;
	
	// avoid invalid characters
	while (length > 0)
	{
		int charLength = mblen(text, length);
		if (charLength > 0)
		{
			text += charLength;
			length -= charLength;
		}
		else
		{
			XRectangle	logical;
			result += XmbTextExtents(fFontSet, (const char *)start, text - start, NULL, &logical);
			
			if (-logical.y > ascent)
				ascent = -logical.y;
			if (logical.height > height)
				height = logical.height;

			result += XmbTextExtents(fFontSet, " ", 1, NULL, NULL);

			text += 1;
			start = text;
			length -= 1;
		}
	}
	
	if (text > start)
	{
		XRectangle	logical;
		result += XmbTextExtents(fFontSet, (const char *)start, text - start, NULL, &logical);
		
		if (-logical.y > ascent)
			ascent = -logical.y;
		if (logical.height > height)
			height = logical.height;	
	}

	ASSERT(ascent < 200);
	ASSERT(height < 200);
	
	return result;
}
