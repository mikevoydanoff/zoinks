// ========================================================================================
//	TGeometry.h				 	Copyright (C) 2001-2002 Mike Lockwood. All rights reserved.
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

#ifndef __TGeometry__
#define __TGeometry__

#include <X11/Xlib.h>


typedef int32 TCoord;


class TPoint
{
public:
				TPoint();
				TPoint(TCoord h, TCoord v);
				TPoint(const TPoint& p);
	
	void		Set(TCoord h, TCoord v);
	void		Set(const TPoint& p);
	void		Offset(TCoord h, TCoord v);
	void		Offset(const TPoint& point);
	
public:
	TCoord		h;
	TCoord		v;
};

extern TPoint	gZeroPoint;

class TRect
{
public:
				TRect();
				TRect(TCoord left, TCoord top, TCoord right, TCoord bottom);
				TRect(const TPoint& p1, const TPoint& p2);
				TRect(const TRect& r);
				TRect(const XRectangle& r);
	
	TCoord 		GetWidth() const;
	TCoord 		GetHeight() const;

	void		SetWidth(TCoord newWidth);
	void		SetHeight(TCoord newHeight);

	void		SetXRectangle(XRectangle& r) const;
	
	void		Set(TCoord left, TCoord top, TCoord right, TCoord bottom);
	void		Set(const TPoint& topLeft, const TPoint&  bottomRight);
	void		Set(const TRect& r);
	void		Set(const XRectangle& r);
	void		Inset(TCoord h, TCoord v);
	void		Inset(TCoord left, TCoord top, TCoord right, TCoord bottom);
	void		Inset(const TRect& rect);
	void		Offset(TCoord h, TCoord v);
	void		Offset(const TPoint& point);

	void		MoveTo(TCoord h, TCoord v);
	void		CenterIn(const TRect& rect);

	void		UnionWith(const TRect& rect);
	void		IntersectWith(const TRect& rect);

	bool		IsEmpty() const;
	void		SetEmpty();
	bool		Contains(const TPoint& point) const;

public:
	TCoord		left;
	TCoord		top;
	TCoord		right;
	TCoord		bottom;
};


extern TRect	gZeroRect;


inline TPoint::TPoint()
	:	h(0),
		v(0)
{
}


inline TPoint::TPoint(TCoord hValue, TCoord vValue)
	:	h(hValue),
		v(vValue)
{
}


inline TPoint::TPoint(const TPoint& p)
	:	h(p.h),
		v(p.v)
{
}


inline void TPoint::Set(TCoord newH, TCoord newV)
{
	h = newH;
	v = newV;
}


inline void TPoint::Set(const TPoint& p)
{
	h = p.h;
	v = p.v;
}


inline void TPoint::Offset(TCoord dh, TCoord dv)
{
	h += dh;
	v += dv;
}


inline void TPoint::Offset(const TPoint& point)
{
	h += point.h;
	v += point.v;
}


inline int operator==(const TPoint& p1, const TPoint& p2)
{
	return (p1.h == p2.h &&
			p1.v == p2.v);
}


inline int operator!=(const TPoint& p1, const TPoint& p2)
{
	return (p1.h != p2.h ||
			p1.v != p2.v);
}


inline TRect::TRect()
	:	left(0),
		top(0),
		right(0),
		bottom(0)
{
}


inline TRect::TRect(TCoord leftValue, TCoord topValue, TCoord rightValue, TCoord bottomValue)
	:	left(leftValue),
		top(topValue),
		right(rightValue),
		bottom(bottomValue)
{
}


inline TRect::TRect(const TPoint& p1, const TPoint& p2)
	:	left(p1.h < p2.h ? p1.h : p2.h),
		top(p1.v < p2.v ? p1.v : p2.v),
		right(p1.h > p2.h ? p1.h : p2.h),
		bottom(p1.v > p2.v ? p1.v : p2.v)
{
}


inline TRect::TRect(const TRect& r)
	:	left(r.left),
		top(r.top),
		right(r.right),
		bottom(r.bottom)
{
}


inline TRect::TRect(const XRectangle& r)
	:	left(r.x),
		top(r.y),
		right(r.x + r.width),
		bottom(r.y + r.height)
{
}


inline TCoord TRect::GetWidth() const
{
	return right - left;
}


inline TCoord TRect::GetHeight() const
{
	return bottom - top;
}


inline void TRect::SetWidth(TCoord newWidth)
{
	right = left + newWidth;
}


inline void TRect::SetHeight(TCoord newHeight)
{
	bottom = top + newHeight;
}


inline void TRect::SetXRectangle(XRectangle& r) const
{
	r.x = left;
	r.y = top;
	r.width = GetWidth();
	r.height = GetHeight();
}


inline void TRect::Set(TCoord newLeft, TCoord newTop, TCoord newRight, TCoord newBottom)
{
	left = newLeft;
	top = newTop;
	right = newRight;
	bottom = newBottom;
}


inline void TRect::Set(const TPoint& topLeft, const TPoint&  bottomRight)
{
	left = topLeft.h;
	top = topLeft.v;
	right = bottomRight.h;
	bottom = bottomRight.v;
}


inline void TRect::Set(const TRect& r)
{
	left = r.left;
	top = r.top;
	right = r.right;
	bottom = r.bottom;
}


inline void TRect::Set(const XRectangle& r)
{
	left = r.x;
	top = r.y;
	right = r.x + r.width;
	bottom = r.y + r.height;
}


inline void TRect::Inset(TCoord dh, TCoord dv)
{
	left += dh;
	top += dv;
	right -= dh;
	bottom -= dv;
}


inline void TRect::Inset(TCoord l, TCoord t, TCoord r, TCoord b)
{
	left += l;
	top += t;
	right -= r;
	bottom -= b;
}


inline void TRect::Inset(const TRect& rect)
{
	left += rect.left;
	top += rect.top;
	right -= rect.right;
	bottom -= rect.bottom;
}


inline void TRect::Offset(TCoord dh, TCoord dv)
{
	left += dh;
	top += dv;
	right += dh;
	bottom += dv;
}


inline void TRect::Offset(const TPoint& point)
{
	left += point.h;
	top += point.v;
	right += point.h;
	bottom += point.v;
}


inline void	TRect::MoveTo(TCoord h, TCoord v)
{
	right += (h - left);
	bottom += (v - top);
	left = h;
	top = v;
}


inline void	TRect::CenterIn(const TRect& rect)
{
	MoveTo(rect.left + (rect.GetWidth() - GetWidth()) / 2, 
		   rect.top + (rect.GetHeight() - GetHeight()) / 2);
}


inline bool TRect::IsEmpty() const
{
	return (right <= left || bottom <= top);
}


inline void TRect::SetEmpty()
{
	left = top = right = bottom = 0;
}


inline bool	TRect::Contains(const TPoint& point) const
{
	return (point.h >= left && point.h < right &&
			point.v >= top && point.v < bottom);
} 


inline int operator==(const TRect& r1, const TRect& r2)
{
	return (r1.left == r2.left &&
			r1.top == r2.top &&
			r1.right == r2.right && 
			r1.bottom == r2.bottom);
}


inline int operator!=(const TRect& r1, const TRect& r2)
{
	return (r1.left != r2.left ||
			r1.top != r2.top ||
			r1.right != r2.right || 
			r1.bottom != r2.bottom);
}

#endif // __TGeometry__
