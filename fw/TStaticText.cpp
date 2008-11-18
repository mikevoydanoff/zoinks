// ========================================================================================
//	TStaticText.cpp			 	Copyright (C) 2001-2002 Mike Lockwood. All rights reserved.
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

#include "TStaticText.h"


TStaticText::TStaticText(TWindow* parent, const TRect& bounds, const TChar* text, TTextAlign alignment)
	:	TWindow(parent, bounds, kChildWindow),
		fText(text),
		fAlignment(alignment)
{
}


TStaticText::~TStaticText()
{
}


void TStaticText::SetText(const TChar* text)
{
	fText = text;
	Redraw();
}


void TStaticText::Draw(TRegion* clip)
{
	TRect			bounds;
	GetLocalBounds(bounds);
	TCoord 			width = bounds.GetWidth();

	TDrawContext	context(this, clip);
	const TChar* text = fText;
	const TChar* start = text;
	const TChar* lastSpace = NULL;

	while (1)
	{
		TChar	ch = *text;
		if (ch == ' ')
			lastSpace = text + 1;

		// first check right margin
		TCoord ascent, height;
		if (text > start && context.MeasureText(start, text - start, ascent, height) > width)
		{
			const TChar* end = (lastSpace ? lastSpace : text - 1);
			if (end == start)
				end++;	// in case we are narrower than one character

			bounds.bottom = bounds.top + height;
			context.DrawTextBox(start, end - start, bounds, fAlignment);

			bounds.top = bounds.bottom;
			start = text = end;
			lastSpace = NULL;
		}

		if (*text == 0)
		{
			if (start < text)
			{
				context.MeasureText(start, text - start, ascent, height);
				bounds.bottom = bounds.top + height;
				context.DrawTextBox(start, text - start, bounds, fAlignment);

				bounds.top = bounds.bottom;
			}

			break;	// done drawing text
		}
				
		++text;
	}

	// paint extra space below
	bounds.bottom = GetHeight();
	context.EraseRect(bounds);
}


