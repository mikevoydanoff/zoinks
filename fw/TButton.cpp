// ========================================================================================
//	TButton.cpp				 	Copyright (C) 2001-2002 Mike Lockwood. All rights reserved.
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

#include "TButton.h"
#include "TDrawContext.h"
#include "TGraphicsUtils.h"


TButton::TButton(TWindow* parent, const TRect& bounds, const TChar* title, 
				 TCommandID commandID, bool dismissesDialog)
	:	TWindow(parent, bounds, kChildWindow),
		fTitle(title),
		fCommandID(commandID),
		fDismissesDialog(dismissesDialog),
		fPressed(false),
		fEnabled(true),
		fDefault(false),
		fTrackingMouse(false)
{
	SetBackColor(kLightGrayColor);
}


TButton::~TButton()
{
}


void TButton::SetDefault()
{
	if (!fDefault)
	{
		TRect bounds(fBounds);
		bounds.right++;
		bounds.bottom++;
		SetBounds(bounds);
		fDefault = true;
	}
}


bool TButton::DismissesDialog(TCommandID command) const 
{
	return fDismissesDialog;
}


void TButton::Draw(TRegion* clip)
{
	TDrawContext	context(this, clip);
	
	TRect	r;
	GetLocalBounds(r);

	if (fPressed)
	{
		TGraphicsUtils::DrawPressed3DBorderAndInset(context, r, fDefault);
		context.SetBackColor(kMediumGrayColor);
	}
	else
		TGraphicsUtils::Draw3DBorderAndInset(context, r, fDefault && fEnabled);

	context.SetForeColor(fEnabled ? fForeColor : kDarkGrayColor);
	context.DrawTextBox(fTitle, fTitle.GetLength(), r, kTextAlignCenter);
}


void TButton::DoMouseDown(const TPoint& point, TMouseButton button, TModifierState state)
{
	if (button == kLeftButton && fEnabled)
	{
		fPressed = true;
		fTrackingMouse = true;
		Redraw();
	}
}


void TButton::DoMouseUp(const TPoint& point, TMouseButton button, TModifierState state)
{
	if (button == kLeftButton && fTrackingMouse)
	{
		if (fPressed)
		{
			fPressed = false;
			Redraw();
			HandleCommand(this, this, fCommandID);
		}

		fTrackingMouse = false;
	}
}


void TButton::DoMouseMoved(const TPoint& point, TModifierState state)
{
	if (fTrackingMouse)
	{
		TRect bounds;
		GetLocalBounds(bounds);

		if (bounds.Contains(point) != fPressed)
		{
			fPressed = !fPressed;
			Redraw();
		}
	}
}


void TButton::SetEnabled(bool enabled)
{
	if (fEnabled != enabled)
	{
		fEnabled = enabled;
		if (IsCreated())
			Redraw();
	}
}	
