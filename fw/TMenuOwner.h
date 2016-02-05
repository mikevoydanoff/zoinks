// ========================================================================================
//	TMenuOwner.h			   Copyright (C) 2001-2002 Mike Voydanoff. All rights reserved.
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

#ifndef __TMenuOwner__
#define __TMenuOwner__

#include "TWindow.h"
#include "TMenuItem.h"
#include "TList.h"

#include <X11/X.h>	// for Time

class TFont;
class TMenuBar;


class TMenuOwner : public TWindow
{
public:
						TMenuOwner(TWindow* parent, const TRect& bounds, TWindowStyle style);
	virtual 			~TMenuOwner();

	inline TFont*		GetFont() const { return fFont; }
	void				SetFont(TFont* font);

	virtual void		MenuItemSelected(TMenu* menu, int itemIndex, Time time) = 0;
	virtual void		DrawItem(TDrawContext& context, int item, bool hilited) = 0;
	virtual TMenuBar*	GetMenuBar() = 0;
	virtual void		PositionMenu(TMenu* menu, int index, TPoint& where) = 0;

	virtual void		Show(bool doShow);

	void				DeleteAllItems();

protected:
	void				SelectMenu(TMenu* selection, int index);

protected:
	TList<TMenuItem>	fItemList;
	TFont*				fFont;
	TMenu*				fSelectedMenu;
	int					fSelectionIndex;
};

#endif // __TMenuOwner__
