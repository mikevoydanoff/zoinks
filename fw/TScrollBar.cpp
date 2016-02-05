// ========================================================================================
//	TScrollBar.cpp			   Copyright (C) 2001-2008 Mike Voydanoff. All rights reserved.
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

#include "TScrollBar.h"
#include "TDrawContext.h"
#include "TPixmap.h"
#include "TGraphicsUtils.h"

#include "Pixmaps/LeftArrow.xpm"
#include "Pixmaps/RightArrow.xpm"
#include "Pixmaps/UpArrow.xpm"
#include "Pixmaps/DownArrow.xpm"

const TCoord kArrowSize = 15;
const TCoord kMinThumbSize = 15;

const TTime kScrollRepeat = 200;

TPixmap* TScrollBar::fArrowPixmaps[4] = {NULL, NULL, NULL, NULL};

const char** TScrollBar::fArrowPixmapData[4] =
{
	LeftArrow_xpm,
	RightArrow_xpm,
	UpArrow_xpm,
	DownArrow_xpm
};

TScrollBar::TScrollBar(TWindow* parent, const TRect& bounds, 
					   Orientation orientation, bool proportionalThumbs)
	:	TWindow(parent, bounds, kChildWindow),
		fMinimum(0),
		fMaximum(0),
		fValue(0),
		fOrientation(orientation),
		fTrackingPart(kNone),
		fProportionalThumbs(proportionalThumbs)
{
	ASSERT(orientation == kHorizontal || orientation == kVertical);
	SetIdleFrequency(kScrollRepeat);
}


TScrollBar::~TScrollBar()
{
}


void TScrollBar::SetRange(int32 minimum, int32 maximum)
{
	fMinimum = minimum;
	fMaximum = maximum;

	if (fValue < minimum)
		fValue = minimum;
	else if (fValue > maximum)
		fValue = maximum;

	TDrawContext context(this);
	DrawThumb(context);
}


void TScrollBar::SetValue(int32 value)
{
	if (value < fMinimum)
		value = fMinimum;
	else if (value > fMaximum)
		value = fMaximum;

	if (value != fValue)
	{
		TRect	oldThumb;
		GetThumb(oldThumb);
		
		fValue = value;

		HandleCommand(this, this, kValueChangedCommandID);

		TRect	newThumb;
		GetThumb(newThumb);

		if (newThumb != oldThumb)
		{
			TDrawContext	context(this);
			TRect			rect;

			if (newThumb.left < oldThumb.left)
				rect.Set(newThumb.right, newThumb.top, oldThumb.right, newThumb.bottom);
			else if (newThumb.left > oldThumb.left)
				rect.Set(oldThumb.left, newThumb.top, newThumb.left, newThumb.bottom);

			if (newThumb.top < oldThumb.top)
				rect.Set(newThumb.left, newThumb.bottom, newThumb.right, oldThumb.bottom);
			else if (newThumb.top > oldThumb.top)
				rect.Set(newThumb.left, oldThumb.top, newThumb.right, newThumb.top);


			if (context.GetDepth() < 8)
			{
				context.SetStipple(TGraphicsUtils::GetGrayStipple());
				context.SetForeColor(kBlackColor);
				context.SetBackColor(kWhiteColor);
			}
			else
				context.SetForeColor(kMediumGrayColor);
			
			context.PaintRect(rect);
			DrawThumb(context);
		}
	}
}


void TScrollBar::Draw(TRegion* clip)
{
	TDrawContext	context(this, clip);

	DrawArrow1(context, fInTrackingRect && fTrackingPart == kArrow1);
	DrawArrow2(context, fInTrackingRect && fTrackingPart == kArrow2);
	DrawThumb(context);
}


void TScrollBar::DoMouseDown(const TPoint& point, TMouseButton button, TModifierState state)
{
	if (button == kLeftButton)
	{
//	ASSERT(fTrackingPart == kNone);

		fTrackingPart = IdentifyPoint(point, fTrackingRect);
		fInTrackingRect = true;

		switch (fTrackingPart)
		{
			case kArrow1:
			{
				TDrawContext	context(this);
				DrawArrow1(context, true);
				HandleCommand(this, this, (IsVertical() ? kScrollUpCommandID : kScrollLeftCommandID));
				EnableIdling(true);
				break;
			}
			
			case kArrow2:
			{
				TDrawContext	context(this);
				DrawArrow2(context, true);
				HandleCommand(this, this, (IsVertical() ? kScrollDownCommandID : kScrollRightCommandID));
				EnableIdling(true);
				break;
			}

			case kPageUp:
				HandleCommand(this, this, (IsVertical() ? kPageUpCommandID : kPageLeftCommandID));
				EnableIdling(true);
				break;

			case kPageDown:
				HandleCommand(this, this, (IsVertical() ? kPageDownCommandID : kPageRightCommandID));
				EnableIdling(true);
				break;

			default:
				break;
		}

		fLastPoint = point;
	}
}


void TScrollBar::DoMouseUp(const TPoint& point, TMouseButton button, TModifierState state)
{
	if (button == kLeftButton)
	{
		if (fInTrackingRect)
		{
			ScrollBarPart part = fTrackingPart;
			fTrackingPart = kNone;
			
			switch (part)
			{
				case kArrow1:
				{
					TDrawContext	context(this);
					DrawArrow1(context, false);
					break;
				}
				
				case kArrow2:
				{
					TDrawContext	context(this);
					DrawArrow2(context, false);
					break;
				}

				default:
					break;
			}

			fInTrackingRect = false;
		}

		EnableIdling(false);
	}
}


void TScrollBar::DoMouseMoved(const TPoint& point, TModifierState state)
{
	bool inTrackingRect = fTrackingRect.Contains(point);

	if (fTrackingPart == kArrow1 || fTrackingPart == kArrow2 || fTrackingPart == kPageUp || fTrackingPart == kPageDown)
	{
		if (inTrackingRect != fInTrackingRect)
		{
			if (fTrackingPart == kArrow1 || fTrackingPart == kArrow2)
			{
				TDrawContext	context(this);

				if (fTrackingPart == kArrow1)
					DrawArrow1(context, inTrackingRect);
				else
					DrawArrow2(context, inTrackingRect);
			}
			
			fInTrackingRect = inTrackingRect;
			EnableIdling(inTrackingRect);
		}
	}
	else if (fTrackingPart == kThumb && fMinimum < fMaximum)
	{
		TCoord delta = (IsVertical() ? point.v - fLastPoint.v : point.h - fLastPoint.h);

		if (delta != 0)
		{
			TRect	thumb, thumbArea;
			GetThumb(thumb);
			GetThumbArea(thumbArea);

			if ((IsVertical() && thumb.GetHeight() < thumbArea.GetHeight()) ||
				(!IsVertical() && thumb.GetWidth() < thumbArea.GetWidth()))
			{
				int32 newValue;
				
				if (IsVertical())
					newValue = (int32)(((float)(thumb.top - thumbArea.top + delta + 1) * 2.0 * (fMaximum - fMinimum)) / ((thumbArea.GetHeight() - thumb.GetHeight()) * 2));
				else
					newValue = (int32)(((float)(thumb.left - thumbArea.left + delta + 1) * 2.0 * (fMaximum - fMinimum)) / ((thumbArea.GetWidth() - thumb.GetWidth()) * 2));

				SetValue(newValue);
			}
		}
	}

	fLastPoint = point;
}


void TScrollBar::DoIdle()
{
	switch (fTrackingPart)
	{
		case kArrow1:
			HandleCommand(this, this, (IsVertical() ? kScrollUpCommandID : kScrollLeftCommandID));
			break;
		
		case kArrow2:
			HandleCommand(this, this, (IsVertical() ? kScrollDownCommandID : kScrollRightCommandID));
			break;

		case kPageUp:
			HandleCommand(this, this, (IsVertical() ? kPageUpCommandID : kPageLeftCommandID));
			break;

		case kPageDown:
			HandleCommand(this, this, (IsVertical() ? kPageDownCommandID : kPageRightCommandID));
			break;

		default:
			break;
	}
}


void TScrollBar::GetArrow1(TRect& r) const
{
	GetLocalBounds(r);

	if (IsVertical())
		r.bottom = kArrowSize;
	else
		r.right = kArrowSize;
}


void TScrollBar::GetArrow2(TRect& r) const
{
	GetLocalBounds(r);

	if (IsVertical())
		r.top = r.bottom - kArrowSize;
	else
		r.left = r.right - kArrowSize;
}


void TScrollBar::GetThumbArea(TRect& r) const
{
	GetLocalBounds(r);

	if (IsVertical())
	{
		r.top = kArrowSize;
		r.bottom -= kArrowSize;
	}
	else
	{
		r.left = kArrowSize;
		r.right -= kArrowSize;
	}
}


void TScrollBar::GetThumb(TRect& r) const
{
	if (fMinimum < fMaximum)
	{
		GetThumbArea(r);

		if (IsVertical())
		{
			TCoord	available = r.GetHeight();

			if (available > kMinThumbSize)
			{
				TCoord thumbSize;
				if (fProportionalThumbs)
					thumbSize = ((long long)fBounds.GetHeight() * (long long)available) / (long long)(fMaximum - fMinimum + fBounds.GetHeight());
				else
					thumbSize = fBounds.GetWidth();
					
				if (thumbSize < kMinThumbSize)
					thumbSize = kMinThumbSize;
				else if (thumbSize > available)
					thumbSize = available;

				available -= thumbSize;				
				r.top += (int)(((long long)(fValue - fMinimum) * (long long)available) / (long long)(fMaximum - fMinimum));
				r.bottom = r.top + thumbSize;
			}
		}
		else
		{
			TCoord	available = r.GetWidth();

			if (available > kMinThumbSize)
			{
				TCoord thumbSize;
				if (fProportionalThumbs)
					thumbSize = ((long long)fBounds.GetWidth() * (long long)available) / (long long)(fMaximum - fMinimum + fBounds.GetWidth());
				else
					thumbSize = fBounds.GetHeight();

				if (thumbSize < kMinThumbSize)
					thumbSize = kMinThumbSize;
				else if (thumbSize > available)
					thumbSize = available;

				available -= thumbSize;			
				r.left += (int)(((long long)(fValue - fMinimum) * (long long)available) / (long long)(fMaximum - fMinimum));
				r.right = r.left + thumbSize;
			}
		}
	}
	else
		r.SetEmpty();
}


ScrollBarPart TScrollBar::IdentifyPoint(const TPoint& point, TRect& outTrackingRect) const
{
	GetArrow1(outTrackingRect);
	if (outTrackingRect.Contains(point))
		return kArrow1;
	
	GetArrow2(outTrackingRect);
	if (outTrackingRect.Contains(point))
		return kArrow2;

	TRect thumb;
	GetThumb(thumb);
	if (thumb.Contains(point))
	{
		outTrackingRect = thumb;
		return kThumb;
	}

	TRect thumbArea;
	GetThumbArea(thumbArea);
	if (! thumbArea.Contains(point))
		return kNone;

	if (IsVertical())
	{
		outTrackingRect.Set(thumbArea.left, thumbArea.top, thumbArea.right, thumb.top);
		if (outTrackingRect.Contains(point))
			return kPageUp;
		else
		{
			outTrackingRect.top = thumb.bottom;
			outTrackingRect.bottom = thumbArea.bottom;
			return kPageDown;
		}
	}
	else
	{
		outTrackingRect.Set(thumbArea.left, thumbArea.top, thumb.left, thumbArea.bottom);
		if (outTrackingRect.Contains(point))
			return kPageUp;
		else
		{
			outTrackingRect.left = thumb.right;
			outTrackingRect.right = thumbArea.right;
			return kPageDown;
		}
	}
}


void TScrollBar::DrawArrow1(TDrawContext& context, bool pressed)
{
	TRect	r;
	GetArrow1(r);

	if (pressed)
		TGraphicsUtils::DrawPressed3DBorderAndInset(context, r);
	else
		TGraphicsUtils::Draw3DBorderAndInset(context, r);
	
	context.SetForeColor(kLightGrayColor);
	context.PaintRect(r);

	TPixmap* pixmap = GetArrowPixmap(IsVertical() ? kUpArrow : kLeftArrow);
	ASSERT(pixmap);

	TRect 	arrowBounds(pixmap->GetBounds());
	arrowBounds.CenterIn(r);
	context.DrawPixmap(pixmap, TPoint(arrowBounds.left, arrowBounds.top));
}


void TScrollBar::DrawArrow2(TDrawContext& context, bool pressed)
{
	TRect	r;
	GetArrow2(r);

	if (pressed)
		TGraphicsUtils::DrawPressed3DBorderAndInset(context, r);
	else
		TGraphicsUtils::Draw3DBorderAndInset(context, r);

	context.SetForeColor(kLightGrayColor);
	context.PaintRect(r);

	TPixmap* pixmap = GetArrowPixmap(IsVertical() ? kDownArrow : kRightArrow);
	ASSERT(pixmap);

	TRect 	arrowBounds(pixmap->GetBounds());
	arrowBounds.CenterIn(r);
	context.DrawPixmap(pixmap, TPoint(arrowBounds.left, arrowBounds.top));
}


void TScrollBar::DrawThumb(TDrawContext& context)
{
	TRect	thumbArea;
	GetThumbArea(thumbArea);

	TRect thumb;
	GetThumb(thumb);

	if (context.GetDepth() < 8)
	{
		context.SetStipple(TGraphicsUtils::GetGrayStipple());
		context.SetForeColor(kBlackColor);
		context.SetBackColor(kWhiteColor);
	}
	else
		context.SetForeColor(kMediumGrayColor);

	if (thumb.IsEmpty())
	{
		context.PaintRect(thumbArea);
	}
	else
	{
		if (IsVertical())
		{
			TRect r(thumbArea.left, thumbArea.top, thumbArea.right, thumb.top);
			context.PaintRect(r);
			r.Set(thumb.left, thumb.bottom, thumbArea.right, thumbArea.bottom);
			context.PaintRect(r);
		}
		else
		{
			TRect r(thumbArea.left, thumbArea.top, thumb.left, thumbArea.bottom);
			context.PaintRect(r);
			r.Set(thumb.right, thumbArea.top, thumbArea.right, thumbArea.bottom);
			context.PaintRect(r);
		}	

		TGraphicsUtils::Draw3DBorderAndInset(context, thumb);
		context.SetForeColor(kLightGrayColor);
		context.PaintRect(thumb);
	}
}


TPixmap* TScrollBar::GetArrowPixmap(ArrowPixmap pixmap)
{
	if (! fArrowPixmaps[pixmap])
		fArrowPixmaps[pixmap] = new TPixmap(fArrowPixmapData[pixmap]);

	return fArrowPixmaps[pixmap];
}

