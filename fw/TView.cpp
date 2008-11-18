// ========================================================================================
//	TView.cpp				 	Copyright (C) 2001-2002 Mike Lockwood. All rights reserved.
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

#include "TView.h"
#include "TDrawContext.h"
#include "TRegion.h"


TView::TView(TWindow* parent, const TRect& bounds)
	:	TWindow(parent, bounds, kChildWindow)
{
}


TView::~TView()
{
}


void TView::SetContentSize(const TPoint& contentSize)
{
	ASSERT(contentSize.h >= 0 && contentSize.v >= 0);
	
	if (fContentSize != contentSize)
	{
		TPoint oldContentSize(fContentSize);
		fContentSize = contentSize;

		TPoint maxScroll;
		GetMaxScroll(maxScroll);

		TCoord adjustH = (fScroll.h > maxScroll.h ? maxScroll.h - fScroll.h : 0);
		TCoord adjustV = (fScroll.v > maxScroll.v ? maxScroll.v - fScroll.v : 0);

		if (adjustH < 0 || adjustV < 0)
			ScrollBy(adjustH, adjustV);

		if (IsVisible())
			EraseContentDifference(oldContentSize, contentSize);

		if (fParent)
			fParent->AdjustScrollBars(this);
	}
}


void TView::EraseContentDifference(const TPoint& oldContentSize, const TPoint& newContentSize)
{
	if (oldContentSize.h > newContentSize.h || oldContentSize.v > newContentSize.v)
	{
		TDrawContext context(this);
		
		if (oldContentSize.h > newContentSize.h)
		{
			TRect	r(newContentSize.h, 0, oldContentSize.h, (oldContentSize.v > newContentSize.v ? oldContentSize.v : newContentSize.v));
			context.EraseRect(r);
		}
		if (oldContentSize.v > newContentSize.v)
		{
			TRect	r(0, newContentSize.v, (oldContentSize.h > newContentSize.h ? oldContentSize.h : newContentSize.h), oldContentSize.v);
			context.EraseRect(r);
		}
	}
}


const TPoint& TView::GetScroll() const
{
	return fScroll;
}


void TView::GetMaxScroll(TPoint& maxScroll)
{
	maxScroll.h = fContentSize.h - fBounds.GetWidth();
	if (maxScroll.h < 0)
		maxScroll.h = 0;

	maxScroll.v = fContentSize.v - fBounds.GetHeight();
	if (maxScroll.v < 0)
		maxScroll.v = 0;
}


void TView::GetScrollableBounds(TRect& bounds) const
{
	GetLocalBounds(bounds);
}


void TView::NotifyBoundsChanged(const TRect& oldBounds)
{
	TWindow::NotifyBoundsChanged(oldBounds);

	// adjust fScroll if necessary.
	TPoint maxScroll;
	GetMaxScroll(maxScroll);
	
	TCoord dh = maxScroll.h - fScroll.h;
	if (dh > 0)
		dh = 0;

	TCoord dv = maxScroll.v - fScroll.v;
	if (dv > 0)
		dv = 0;

	if (dh || dv)
		ScrollBy(dh, dv);

	if (fParent)
		fParent->AdjustScrollBars(this);
}


void TView::ScrollBy(TCoord dh, TCoord dv, bool notifyParent)
{
	TPoint maxScroll;
	GetMaxScroll(maxScroll);

	TPoint oldScroll(fScroll);

	fScroll.h += dh;
	if (fScroll.h < 0)
		fScroll.h = 0;
	else if (fScroll.h > maxScroll.h)
		fScroll.h = maxScroll.h;

	fScroll.v += dv;
	if (fScroll.v < 0)
		fScroll.v = 0;
	else if (fScroll.v > maxScroll.v)
		fScroll.v = maxScroll.v;

	TCoord deltaH = fScroll.h - oldScroll.h;
	TCoord deltaV = fScroll.v - oldScroll.v;
	
	if (deltaH || deltaV)
	{
		TRect	scrollableBounds;
		GetScrollableBounds(scrollableBounds);
		TRect	src(scrollableBounds);

		src.Offset(deltaH, deltaV);
		src.IntersectWith(scrollableBounds);

		if (fObscured || src.IsEmpty())
		{
			Redraw();			
		}
		else
		{			
			TDrawContext	context(this);
			context.CopyRect(this, src, TPoint(src.left - deltaH, src.top - deltaV), false);

			if (deltaH > 0)
			{
				TRect	r(scrollableBounds.right - deltaH, scrollableBounds.top, scrollableBounds.right, scrollableBounds.bottom);
				r.Offset(fScroll);
				RedrawRect(r);
			}
			else if (deltaH < 0)
			{
				TRect	r(scrollableBounds.left, scrollableBounds.top, scrollableBounds.left - deltaH, scrollableBounds.bottom);
				r.Offset(fScroll);
				RedrawRect(r);
			}

			if (deltaV > 0)
			{
				TRect	r(scrollableBounds.left, scrollableBounds.bottom - deltaV, scrollableBounds.right, scrollableBounds.bottom);
				r.Offset(fScroll);
				RedrawRect(r);
			}
			else if (deltaV < 0)
			{
				TRect	r(scrollableBounds.left, scrollableBounds.top, scrollableBounds.right, scrollableBounds.top - deltaV);
				r.Offset(fScroll);
				RedrawRect(r);
			}
		}

		TListIterator<TWindow>	iter(fChildren);
		TWindow* child;
		
		while ((child = iter.Next()) != NULL)
		{
			TRect bounds = child->GetBounds();
			bounds.Offset(-deltaH, -deltaV);
			child->SetBounds(bounds);
		}

		if (notifyParent && fParent)
			fParent->ChildScrollChanged(this, deltaH, deltaV);	
	}
}


void TView::ScrollIntoView(const TRect& r)
{
	TCoord deltaH = 0;
	TCoord deltaV = 0;
	TRect	bounds;
	GetVisibleBounds(bounds);

/*
 * disabled until we fix TListView so the cell width isn't 10000
 	if (r.left < bounds.left)
		deltaH = r.left - bounds.left;
	else if (r.right > bounds.right)
		deltaH = r.right - bounds.right;
*/
	if (r.top < bounds.top)
		deltaV = r.top - bounds.top;
	else if (r.bottom > bounds.bottom)
		deltaV = r.bottom - bounds.bottom;

	if (deltaH || deltaV)
		ScrollBy(deltaH, deltaV);	
}


TCoord TView::GetScrollIncrement(TScrollDirection direction) const
{
	switch (direction)
	{
		case kScrollLeft:
		case kScrollUp:
			return -10;
			
		case kScrollRight:
		case kScrollDown:
			return 10;
	}

	ASSERT(0);
	return 0;
}


TCoord TView::GetPageIncrement(TScrollDirection direction) const
{
	switch (direction)
	{
		case kScrollLeft:
			return -GetWidth();
			
		case kScrollRight:
			return GetWidth();
			
		case kScrollUp:
			return -GetHeight();
			
		case kScrollDown:
			return GetHeight();
	}

	ASSERT(0);
	return 0;
}




