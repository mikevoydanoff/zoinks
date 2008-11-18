// ========================================================================================
//	TInputContext.cpp		 	Copyright (C) 2001-2002 Mike Lockwood. All rights reserved.
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

#include "TInputContext.h"
#include "TGeometry.h"
#include "TWindow.h"
#include "TFont.h"

struct ICArg
{
	const char* name;
	void*		value;
};


TWindow* TInputContext::sFocusedWindow = NULL;


TInputContext::TInputContext(XIC xic)
	:	fXIC(xic)
{
}


TInputContext::~TInputContext()
{
	char* result = XmbResetIC(fXIC);
		if (result)
			XFree(result);

	XDestroyIC(fXIC);
}


void TInputContext::SetFocus(TWindow* window)
{
	XSetICFocus(fXIC);		
	sFocusedWindow = window;
}


void TInputContext::ClearFocus()
{		
	XUnsetICFocus(fXIC);
	sFocusedWindow = NULL;
}


void TInputContext::SetSpotLocation(const TPoint& point)
{
	XPoint xpoint;
	xpoint.x = point.h;
	xpoint.y = point.v;
	
	ICArg args[2] = { {XNSpotLocation, &xpoint}, {NULL, NULL} };
	XSetICValues(fXIC, XNPreeditAttributes, args, NULL);
}
