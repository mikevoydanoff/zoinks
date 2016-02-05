// ========================================================================================
//	TMenu.cpp				   Copyright (C) 2001-2002 Mike Voydanoff. All rights reserved.
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

#include "TMenu.h"
#include "TSeparatorItem.h"
#include "TMenuBar.h"
#include "TApplication.h"
#include "TFont.h"
#include "TDrawContext.h"
#include "TGraphicsUtils.h"
#include "TPixmap.h"

#include "intl.h"

#include "Pixmaps/MenuArrow.xpm"
#include "Pixmaps/MenuCheck.xpm"

const TCoord kItemInset = 5;
const TCoord kSeparatorHeight = 5;
const TCoord kVertItemPadding = 5;

static TPixmap* sMenuArrowIcon = NULL;
static TPixmap* sMenuCheckIcon = NULL;


inline bool IsMenu(TMenuItem* item)
{
	return (dynamic_cast<TMenu*>(item) != NULL);
}


TMenu::TMenu(const TChar* title, const TMenuItemRec* menuItemRec)
	:	TMenuOwner(NULL, TRect(0, 0, 1, 1), kPopupWindow),
		TMenuItem(title),
		fOwner(NULL),
		fHasCheck(false)
{
	SetBackColor(kLightGrayColor);
	fEnabled = true;

	if (!sMenuArrowIcon)
		sMenuArrowIcon = new TPixmap(MenuArrow_xpm);
	if (!sMenuCheckIcon)
		sMenuCheckIcon = new TPixmap(MenuCheck_xpm);
		
	if (menuItemRec)
	{
		while (menuItemRec->fTitle[0])
		{
			if (menuItemRec->fSubMenu)
				AddMenu(gettext(menuItemRec->fTitle), menuItemRec->fSubMenu);
			else if (menuItemRec->fTitle[0] == '-' && menuItemRec->fTitle[1] == 0)
				AddSeparatorItem();
			else
				AddItem(gettext(menuItemRec->fTitle), menuItemRec->fCommandID, menuItemRec->fShortCutModifier, menuItemRec->fShortCutKey);

			menuItemRec++;
		}
	}
}


TMenu::~TMenu()
{
}


void TMenu::AddItem(TMenuItem* item)
{
	fItemList.InsertLast(item);
}


TMenuItem* TMenu::AddItem(const TChar* title, TCommandID commandID, 
					TModifierState shortCutModifier, TChar shortCutKey)
{
	TMenuItem* item = new TMenuItem(title, commandID, shortCutModifier, shortCutKey);
	AddItem(item);
	return item;
}

TMenu* TMenu::AddMenu(const TChar* title, const TMenuItemRec* menuItemRec)
{
	TMenu* menu = new TMenu(title, menuItemRec);
	AddItem(menu);
	return menu;
}



void TMenu::AddSeparatorItem()
{
	AddItem(new TSeparatorItem());
}


void TMenu::ShowMenu(const TPoint& where)
{	
	fSelectionIndex = -1;

	TRect bounds;
	CalcMenuBounds(where, bounds);

	SetBounds(bounds);

	if (!IsCreated())
		Create();
	Show(true);
	Raise();
}


void TMenu::SetOwner(TMenuOwner* owner)
{
	if (owner != fOwner)
	{
		fOwner = owner;
		if (owner)
			SetFont(owner->GetFont());
	}
}


void TMenu::DisableAll()
{
	for (int i = 0; i < fItemList.GetSize(); i++)
		fItemList[i]->DisableAll();
		
	fHasCheck = false;
}


void TMenu::EnableCommand(TCommandID command, bool check)
{
	TMenuItem::EnableCommand(command, check);

	for (int i = 0; i < fItemList.GetSize(); i++)
	{
		TMenuItem* item = fItemList[i];
		item->EnableCommand(command, check);
		if (item->IsChecked())
			fHasCheck = true;
	}
}


void TMenu::Draw(TRegion* clip)
{
	TDrawContext	context(this, clip);
	context.SetFont(fFont);

	TRect	r;
	GetLocalBounds(r);

	TGraphicsUtils::Draw3DBorderAndInset(context, r);

	for (int i = 0; i < fItemList.GetSize(); i++)
		DrawItem(context, i, (i == fSelectionIndex));
}


void TMenu::DoMouseDown(const TPoint& point, TMouseButton button, TModifierState state)
{
	if (HasPointerGrab())
		UngrabPointer(fCurrentEventTime);

	MenuItemSelected(this, fSelectionIndex, fCurrentEventTime);
}


void TMenu::DoMouseUp(const TPoint& point, TMouseButton button, TModifierState state)
{
	if (HasPointerGrab())
		UngrabPointer(fCurrentEventTime);

	MenuItemSelected(this, fSelectionIndex, fCurrentEventTime);
}


void TMenu::DoMouseMoved(const TPoint& point, TModifierState state)
{
	TRect bounds;
	GetLocalBounds(bounds);
	
	if (bounds.Contains(point))
	{
		SelectItem(FindItem(point));
	}
	else if (fSelectedMenu)
	{
		TPoint	pt(point);
		LocalToRoot(pt);

		TRect bounds;
		fSelectedMenu->GetRootBounds(bounds);
		
		if (bounds.Contains(pt))
		{
			if (HasPointerGrab())
				UngrabPointer();
			fSelectedMenu->GrabPointer();
		}
	}
	else
	{
		SelectItem(-1);
	
		if (fOwner)
		{
			TPoint	pt(point);
			LocalToRoot(pt);
	
			TRect menuBarBounds;
			fOwner->GetRootBounds(menuBarBounds);
			
			if (menuBarBounds.Contains(pt))
			{
				if (HasPointerGrab())
					UngrabPointer();
				fOwner->GrabPointer();
			}
		}
	}
}


void TMenu::DoMouseLeave(const TPoint& point, TModifierState state)
{
	if (!fSelectedMenu)
		SelectItem(-1);
}


TMenuItem* TMenu::FindShortCut(KeySym key, TModifierState state, const char* string)
{
	TListIterator<TMenuItem> iter(fItemList);
	TMenuItem* item;
	
	while ((item = iter.Next()) != NULL)
	{
		TMenuItem* found = item->FindShortCut(key, state, string);
		if (found)
			return found;
	}

	return NULL;
}


int TMenu::FindItem(const TPoint& point)
{
	TRect bounds;
	GetLocalBounds(bounds);

	if (!bounds.Contains(point))
		return -1;

	for (int i = 0; i < fItemList.GetSize(); i++)
	{
		GetItemBounds(i, bounds);
		bounds.right = GetWidth();	// expand bounds to right edge for hierarchical menus to work
		bounds.left = 0;			// tweak for submenus displayed to the left the menu

		if (bounds.Contains(point))
			return i;
	}

	return -1;
}


void TMenu::SelectItem(int selection)
{
	if (selection != fSelectionIndex)
	{
		TDrawContext	context(this);
		context.SetFont(fFont);

		if (fSelectionIndex != -1)
		{			
			DrawItem(context, fSelectionIndex, false);

			if (fSelectedMenu)
				SelectMenu(NULL, -1);
		}
		
		if (selection != -1)
		{
			TMenuItem* item = fItemList[selection];

			if (item->IsEnabled())
				DrawItem(context, selection, true);
			else
				selection = -1;

			if (IsMenu(item))
			{
				TMenu* menu = dynamic_cast<TMenu*>(item);
				ASSERT(menu);
				menu->SetOwner(this);
				SelectMenu(menu, selection);
			}
		}

		fSelectionIndex = selection;
	}
}


void TMenu::DrawItem(TDrawContext& context, int index, bool hilited)
{
	TRect bounds;
	GetItemBounds(index, bounds);
//	bounds.bottom--;
	TMenuItem* item = fItemList[index];
	bool enabled = item->IsEnabled();
	bool separator = item->IsSeparator();
	
	if (hilited && !separator)
	{
		context.SetForeColor(enabled ? kWhiteColor : kGrayColor);
		context.SetBackColor(kDarkGrayColor);
	}
	else
	{
		context.SetForeColor(enabled ? kBlackColor : kGrayColor);
		context.SetBackColor(kLightGrayColor);
	}

	TCoord	vert = 15; // fix this

	context.EraseRect(bounds);

	if (separator)
	{
		TCoord v = bounds.top + bounds.GetHeight() / 2;
		context.DrawLine(bounds.left + 1, v, bounds.right - 2, v);
	}
	else
	{
		context.MoveTo(bounds.left + kItemInset, bounds.top + vert);

		if (item->IsChecked())
			context.DrawPixmap(sMenuCheckIcon, TPoint(context.GetPen().h, context.GetPen().v- sMenuCheckIcon->GetHeight()));

		if (fHasCheck)
			context.Move(sMenuCheckIcon->GetWidth() + 1, 0);
		
		context.DrawText(item->GetTitle());

		if (IsMenu(item))
		{
			context.DrawPixmap(sMenuArrowIcon, TPoint(bounds.right - kItemInset - sMenuArrowIcon->GetWidth(), context.GetPen().v - sMenuArrowIcon->GetHeight()));
		}
		else
		{
			TString shortCutString;
			item->GetShortCutLabel(shortCutString);

			if (shortCutString.GetLength() > 0)
			{
				TCoord width = fFont->MeasureText(shortCutString);
				context.MoveTo(bounds.right - kItemInset - width, context.GetPen().v);
				context.DrawText(shortCutString);
			}
		}
	}
}


TCoord TMenu::GetItemHeight(int index)
{
	TMenuItem* item = fItemList[index];

	if (item->IsSeparator())
		return kSeparatorHeight;

	TCoord ascent, height;
	fFont->MeasureText(item->GetTitle(), item->GetTitle().GetLength(), ascent, height);
	return height + kVertItemPadding;
}


void TMenu::GetItemBounds(int index, TRect& bounds)
{
	ASSERT(index >= 0 && index < fItemList.GetSize());

	bounds.left = TGraphicsUtils::f3DBorderInset.left;
	bounds.right = fBounds.GetWidth() - TGraphicsUtils::f3DBorderInset.right;
	bounds.top = TGraphicsUtils::f3DBorderInset.top;

	for (int i = 0; i < index; i++)
		bounds.top += GetItemHeight(i);
	
	bounds.bottom = bounds.top + GetItemHeight(index);
}


void TMenu::CalcMenuBounds(const TPoint& topLeft, TRect& bounds)
{
	TCoord height;
	if (fItemList.GetSize() > 0)
	{
		TRect bounds;
		GetItemBounds(fItemList.GetSize() - 1, bounds);
		height = bounds.bottom;
	}
	else
		height = TGraphicsUtils::f3DBorderInset.top;
	
	bounds.left = topLeft.h;
	bounds.top = topLeft.v;
	bounds.bottom = bounds.top + height + TGraphicsUtils::f3DBorderInset.bottom;

	TCoord maxWidth = 0;

	for (int i = 0; i < fItemList.GetSize(); i++)
	{
		TMenuItem* item = fItemList[i];	
		TCoord width = fFont->MeasureText(item->GetTitle());

		// room for check
		width += sMenuCheckIcon->GetWidth() + 1;

		if (IsMenu(item))
		{
			width += sMenuArrowIcon->GetWidth() + 1;
		}
		else
		{
			TString	shortCutString;
			item->GetShortCutLabel(shortCutString);

			if (shortCutString.GetLength() > 0)
			{
				width += (fFont->MeasureText(shortCutString) + 2 * kItemInset);
			}
		}
		
		if (width > maxWidth)
			maxWidth = width;
	}
	
	bounds.right = bounds.left + maxWidth + 2 * kItemInset + TGraphicsUtils::f3DBorderInset.left + TGraphicsUtils::f3DBorderInset.right;
}


TMenuBar* TMenu::GetMenuBar()
{
	if (fOwner)
		return fOwner->GetMenuBar();
	else
		return NULL;
}


void TMenu::PositionMenu(TMenu* menu, int index, TPoint& where)
{
	TRect	menuBounds;
	TRect	itemBounds;

	GetItemBounds(index, itemBounds);
	menu->CalcMenuBounds(gZeroPoint, menuBounds);
	TCoord menuWidth = menuBounds.GetWidth();
	TCoord menuHeight = menuBounds.GetHeight();

	where.Set(itemBounds.right + 2, itemBounds.top);
	LocalToRoot(where);
	
	// vertical adjustment
	if (where.v < 0)
		where.v = 0;
	else if (where.v + menuHeight >= gApplication->GetScreenHeight())
		where.v -= (where.v + menuHeight - gApplication->GetScreenHeight());
		
	// horizontal adjustment
	if (where.h + menuWidth >= gApplication->GetScreenWidth())
	{
		TCoord left = where.h - GetWidth() - menuWidth - 1;
		if (left >= 0)
			where.h = left;
	}
}


void TMenu::MenuItemSelected(TMenu* menu, int itemIndex, Time time)
{
	ASSERT(fOwner);
	fOwner->MenuItemSelected(menu, itemIndex, time);
}


void TMenu::PreDisplayMenu()
{
	TMenuBar* menuBar = GetMenuBar();
	
	if (menuBar)
	{
		DisableAll();
		GetMenuBar()->GetTarget()->HandleSetupMenu(this);
	}
}



