// ========================================================================================
//	TStatusBar.h			 	Copyright (C) 2001-2002 Mike Lockwood. All rights reserved.
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

#ifndef __TStatusBar__
#define __TStatusBar__

#include "TWindow.h"
#include "TList.h"

class TFont;

class TStatusBar : public TWindow
{
public:
						TStatusBar(TWindow* parent, const TRect& bounds, TFont* font);
	virtual 			~TStatusBar();

	virtual void		Draw(TRegion* clip);

	inline TFont*		GetFont() const { return fFont; }
	void				SetFont(TFont* font);

	void				InsertItem(TCoord offset);
	void				SetStatus(const TChar* string, int index);
	
	inline int			GetItemCount() const { return fStatusItems.GetSize(); }

protected:
	struct StatusItem
	{
		TString		text;
		TCoord		offset;
	};

	TList<StatusItem>	fStatusItems;
	TFont*				fFont;
};

#endif // __TStatusBar__
