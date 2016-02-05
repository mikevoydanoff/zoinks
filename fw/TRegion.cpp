// ========================================================================================
//	TRegion.cpp				   Copyright (C) 2001-2002 Mike Voydanoff. All rights reserved.
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

#include "TRegion.h"


TRegion::TRegion()
	:	fRegion(0)
{
	fRegion =  XCreateRegion();
	ASSERT(fRegion);
}


TRegion::TRegion(const TRect& rect)
	:	fRegion(0)
{
	fRegion =  XCreateRegion();
	ASSERT(fRegion);
	Union(rect);
}


TRegion::TRegion(const TRegion& region)
	:	fRegion(0)
{
	fRegion =  XCreateRegion();
	ASSERT(fRegion);
	Union(&region);
}


TRegion::TRegion(XRectangle& rect)
	:	fRegion(0)
{
	fRegion =  XCreateRegion();
	ASSERT(fRegion);
	XUnionRectWithRegion(&rect, fRegion, fRegion);
}


TRegion::TRegion(Region region)
	:	fRegion(0)
{
	fRegion =  XCreateRegion();
	ASSERT(fRegion);
	XUnionRegion(fRegion, region, fRegion);
}


TRegion::~TRegion()
{
	XDestroyRegion(fRegion);
}


void TRegion::Union(const TRect& rect)
{
	XRectangle	r;
	rect.SetXRectangle(r);
	XUnionRectWithRegion(&r, fRegion, fRegion);
}


void TRegion::Union(const TRegion* region)
{
	XUnionRegion(fRegion, region->GetRegion(), fRegion);
}


void TRegion::Intersect(const TRegion* region)
{
	XIntersectRegion(fRegion, region->GetRegion(), fRegion);
}


void TRegion::Difference(const TRegion* region)
{
	XSubtractRegion(fRegion, region->GetRegion(), fRegion);
}


void TRegion::Xor(const TRegion* region)
{
	XXorRegion(fRegion, region->GetRegion(), fRegion);
}


void TRegion::Offset(TCoord deltaH, TCoord deltaV)
{
	XOffsetRegion(fRegion, deltaH, deltaV);
}


bool TRegion::IsEmpty() const
{
	return XEmptyRegion(fRegion);
}


//void TRegion::SetEmpty();

bool TRegion::Contains(const TPoint& point) const
{
	return XPointInRegion(fRegion, point.h, point.v);
}


bool TRegion::Contains(const TRect& rect) const
{
	return (XRectInRegion(fRegion, rect.left, rect.top, rect.GetWidth(), rect.GetHeight()) == RectangleIn);
}


bool TRegion::Intersects(const TRect& rect) const
{
	int result = XRectInRegion(fRegion, rect.left, rect.top, rect.GetWidth(), rect.GetHeight());
	return (result == RectangleIn || result == RectanglePart);
}









