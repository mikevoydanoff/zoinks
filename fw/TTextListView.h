// ========================================================================================
//	TTextListView.h			 	Copyright (C) 2001-2006 Mike Lockwood. All rights reserved.
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

#ifndef __TTextListView__
#define __TTextListView__

#include "TListView.h"
#include "TTypeSelectable.h"


class TTextListView : public TListView, public TTypeSelectable
{
public:
							TTextListView(TWindow* parent, const TRect& bounds);

	virtual void			Create();

	virtual TCoord			GetRowHeight(int row) const;
	virtual void			DrawCell(int row, int column, const TRect& cellBounds, TDrawContext& context);

	virtual void			GetText(int row, TString& outText) = 0;

protected:				
	virtual					~TTextListView();

	virtual void			InitDrawContext(TDrawContext& context);
	
	// TTypeSelectable interface
	virtual bool			TTypeSelectable_IsMultiSelect();
	virtual void			TTypeSelectable_SelectAll();
	virtual int				TTypeSelectable_GetCount();
	virtual bool			TTypeSelectable_AllowSelect(int i);
	virtual void			TTypeSelectable_GetText(int i, TString& text);
	virtual int				TTypeSelectable_GetSelection();
	virtual void 			TTypeSelectable_Select(int i);

protected:
	TCoord					fRowHeight;
};

#endif // __TTextListView__
