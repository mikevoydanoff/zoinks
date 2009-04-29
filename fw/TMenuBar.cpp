// ========================================================================================
//	TMenuBar.cpp			 	Copyright (C) 2001-2002 Mike Lockwood. All rights reserved.
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

#include "TMenuBar.h"
#include "TDrawContext.h"
#include "TMenu.h"
#include "TApplication.h"
#include "TColor.h"
#include "TFont.h"
#include "TTopLevelWindow.h"
#include "TGraphicsUtils.h"

#include "intl.h"

#include <limits.h>
#include <stdio.h>

const TCoord kMenuInset = 5;

TFont* TMenuBar::sDefaultFont = NULL;


TMenuBar::TMenuBar(TWindow* parent, const TRect& bounds, TFont* font)
	:	TMenuOwner(parent, bounds, kChildWindow),
		fTracking(false),
		fTrackingMouseUp(false)
{
	if (!font)
	{
		if (!sDefaultFont)
			sDefaultFont = new TFont(_("-adobe-helvetica-medium-r-normal-*-12-*-*-*-*-*-iso8859-1"));
		
		font = sDefaultFont;
	}
	
	SetFont(font);

	SetBackColor(kLightGrayColor);

	TTopLevelWindow* window = (parent ? parent->GetTopLevelWindow() : NULL);
	if (window)
		window->SetMenuBar(this);
}


TMenuBar::~TMenuBar()
{
	for (int i = fItemList.GetSize() - 1; i >= 0; i--)
	{
		TMenu* menu = dynamic_cast<TMenu*>(fItemList[i]);
		ASSERT(menu);
		menu->Destroy();
	}
}


void TMenuBar::AddMenu(TMenu* menu, TMenu* beforeMenu)
{
	if (beforeMenu)
	{
		int32 index = fItemList.FindIndex(dynamic_cast<TMenuItem*>(beforeMenu));

		if (index > 0)
			 fItemList.InsertAt(menu, index);
		else
			fItemList.InsertLast(menu);
	}
	else
		fItemList.InsertLast(menu);

	menu->SetOwner(this);
	Redraw();
}


TMenu* TMenuBar::AddMenu(const TChar* title, const TMenuItemRec* menuItemRec, TMenu* beforeMenu)
{
	TMenu* menu = new TMenu(title, menuItemRec);
	AddMenu(menu, beforeMenu);
	return menu;
}


void TMenuBar::RemoveMenu(TMenu* menu)
{
	fItemList.Remove(menu);
	
	// need to erase since drawable area of menu bar will shrink
	TDrawContext	context(this);
	TRect			r;
	GetLocalBounds(r);
	context.EraseRect(r);
	
	Redraw();
}


void TMenuBar::DisableAll()
{
	for (int i = 0; i < fItemList.GetSize(); i++)
		fItemList[i]->DisableAll();
}


void TMenuBar::MenuItemSelected(TMenu* menu, int itemIndex, Time time)
{
	StopTracking(time);

	if (itemIndex >= 0)
	{
		TMenuItem* item = menu->GetItem(itemIndex);
		ASSERT(item);

		if (item->GetCommandID() != kNoCommandID)
			GetTarget()->HandleCommand(this, this, item->GetCommandID());
	}
}


void TMenuBar::Draw(TRegion* clip)
{
	TDrawContext	context(this, clip);
	context.SetFont(fFont);

	TRect bounds;
	GetLocalBounds(bounds);
	TGraphicsUtils::Draw3DBorderAndInset(context, bounds);

	if (fItemList.GetSize() > 0)
	{
		TRect menuBounds;
		GetItemBounds(fItemList.GetSize() - 1, menuBounds);
		bounds.left = menuBounds.right;
	}

	for (int i = 0; i < fItemList.GetSize(); i++)
		DrawItem(context, i, (i == fSelectionIndex));
}


void TMenuBar::DoMouseDown(const TPoint& point, TMouseButton button, TModifierState states)
{
	if (button == kLeftButton)
	{
		if (fTracking)
		{
			StopTracking(fCurrentEventTime);
		}
		else
		{
			int index = FindMenu(point);
			TMenu* menu = (index >= 0 ? GetMenu(index) : NULL);
			SelectMenu(menu, index);

			fTracking = true;
			fTrackingMouseUp = false;
			fMouseMoved = false;
			GrabPointer(fCurrentEventTime);
		}
	}
}


void TMenuBar::DoMouseUp(const TPoint& point, TMouseButton button, TModifierState state)
{
	if (fTracking)
	{
		if (fMouseMoved && button == kLeftButton)
		{
			StopTracking(fCurrentEventTime);
		}
		else
			fTrackingMouseUp = true;
	}
}


void TMenuBar::DoMouseMoved(const TPoint& point, TModifierState state)
{
	if (fTracking)
	{
		if (fBounds.Contains(point))
		{
			int index = FindMenu(point);
			TMenu* menu = (index >= 0 ? GetMenu(index) : NULL);
			SelectMenu(menu, index);

			fMouseMoved = true;
		}
		else if (fSelectedMenu && HasPointerGrab())
		{
			TPoint	pt(point);
			LocalToRoot(pt);

			TRect bounds;
			fSelectedMenu->GetRootBounds(bounds);
			
			if (bounds.Contains(pt))
			{
				UngrabPointer();
				fSelectedMenu->GrabPointer();
			}
		}
		else if (fTrackingMouseUp)
		{
			fTracking = false;
			fTrackingMouseUp = false;
			UngrabPointer();
		}
	}
}


void TMenuBar::StopTracking(Time time)
{
	SelectMenu(NULL, -1);
	fTracking = false;
	fTrackingMouseUp = false;

	if (HasPointerGrab())
		UngrabPointer(fCurrentEventTime);
}


int TMenuBar::FindMenu(const TPoint& point)
{
	int result = -1;
	
	for (int i = 0; i < fItemList.GetSize(); i++)
	{
		TRect	bounds;
		GetItemBounds(i, bounds);
		// only compare against horizontal boundaries
		bounds.top = INT_MIN;
		bounds.bottom = INT_MAX;

		if (bounds.Contains(point))
		{
			result = i;
			break;
		}
	}

	return result;
}


TWindow* TMenuBar::GetTarget()
{
	TWindow* result = NULL;
	TTopLevelWindow* topLevel = GetTopLevelWindow();
	
	if (topLevel)
		result = topLevel->GetTarget();

	if (!result)
		result = this;
		
	return result;
}


bool TMenuBar::DoShortCut(KeySym key, TModifierState state, const char* string)
{
	TListIterator<TMenuItem> iter(fItemList);
	TMenuItem* item;
	TMenu* menu = NULL;
	TMenuItem* found = NULL;
	
	while ((item = iter.Next()) != NULL)
	{
		menu = dynamic_cast<TMenu*>(item);
		
		found = menu->FindShortCut(key, state, string);
		if (found)
			break;
	}

	if (found && found->GetCommandID() != kNoCommandID)
	{
		if (fTracking)
			StopTracking(fCurrentEventTime);
	
		menu->PreDisplayMenu();

		if (found->IsEnabled())
			GetTarget()->HandleCommand(this, this, found->GetCommandID());

		return true;
	}
	else
		return false;
}




void TMenuBar::DrawItem(TDrawContext& context, int item, bool hilited)
{
	TRect bounds;
	GetItemBounds(item, bounds);
	bounds.bottom--;
	
	if (hilited)
	{
		context.SetForeColor(kWhiteColor);
		context.SetBackColor(kDarkGrayColor);
	}
	else
	{
		context.SetForeColor(kBlackColor);
		context.SetBackColor(kLightGrayColor);
	}

	TCoord	vert = 15; // fix this

	context.EraseRect(bounds);
	context.MoveTo(bounds.left + kMenuInset, vert);
	TMenu* menu =  dynamic_cast<TMenu*>(fItemList[item]);
	context.DrawText(menu->GetTitle());
}


void TMenuBar::GetItemBounds(int index, TRect& bounds)
{
	ASSERT(index >= 0 && index < fItemList.GetSize());

	TDrawContext	context(this);

	TCoord left = TGraphicsUtils::f3DBorderInset.left;
	TCoord right = left;
	
	for (int i = 0; i <= index; i++)
	{
		TMenu* menu =  dynamic_cast<TMenu*>(fItemList[i]);

		left = right;
		right += fFont->MeasureText(menu->GetTitle()) + 2 * kMenuInset;
	}

	bounds.Set(left, TGraphicsUtils::f3DBorderInset.top, right, fBounds.GetHeight() - TGraphicsUtils::f3DBorderInset.bottom + 1);
}


TMenu* TMenuBar::GetMenu(int index) const
{
	return dynamic_cast<TMenu*>(fItemList[index]);
}

TMenuBar* TMenuBar::GetMenuBar()
{
	return this;
}


void TMenuBar::PositionMenu(TMenu* menu, int index, TPoint& where)
{
	TRect	menuBounds;
	TRect	itemBounds;

	GetItemBounds(index, itemBounds);
	menu->CalcMenuBounds(gZeroPoint, menuBounds);
	TCoord menuWidth = menuBounds.GetWidth();
	TCoord menuHeight = menuBounds.GetHeight();

	where.Set(itemBounds.left, GetHeight() - 1); // subtract 1 so borders will overlap
	LocalToRoot(where);
	
	// horizontal adjustment
	if (where.h < 0)
		where.h = 0;
	else if (where.h + menuWidth >= gApplication->GetScreenWidth())
		where.h -= (where.h + menuWidth - gApplication->GetScreenWidth());
		
	// vertical adjustment
	if (where.v + menuHeight >= gApplication->GetScreenHeight())
	{
		TCoord top = where.v - GetHeight() - menuHeight - 1;
		if (top >= 0)
			where.v = top;
	}
}

