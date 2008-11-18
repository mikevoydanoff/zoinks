// ========================================================================================
//	TDiffListView.h					 Copyright (C) 2003 Mike Lockwood. All rights reserved.
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

#ifndef __TDiffListView__
#define __TDiffListView__

#include "fw/TDynamicArray.h"
#include "fw/TTextListView.h"

struct TDiffRec;


class TDiffListView : public TTextListView
{	
public:
								TDiffListView(TWindow* parent, const TRect& bounds, TDynamicArray<TDiffRec>& diffList);

	virtual int					GetRowCount() const;

	virtual void				GetText(int row, TString& outText);
	virtual void				DrawCell(int row, int column, const TRect& cellBounds, TDrawContext& context);

	void						DiffListChanged();

protected:				
	virtual						~TDiffListView();

	virtual bool				AllowCellSelect(int row, int column, bool multiple);

protected:
	TCoord						fRowHeight;
	
	TDynamicArray<TDiffRec>& 	fDiffList;
};

#endif // __TDiffListView__
