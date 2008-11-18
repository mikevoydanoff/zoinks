// ========================================================================================
//	TMenu.h		 				Copyright (C) 2001-2002 Mike Lockwood. All rights reserved.
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

#ifndef __TMenu__
#define __TMenu__

#include "TMenuOwner.h"
#include "TMenuItem.h"

class TDrawContext;
class TMenuBar;


class TMenu : public TMenuOwner, public TMenuItem
{
public:
							TMenu(const TChar* title, const TMenuItemRec* menuItemRec = NULL);
	virtual 				~TMenu();

	void					AddItem(TMenuItem* item);
	TMenuItem*				AddItem(const TChar* title, TCommandID commandID = kNoCommandID, 
									TModifierState shortCutModifier = 0, TChar shortCutKey = 0);
	TMenu*					AddMenu(const TChar* title, const TMenuItemRec* menuItemRec = NULL);
	void					AddSeparatorItem();

	void					ShowMenu(const TPoint& where);
	void					SetOwner(TMenuOwner* owner);

	// used in menu setup
	virtual void			DisableAll();
	virtual void			EnableCommand(TCommandID command, bool check = false);

	virtual void			Draw(TRegion* clip);

	virtual void			DoMouseDown(const TPoint& point, TMouseButton button, TModifierState state);
	virtual void			DoMouseUp(const TPoint& point, TMouseButton button, TModifierState state);
	virtual void			DoMouseMoved(const TPoint& point, TModifierState state);
	virtual void			DoMouseLeave(const TPoint& point, TModifierState state);

	virtual TMenuItem*		FindShortCut(KeySym key, TModifierState state, const char* string);

	inline TMenuItem*		GetItem(int index) const { return fItemList[index]; }

	virtual TMenuBar*		GetMenuBar();
	virtual void			PositionMenu(TMenu* menu, int index, TPoint& where);

	virtual void			PreDisplayMenu();

	virtual void			MenuItemSelected(TMenu* menu, int itemIndex, Time time);

	void					CalcMenuBounds(const TPoint& topLeft, TRect& bounds);

protected:
	int						FindItem(const TPoint& point);
	void					SelectItem(int selection);
	virtual void			DrawItem(TDrawContext& context, int item, bool hilited);
	TCoord					GetItemHeight(int index);
	 void					GetItemBounds(int index, TRect& bounds);

protected:
	TMenuOwner*				fOwner;
	bool					fHasCheck;		// true if an item is currently checked
};

#endif // __TMenu__
