// ========================================================================================
//	TDrawable.h		 		   Copyright (C) 2001-2002 Mike Voydanoff. All rights reserved.
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

#ifndef __TDrawable__
#define __TDrawable__

#include "TGeometry.h"

#include <X11/Xlib.h>

class TDrawContext;

class TDrawable
{
public:
								TDrawable();				
								TDrawable(const TRect& bounds);				
	virtual						~TDrawable();

	virtual Drawable			GetDrawable() const = 0;

	virtual const TPoint&	 	GetScroll() const;

	virtual void			 	InitDrawContext(TDrawContext& context);

	inline const TRect&			GetBounds() const { return fBounds; }
	inline TCoord				GetWidth() const { return fBounds.GetWidth(); }
	inline TCoord				GetHeight() const { return fBounds.GetHeight(); }
	inline int					GetDepth() const { return fDepth; }

protected:
	TRect						fBounds;
	int							fDepth;
};

#endif // __TDrawable__
