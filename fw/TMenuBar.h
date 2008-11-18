// ========================================================================================
//	TMenuBar.h		 			Copyright (C) 2001-2002 Mike Lockwood. All rights reserved.
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

#ifndef __TMenuBar__
#define __TMenuBar__

#include "TMenuOwner.h"
#include "TList.h"

class TMenu;
class TDrawContext;


class TMenuBar : public TMenuOwner
{
public:
						TMenuBar(TWindow* parent, const TRect& bounds, TFont* font = NULL);
	virtual 			~TMenuBar();

	void				AddMenu(TMenu* menu, TMenu* beforeMenu = NULL);
	TMenu*				AddMenu(const TChar* title, const TMenuItemRec* menuItemRec = NULL, TMenu* beforeMenu = NULL);
	void				RemoveMenu(TMenu* menu);

	void				DisableAll();

	virtual void		MenuItemSelected(TMenu* menu, int itemIndex, Time time);

	virtual void		Draw(TRegion* clip);

	virtual void		DoMouseDown(const TPoint& point, TMouseButton button, TModifierState state);
	virtual void		DoMouseUp(const TPoint& point, TMouseButton button, TModifierState state);
	virtual void		DoMouseMoved(const TPoint& point, TModifierState state);

	bool				DoShortCut(KeySym key, TModifierState state, const char* string);

	TMenu*				GetMenu(int index) const;
	virtual TMenuBar*	GetMenuBar();
	virtual void		PositionMenu(TMenu* menu, int index, TPoint& where);

	TWindow*			GetTarget();
	
	static inline TFont*	GetDefaultFont() { return sDefaultFont; }

protected:
	void				StopTracking(Time time);
	int					FindMenu(const TPoint& point);
	virtual void		DrawItem(TDrawContext& context, int item, bool hilited);
	void				GetItemBounds(int index, TRect& bounds);

protected:
	// for mouse tracking
	bool				fTracking;
	bool				fTrackingMouseUp;
	bool				fMouseMoved;
	
	static TFont* 		sDefaultFont;

};

#endif // __TMenuBar__
