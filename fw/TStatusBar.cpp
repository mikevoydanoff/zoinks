// ========================================================================================
//	TStatusBar.cpp			   Copyright (C) 2001-2002 Mike Voydanoff. All rights reserved.
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

#include "TStatusBar.h"
#include "TDrawContext.h"
#include "TColor.h"
#include "TFont.h"
#include "TGraphicsUtils.h"


TStatusBar::TStatusBar(TWindow* parent, const TRect& bounds, TFont* font)
	:	TWindow(parent, bounds, kChildWindow),
		fFont(NULL)
{
	SetFont(font);
	SetBackColor(kLightGrayColor);
}


TStatusBar::~TStatusBar()
{
	fStatusItems.DeleteAll();
	
	if (fFont)
		fFont->RemoveRef();
}


void TStatusBar::Draw(TRegion* clip)
{
	TDrawContext	context(this, clip);
	context.SetFont(fFont);

	TRect bounds;
	GetLocalBounds(bounds);
	TGraphicsUtils::Draw3DBorderAndInset(context, bounds);
	context.SetForeColor(kBlackColor);
	
	for (int i = 0; i < fStatusItems.GetSize(); i++)
	{
		StatusItem* item = fStatusItems[i];
		ASSERT(item);
		
		TRect	r;
		r.top = bounds.top;
		r.bottom = bounds.bottom;
		r.left = item->offset;
		r.right = (i == fStatusItems.GetSize() - 1 ? bounds.right : fStatusItems[i + 1]->offset);
		
		context.DrawTextBox(item->text, item->text.GetLength(), r, kTextAlignLeft);
	}
}


void TStatusBar::SetFont(TFont* font)
{
	if (fFont)
		fFont->RemoveRef();

	fFont = font;
	
	if (font)
		font->AddRef();
}


void TStatusBar::InsertItem(TCoord offset)
{
	StatusItem* item = new StatusItem;
	item->offset = offset;
	fStatusItems.InsertLast(item);
}


void TStatusBar::SetStatus(const TChar* string, int index)
{
	ASSERT(index >= 0 && index < fStatusItems.GetSize());
	
	fStatusItems[index]->text = string;
	if (IsVisible())
		Redraw();
}

