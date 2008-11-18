// ========================================================================================
//	TSplitter.cpp			 	Copyright (C) 2001-2002 Mike Lockwood. All rights reserved.
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

#include "TSplitter.h"
#include "TDrawContext.h"
#include "TColor.h"
#include "TCursor.h"
#include "TFWCursors.h"


const TCoord kSplitterResizeMargin = 30;
const TCoord kSplitterWidth = 6;


TSplitter::TSplitter(TWindow* parent, const TRect& bounds,
							 bool vertical, bool liveDrag, float ratio)
	:	TWindow(parent, bounds),
		fChild1(NULL),
		fChild2(NULL),
		fVertical(vertical),
		fRatio(ratio),
		fTracking(false),
		fLiveDrag(liveDrag)
{
	ASSERT(ratio >= 0.0 && ratio <= 1.0);
}



TSplitter::~TSplitter()
{
}


void TSplitter::SetChildren(TWindow* child1, TWindow* child2)
{
	ASSERT(!fChild1 && !fChild2);
	
	fChild1 = child1;
	fChild2 = child2;

	if (child1)
	{
		ASSERT(child1->GetParent() == this);
		//child1->SetParent(this);
	}
	if (child2)
	{
		ASSERT(child2->GetParent() == this);
		//child2->SetParent(this);
	}

	ResizeChildren();
}


void TSplitter::ResizeChildren()
{
	if (fChild1)
	{
		TRect bounds;
		GetChild1Rect(bounds);
		fChild1->SetBounds(bounds);

	}
	if (fChild2)
	{
		TRect bounds;
		GetChild2Rect(bounds);
		fChild2->SetBounds(bounds);
	}
}


void TSplitter::GetChild1Rect(TRect& rect) const
{
	TRect	splitter;	
	GetSplitterRect(fRatio, splitter);
	
	GetLocalBounds(rect);

	if (fVertical)
		rect.right = splitter.left;
	else
		rect.bottom = splitter.top;
}


void TSplitter::GetChild2Rect(TRect& rect) const
{
	TRect	splitter;
	
	GetSplitterRect(fRatio, splitter);
	GetLocalBounds(rect);

	if (fVertical)
		rect.left = splitter.right - 1;
	else
		rect.top = splitter.bottom - 1;
}


void TSplitter::GetSplitterRect(float ratio, TRect& rect) const
{
	ASSERT(ratio >= 0.0 && ratio <= 1.0);

	GetLocalBounds(rect);

	if (fVertical)
	{
		rect.left = (TCoord)((rect.GetWidth() - kSplitterWidth) * fRatio);
		rect.right = rect.left + kSplitterWidth;
	}
	else
	{
		rect.top = (TCoord)((rect.GetHeight() - kSplitterWidth) * fRatio);
		rect.bottom = rect.top + kSplitterWidth;
	}
}


void TSplitter::Create()
{
	TWindow::Create();

	TCursor* cursor;

	if (fVertical)
		cursor = GetVertSplitterCursor();
	else
		cursor = GetHorizSplitterCursor();

	ASSERT(cursor);
	SetCursor(cursor);
}


void TSplitter::Draw(TRegion* clip)
{
	TDrawContext	context(this, clip);
	DrawSplitterRect(context);
}


void TSplitter::DoMouseDown(const TPoint& mouse, TMouseButton button, TModifierState state)
{
	if (! fTracking && button == kLeftButton)
	{	
		TPoint point;
		ConstrainMouse(mouse, point);

		if (!fLiveDrag)
		{
			TRect	rect;
			GetTrackingRect(point, rect);
			DrawTrackingRect(rect);
		}

		fTracking = true;
		fLastMouse = point;
	}
}


void TSplitter::DoMouseUp(const TPoint& mouse, TMouseButton button, TModifierState state)
{
	if (fTracking && button == kLeftButton)
	{
		TPoint point;
		ConstrainMouse(mouse, point);

		if (!fLiveDrag)
		{
			TRect	rect;
			GetTrackingRect(fLastMouse, rect);
			DrawTrackingRect(rect);

			TRect bounds;
			GetLocalBounds(bounds);

			if (fVertical)
				fRatio = (float)point.h / (float)bounds.GetWidth();	
			else
				fRatio = (float)point.v / (float)bounds.GetHeight();
			
			if (fRatio < 0.0)
				fRatio = 0.0;
			else if (fRatio > 1.0)
				fRatio = 1.0;

			ResizeChildren();
		}

		fTracking = false;
	}
}


void TSplitter::DoMouseMoved(const TPoint& mouse, TModifierState state)
{
	if (fTracking)
	{
		TPoint point;
		ConstrainMouse(mouse, point);

		if ((fVertical && fLastMouse.h != point.h) ||
			(!fVertical && fLastMouse.v != point.v))
		{
			if (fLiveDrag)
			{
				TRect bounds;
				GetLocalBounds(bounds);

				if (fVertical)
					fRatio = (float)point.h / (float)bounds.GetWidth();	
				else
					fRatio = (float)point.v / (float)bounds.GetHeight();
				
				if (fRatio < 0.0)
					fRatio = 0.0;
				else if (fRatio > 1.0)
					fRatio = 1.0;

				ResizeChildren();

				TDrawContext	context(this);
				DrawSplitterRect(context);
			}
			else
			{
				TRect	rect;

				GetTrackingRect(fLastMouse, rect);
				DrawTrackingRect(rect);
				GetTrackingRect(point, rect);
				DrawTrackingRect(rect);
			}
		}

		fLastMouse = point;
	}
}


void TSplitter::ConstrainMouse(const TPoint& mouse, TPoint& newPoint)
{
	newPoint = mouse;
	
	if (fVertical)
	{
		if (mouse.h < kSplitterResizeMargin)
			newPoint.h = kSplitterResizeMargin;
		else if (mouse.h + kSplitterResizeMargin > fBounds.GetWidth())
			newPoint.h =  fBounds.GetWidth() - kSplitterResizeMargin;
	}
	else
	{
		if (mouse.v < kSplitterResizeMargin)
			newPoint.v = kSplitterResizeMargin;
		else if (mouse.v + kSplitterResizeMargin > fBounds.GetHeight())
			newPoint.v =  fBounds.GetHeight() - kSplitterResizeMargin;
	}
}


void TSplitter::GetTrackingRect(const TPoint& point, TRect& rect)
{
	GetLocalBounds(rect);

	if (fVertical)
	{
		rect.left = point.h - kSplitterWidth/2;
		rect.right = rect.left + kSplitterWidth;

		if (rect.left < 0)
			rect.Offset(-rect.left, 0);
		else if (rect.right > fBounds.GetWidth())
			rect.Offset(fBounds.GetWidth() - rect.right, 0);
	}
	else
	{
		rect.top = point.v - kSplitterWidth/2;
		rect.bottom = rect.top + kSplitterWidth;

		if (rect.top < 0)
			rect.Offset(0, -rect.top);
		else if (rect.bottom > fBounds.GetHeight())
			rect.Offset(0, fBounds.GetHeight() - rect.bottom);
	}
}


void TSplitter::DrawTrackingRect(const TRect& rect)
{	
	TDrawContext context(this);
	context.SetForeColor(kMediumGrayColor);
	
	context.ClipSubWindows(false);
	context.SetPenMode(kXorMode);
	context.PaintRect(rect);
}


void TSplitter::DrawSplitterRect(TDrawContext& context)
{
	context.SetForeColor(kMediumGrayColor);

	TRect r;
	GetSplitterRect(fRatio, r);

	if (fVertical)
	{
		context.SetForeColor(kWhiteColor);
		context.DrawLine(r.left, r.top, r.left, r.bottom);
		
		r.left += 1;
		r.right -= 1;

		context.SetForeColor(kLightGrayColor);
		context.PaintRect(r);

		context.SetForeColor(kMediumGrayColor);
		context.DrawLine(r.right - 1, r.top, r.right - 1, r.bottom);
	}
	else
	{
		context.SetForeColor(kWhiteColor);
		context.DrawLine(r.left, r.top, r.right, r.top);
		
		r.top += 1;
		r.bottom -= 1;

		context.SetForeColor(kLightGrayColor);
		context.PaintRect(r);

		context.SetForeColor(kMediumGrayColor);
		context.DrawLine(r.left, r.bottom - 1, r.right, r.bottom - 1);
	}
}


void TSplitter::NotifyBoundsChanged(const TRect& /*oldBounds*/)
{
	ResizeChildren();
}

