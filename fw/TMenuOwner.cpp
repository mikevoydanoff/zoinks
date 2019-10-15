// ========================================================================================
//	TMenuOwner.cpp		 	   Copyright (C) 2001-2002 Mike Voydanoff. All rights reserved.
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

#include "TMenuOwner.h"
#include "TMenu.h"
#include "TMenuBar.h"
#include "TDrawContext.h"
#include "TFont.h"


TMenuOwner::TMenuOwner(TWindow* parent, const TRect& bounds, TWindowStyle style)
	:	TWindow(parent, bounds, style),
		fFont(NULL),
		fSelectedMenu(NULL),
		fSelectionIndex(-1)
{
	fFont = TDrawContext::GetDefaultFont();
	fFont->AddRef();
}


TMenuOwner::~TMenuOwner()
{
	if (fFont)
		fFont->RemoveRef();
}

TFont* TMenuOwner::GetFont() const {
	return fFont;
}

void TMenuOwner::SetFont(TFont* font)
{
	if (fFont)
		fFont->RemoveRef();

	fFont = font;
	
	if (font)
		font->AddRef();
}


void TMenuOwner::Show(bool doShow)
{
	TWindow::Show(doShow);

	if (!doShow && fSelectedMenu)
	{
		fSelectedMenu->Show(false);
		fSelectedMenu = NULL;
		fSelectionIndex = -1;
	}
}


void TMenuOwner::SelectMenu(TMenu* selection, int index)
{
	if (selection != fSelectedMenu)
	{
		TDrawable* drawable = dynamic_cast<TDrawable*>(this);
		ASSERT(drawable);
		TDrawContext	context(this);
		context.SetFont(fFont);

		if (fSelectedMenu)
		{
			if (fSelectedMenu->HasPointerGrab())
				fSelectedMenu->UngrabPointer(fCurrentEventTime);

			fSelectedMenu->Show(false);
			
			DrawItem(context, fSelectionIndex, false);

			if (!HasPointerGrab())
				GrabPointer(fCurrentEventTime);
		}
		
		if (selection)
		{
			DrawItem(context, index, true);
			selection->PreDisplayMenu();
			
			TPoint where;
			PositionMenu(selection, index, where);
			selection->ShowMenu(where);
		}

		fSelectedMenu = selection;
		fSelectionIndex = index;
	}
}


void TMenuOwner::DeleteAllItems()
{
	TListIterator<TMenuItem> iter(fItemList);
	TMenuItem* item;
	
	while ((item = iter.Next()) != NULL)
		delete item;

	fItemList.RemoveAll();		
}


