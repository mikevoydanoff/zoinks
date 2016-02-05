// ========================================================================================
//	TScroller.cpp			   Copyright (C) 2001-2002 Mike Voydanoff. All rights reserved.
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

#include "TScroller.h"
#include "TScrollBar.h"
#include "TView.h"
#include "TDrawContext.h"
#include "TColor.h"
#include "TWindowPositioners.h"

const TCoord kScrollBarWidth = 16;

TScroller::TScroller(TWindow* parent, const TRect& bounds,
						bool horizScrollBar, bool vertScrollBar)
	:	TWindow(parent, bounds),
		fContainedView(NULL),
		fHorizScrollBar(NULL),
		fVertScrollBar(NULL),
		fHasHorizScrollBar(horizScrollBar),
		fHasVertScrollBar(vertScrollBar),
		fNeedsAdjust(false)
{
	ASSERT(horizScrollBar || vertScrollBar);
}


TScroller::~TScroller()
{
}


void TScroller::Create()
{
	TWindow::Create();

	// note we do not show the scroll bars here.  
	// wait for AdjustScrollBars to do it.
	if (fHasHorizScrollBar)
	{
		TRect bounds(0, fBounds.GetHeight() - kScrollBarWidth, fBounds.GetWidth() - kScrollBarWidth, fBounds.GetHeight());
		fHorizScrollBar = new TScrollBar(this, bounds, TScrollBar::kHorizontal, true);
		fHorizScrollBar->Show(false);
	}
	
	if (fHasVertScrollBar)
	{
		TRect bounds(fBounds.GetWidth() - kScrollBarWidth, 0, fBounds.GetWidth(), fBounds.GetHeight() - kScrollBarWidth);
		fVertScrollBar = new TScrollBar(this, bounds, TScrollBar::kVertical, true);
		fVertScrollBar->Show(false);
	}

	if (fContainedView)
	{
		TRect bounds;
		GetChildBounds(bounds);
		fContainedView->SetBounds(bounds);

		AdjustScrollBars(fContainedView);
	}
}


void TScroller::DoMapped(bool mapped)
{
	TWindow::DoMapped(mapped);

	if (mapped && fNeedsAdjust && fContainedView)
	{
		AdjustScrollBars(fContainedView);

		const TPoint& scroll = fContainedView->GetScroll();
		ChildScrollChanged(fContainedView, scroll.h, scroll.v);

		fNeedsAdjust = false;
	}
}


void TScroller::SetContainedView(TView* view)
{
	if (view)
	{
		ASSERT(!fContainedView);
		fContainedView = view;
		view->SetWindowPositioner(SizeRelativeParent);
		AdjustScrollBars(view);

		if (IsCreated())
		{
			TRect bounds;
			GetChildBounds(bounds);
			view->SetBounds(bounds);
		}
	}
	else
		fContainedView = NULL;
}


void TScroller::RemoveChild(TWindow* child)
{
	if (child == fContainedView)
		fContainedView = NULL;
	else if (child == fHorizScrollBar)
		fHorizScrollBar = NULL;
	else if (child == fVertScrollBar)
		fVertScrollBar = NULL;
	
	TWindow::RemoveChild(child);
}


void TScroller::GetChildBounds(TRect& bounds) const
{
	ASSERT(IsCreated());
	GetLocalBounds(bounds);

	if (fHorizScrollBar && fHorizScrollBar->GetVisibility() == kWindowVisible)
		bounds.bottom -= fHorizScrollBar->GetHeight();
	if (fVertScrollBar && fVertScrollBar->GetVisibility() == kWindowVisible)
		bounds.right -= fVertScrollBar->GetWidth();
}


bool TScroller::DoCommand(TCommandHandler* sender, TCommandHandler* receiver, TCommandID command)
{
	if (fContainedView)
	{
		TScrollBar* scrollBar = NULL;
		int valueDelta = 0;
		
		switch (command)
		{
			case kScrollLeftCommandID:
				scrollBar = fHorizScrollBar;
				valueDelta = fContainedView->GetScrollIncrement(kScrollLeft);
				break;

			case kScrollRightCommandID:
				scrollBar = fHorizScrollBar;
				valueDelta = fContainedView->GetScrollIncrement(kScrollRight);
				break;
					
			case kPageLeftCommandID:
				scrollBar = fHorizScrollBar;
				valueDelta = fContainedView->GetPageIncrement(kScrollLeft);
				break;


			case kPageRightCommandID:
				scrollBar = fHorizScrollBar;
				valueDelta = fContainedView->GetPageIncrement(kScrollRight);
				break;

			case kScrollUpCommandID:
				scrollBar = fVertScrollBar;
				valueDelta = fContainedView->GetScrollIncrement(kScrollUp);
				break;


			case kScrollDownCommandID:
				scrollBar = fVertScrollBar;
				valueDelta = fContainedView->GetScrollIncrement(kScrollDown);
				break;

			case kPageUpCommandID:
				scrollBar = fVertScrollBar;
				valueDelta = fContainedView->GetPageIncrement(kScrollUp);
				break;

			case kPageDownCommandID:
				scrollBar = fVertScrollBar;
				valueDelta = fContainedView->GetPageIncrement(kScrollDown);
				break;

			case kValueChangedCommandID:
				if (sender == fHorizScrollBar)
				{
					const TPoint& scroll = fContainedView->GetScroll();
					fContainedView->ScrollBy(fHorizScrollBar->GetValue() - scroll.h, 0, false);
					return true;
				}
				else if (sender == fVertScrollBar)
				{
					const TPoint& scroll = fContainedView->GetScroll();
					fContainedView->ScrollBy(0, fVertScrollBar->GetValue() - scroll.v, false);
					return true;
				}
				break;

			default:
				break;
		}

		if (valueDelta)
		{
			if (scrollBar)
				scrollBar->SetValue(scrollBar->GetValue() + valueDelta);
			else
			{
				switch (command)
				{
					case kScrollLeftCommandID:
					case kScrollRightCommandID:
					case kPageLeftCommandID:
					case kPageRightCommandID:
						fContainedView->ScrollBy(valueDelta, 0, false);
						break;
						
					case kScrollUpCommandID:
					case kScrollDownCommandID:
					case kPageUpCommandID:
					case kPageDownCommandID:
						fContainedView->ScrollBy(0, valueDelta, false);
						break;
				}
			}
				
			return true;
		}
	}

	return TWindow::DoCommand(sender, receiver, command);
}


void TScroller::AdjustScrollBars(TView* view)
{	

	if (IsVisible() && view == fContainedView)
	{
		fContainedView = NULL;	// hack to avoid recursion here
				
		TRect	bounds;
		GetLocalBounds(bounds);
		TPoint contentSize = view->GetContentSize();
		TCoord horizScrollBarHeight = (fHorizScrollBar ? fHorizScrollBar->GetHeight() : 0);
		TCoord vertScrollBarWidth = (fVertScrollBar ? fVertScrollBar->GetWidth() : 0);
		bool showHorizScrollBar = false;
		bool showVertScrollBar = false;
		
		if (fHorizScrollBar && contentSize.h > bounds.GetWidth())
			showHorizScrollBar = true;
			
		if (fVertScrollBar && contentSize.v > bounds.GetHeight() - (showHorizScrollBar ? horizScrollBarHeight : 0))
		{
			showVertScrollBar = true;
			
			if (fHorizScrollBar && !showHorizScrollBar &&
				contentSize.h > bounds.GetWidth() - vertScrollBarWidth)
				showHorizScrollBar = true;		
		}
			
		if (fHorizScrollBar)
			fHorizScrollBar->Show(showHorizScrollBar);
			
		if (fVertScrollBar)
			fVertScrollBar->Show(showVertScrollBar);

		if (showHorizScrollBar)
		{
			fHorizScrollBar->SetBounds(0, 
									   bounds.bottom - horizScrollBarHeight,
									   showVertScrollBar ? bounds.right - vertScrollBarWidth : bounds.right,
									   bounds.bottom);
		}

		if (showVertScrollBar)
		{
			fVertScrollBar->SetBounds(bounds.right - vertScrollBarWidth, 
									   0,
									   bounds.right,
									   showHorizScrollBar ? bounds.bottom - horizScrollBarHeight : bounds.bottom);
		}
		
		GetChildBounds(bounds);
		view->SetBounds(bounds);

		TPoint maxScroll;
		view->GetMaxScroll(maxScroll);
		
		if (showHorizScrollBar)
			fHorizScrollBar->SetRange(0, maxScroll.h);

		if (showVertScrollBar)
			fVertScrollBar->SetRange(0, maxScroll.v);

		fContainedView = view;
	}
	else if (view == fContainedView)
		fNeedsAdjust = true;
}


void TScroller::ChildScrollChanged(TView* child, TCoord deltaH, TCoord deltaV)
{
	if (deltaH && fHorizScrollBar)
		fHorizScrollBar->SetValue(fHorizScrollBar->GetValue() + deltaH);
	if (deltaV && fVertScrollBar)
		fVertScrollBar->SetValue(fVertScrollBar->GetValue() + deltaV);
}


void TScroller::Draw(TRegion* clip)
{
	if (fHorizScrollBar && fHorizScrollBar->GetVisibility() == kWindowVisible && 
		fVertScrollBar && fVertScrollBar->GetVisibility() == kWindowVisible)
	{
		TDrawContext context(this, clip);

		TCoord width = GetWidth();
		TCoord height = GetHeight();

		TRect r(width - fVertScrollBar->GetWidth(), height - fHorizScrollBar->GetHeight(), width, height);
		context.SetForeColor(kMediumGrayColor);
		context.PaintRect(r);
	}
}


void TScroller::DoScrollWheel(bool down)
{
	if (fContainedView)
		HandleCommand(this, this, (down ? kScrollDownCommandID : kScrollUpCommandID));
}


