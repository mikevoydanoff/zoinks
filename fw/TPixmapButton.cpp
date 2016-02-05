// ========================================================================================
//	TPixmapButton.cpp		   Copyright (C) 2001-2003 Mike Voydanoff. All rights reserved.
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

#include "TPixmapButton.h"
#include "TDrawContext.h"
#include "TGraphicsUtils.h"


TPixmapButton::TPixmapButton(TWindow* parent, const TRect& bounds, 
							 const TPixmap* enabledPixmap, const TPixmap* disabledPixmap,
							 TCommandID commandID, bool dismissesDialog)
	:	TButton(parent, bounds, "", commandID, dismissesDialog),
		fEnabledPixmap(enabledPixmap),
		fDisabledPixmap(disabledPixmap)
{
}


TPixmapButton::~TPixmapButton()
{
}


void TPixmapButton::SetDefault()
{
	// override resizing behavior of TButton
	fDefault = true;
}


void TPixmapButton::Draw(TRegion* clip)
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
	
	const TPixmap* pixmap = (IsEnabled() ? fEnabledPixmap : fDisabledPixmap);
	
	TPoint dest(r.left + (r.GetWidth() - pixmap->GetWidth())/2, r.top + (r.GetHeight() - pixmap->GetHeight())/2);
	context.DrawPixmap(pixmap, dest);
}
