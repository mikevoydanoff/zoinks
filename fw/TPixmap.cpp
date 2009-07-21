// ========================================================================================
//	TPixmap.cpp				 	Copyright (C) 2001-2002 Mike Lockwood. All rights reserved.
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

#include "TPixmap.h"
#include "TGeometry.h"
#include "TApplication.h"
#include "TColor.h"

#include <X11/xpm.h>
#include <string.h>


TPixmap::TPixmap(const char** xpmData)
	:	fPixmap(0),
		fMask(0)
{
	XpmAttributes	attributes;
	attributes.valuemask = XpmCloseness;
	attributes.closeness = 40000;
	
   int result = XpmCreatePixmapFromData(gApplication->GetDisplay(), gApplication->GetRootWindow(),
   									(char **)xpmData, &fPixmap, &fMask, &attributes);
   	ASSERT(result == 0);
   	
   	// Fill out TDrawable values
   	fBounds.right = attributes.width;
   	fBounds.bottom = attributes.height;
   	fDepth = 0;	// fix me
}


TPixmap::TPixmap(char* xpmFilePath)
	:	fPixmap(0),
		fMask(0)
{
	XpmAttributes	attributes;
	attributes.valuemask = XpmCloseness;
	attributes.closeness = 40000;
	
   int result = XpmReadFileToPixmap(gApplication->GetDisplay(), gApplication->GetRootWindow(),
   									xpmFilePath, &fPixmap, &fMask, &attributes);
   	ASSERT(result == 0);
   	fBounds.right = attributes.width;
   	fBounds.bottom = attributes.height;
   	fDepth = 0;	// fix me
}


TPixmap::~TPixmap()
{
	XFreePixmap(gApplication->GetDisplay(), fPixmap);
	XFreePixmap(gApplication->GetDisplay(), fMask);
}


Drawable TPixmap::GetDrawable() const
{
	return fPixmap;
}


