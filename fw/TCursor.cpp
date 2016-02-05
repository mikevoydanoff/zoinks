// ========================================================================================
//	TCursor.cpp		 		   Copyright (C) 2001-2002 Mike Voydanoff. All rights reserved.
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

#include "TCursor.h"
#include "TGeometry.h"
#include "TApplication.h"
#include "TColor.h"

#include <X11/Xlib.h>


TCursor::TCursor(unsigned int fontCursorIndex)
	:	fCursor(0)
{
	fCursor =  XCreateFontCursor(gApplication->GetDisplay(), fontCursorIndex);
	ASSERT(fCursor);
}


TCursor::TCursor(char* data, char* mask, const TPoint& hotspot)
	:	fCursor(0)
{
	Display* display = gApplication->GetDisplay();

	Pixmap dataPM = XCreatePixmapFromBitmapData(display, gApplication->GetRootWindow(), data, 16, 16, 1, 0, 1);
	ASSERT(dataPM);
	Pixmap maskPM = XCreatePixmapFromBitmapData(display, gApplication->GetRootWindow(), mask, 16, 16, 1, 0, 1);
	ASSERT(maskPM);

	XColor foreground, background;
	Colormap colorMap = DefaultColormap(display, gApplication->GetDefaultScreen());
	XParseColor(display, colorMap, "black", &foreground);
	XParseColor(display, colorMap, "white", &background);

	fCursor =  XCreatePixmapCursor(display, dataPM, maskPM, &foreground, &background, hotspot.h, hotspot.v);
	ASSERT(fCursor);

	XFreePixmap(display, dataPM);
	XFreePixmap(display, maskPM);
}


TCursor::~TCursor()
{
	XFreeCursor(gApplication->GetDisplay(), fCursor);
}

