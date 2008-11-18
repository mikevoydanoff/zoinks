// ========================================================================================
//	TPopupMenu.h			 	Copyright (C) 2001-2002 Mike Lockwood. All rights reserved.
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

#ifndef __TPopupMenu__
#define __TPopupMenu__

#include "TMenu.h"


class TPopupMenu : public TMenu
{
public:
						TPopupMenu(const TMenuItemRec* menuItemRec = NULL);
	virtual 			~TPopupMenu();

	void				Display(TWindow* owningWindow, const TPoint& point, Time eventTime);

	virtual void		PreDisplayMenu();
	virtual void		MenuItemSelected(TMenu* menu, int itemIndex, Time time);
	virtual bool		DoCommand(TCommandHandler* sender, TCommandHandler* receiver, TCommandID command);

	virtual void		DoMouseDown(const TPoint& point, TMouseButton button, TModifierState state);
	virtual void		DoMouseUp(const TPoint& point, TMouseButton button, TModifierState state);
	virtual void		DoMouseMoved(const TPoint& point, TModifierState state);
	
	TMenuItem*			GetSelectedItem();

protected:
	TWindow*			fOwningWindow;
	bool				fMouseMoved;
	bool				fTrackingMouseUp;
	int					fSelectedItemIndex;
};

#endif // __TPopupMenu__
