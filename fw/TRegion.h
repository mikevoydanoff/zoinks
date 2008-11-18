// ========================================================================================
//	TRegion.h				 	Copyright (C) 2001-2002 Mike Lockwood. All rights reserved.
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

#ifndef __TRegion__
#define __TRegion__

#include "TGeometry.h"
#include <X11/Xutil.h>

class TPoint;

class TRegion
{
public:
							TRegion();
							TRegion(const TRect& rect);
							TRegion(const TRegion& region);
							TRegion(XRectangle& rect);
							TRegion(Region region);				
													
	virtual 				~TRegion();

	void					Union(const TRect& rect);
	void					Union(const TRegion* region);
	void					Intersect(const TRegion* region);
	void					Difference(const TRegion* region);
	void					Xor(const TRegion* region);
	void					Offset(TCoord deltaH, TCoord deltaV);

	bool					IsEmpty() const;
	void					SetEmpty();
	bool					Contains(const TPoint& point) const;
	bool					Contains(const TRect& rect) const;
	bool					Intersects(const TRect& rect) const;

	inline Region		 	GetRegion() const { return fRegion; }

protected:
	Region					fRegion;
};


inline int operator==(const TRegion& region1, const TRegion& region2)
{
	return XEqualRegion(region1.GetRegion(), region2.GetRegion());
}


inline int operator!=(const TRegion& region1, const TRegion& region2)
{
	return !XEqualRegion(region1.GetRegion(), region2.GetRegion());
}


#endif // __TRegion__
