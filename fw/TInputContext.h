// ========================================================================================
//	TInputContext.h			   Copyright (C) 2001-2002 Mike Voydanoff. All rights reserved.
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

#ifndef __TInputContext__
#define __TInputContext__

#include <X11/Xlib.h>

class TPoint;
class TWindow;

class TInputContext
{
public:
							TInputContext(XIC xic);
	virtual					~TInputContext();
						
	void					SetFocus(TWindow* window);
	void					ClearFocus();
	void					SetSpotLocation(const TPoint& point);

	inline XIC				GetXIC() const { return fXIC; }
	static inline TWindow*	GetFocusedWindow() { return sFocusedWindow; }
					
private:
	XIC						fXIC;
	static TWindow*			sFocusedWindow;
};

#endif // __TInputContext__
