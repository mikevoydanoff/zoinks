// ========================================================================================
//	TView.h		 				Copyright (C) 2001-2002 Mike Lockwood. All rights reserved.
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

#ifndef __TView__
#define __TView__

#include "TWindow.h"

enum TScrollDirection
{
	kScrollLeft,
	kScrollRight,
	kScrollUp,
	kScrollDown
};


class TView : public TWindow
{
public:
							TView(TWindow* parent, const TRect& bounds);

	inline const TPoint&	GetContentSize() const { return fContentSize; }

	virtual const TPoint&	GetScroll() const;
	
	void					GetMaxScroll(TPoint& maxScroll);
	inline void				GetContentBounds(TRect& bounds) const { bounds.Set(0, 0, fContentSize.h, fContentSize.v); }
	inline void				GetVisibleBounds(TRect& bounds) const { bounds.Set(fScroll.h, fScroll.v, GetWidth() + fScroll.h, GetHeight() + fScroll.v); }
	virtual void			GetScrollableBounds(TRect& bounds) const;
	
	virtual void			ScrollBy(TCoord dh, TCoord dv, bool notifyParent = true);
	inline void				ScrollTo(TCoord h, TCoord v) { ScrollBy(h - fScroll.h, v - fScroll.v); }
	inline void				ScrollToTop() { ScrollBy(-fScroll.h, -fScroll.v); }
	void					ScrollIntoView(const TRect& r);

	virtual TCoord			GetScrollIncrement(TScrollDirection direction) const;
	virtual TCoord			GetPageIncrement(TScrollDirection direction) const;

//	virtual void			Draw(TRegion* clip);
	
protected:				
	virtual					~TView();

	virtual void			SetContentSize(const TPoint& contentSize); 
	virtual void			EraseContentDifference(const TPoint& oldContentSize, const TPoint& newContentSize);
	virtual void			NotifyBoundsChanged(const TRect& oldBounds);

protected:
	TPoint					fContentSize;
	TPoint					fScroll;
};

#endif // __TView__
