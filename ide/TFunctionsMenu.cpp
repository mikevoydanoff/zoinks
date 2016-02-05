// ========================================================================================
//	TFunctionsMenu.cpp		   Copyright (C) 2001-2002 Mike Voydanoff. All rights reserved.
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

#include "IDECommon.h"

#include "TFunctionsMenu.h"
#include "TFunctionScanner.h"
#include "fw/TTextView.h"


TFunctionsMenu::TFunctionsMenu(TTextView* textView, const TChar* title)
	:	TMenu(title),
		fTextView(textView)
{
	ASSERT(textView);
}


TFunctionsMenu::~TFunctionsMenu()
{
}


void TFunctionsMenu::FunctionHandler(const TChar* functionName, int functionNameLength, STextOffset offset, void* userData)
{
	TString	title(functionName, functionNameLength);
	TFunctionsMenu* self = (TFunctionsMenu *)userData;
	
	TMenuItem* item = new TMenuItem(title, offset);
	item->Enable(true);
	self->AddItem(item);
}


void TFunctionsMenu::PreDisplayMenu()
{
	DeleteAllItems();

	TFunctionScanner	scanner(fTextView->GetTextLayout());
	scanner.ScanFunctions(FunctionHandler, this);
}


void TFunctionsMenu::MenuItemSelected(TMenu* menu, int itemIndex, Time time)
{
	if (itemIndex >= 0)
	{
		TMenuItem* item = menu->GetItem(itemIndex);
		ASSERT(item);
	
		uint32 line = fTextView->GetTextLayout()->OffsetToLine(item->GetCommandID());
		fTextView->ScrollToLine(line > 0 ? line - 1 : 0);
		fTextView->SetSelection(fTextView->GetTextLayout()->LineToOffset(line));
	}
	
	TMenu::MenuItemSelected(menu, -1, time);
}
