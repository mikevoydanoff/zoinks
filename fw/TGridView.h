// ========================================================================================
//	TGridView.h		 		   Copyright (C) 2001-2002 Mike Voydanoff. All rights reserved.
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

#ifndef __TGridView__
#define __TGridView__

#include "TMouseTrackingIdler.h"
#include "TView.h"
#include "TRegion.h"


class TGridView : public TView
{
public:
							TGridView(TWindow* parent, const TRect& bounds);

	virtual void			Create();
	virtual void			Draw(TRegion* clip);

	virtual TCoord			GetRowHeight(int row) const = 0;
	virtual TCoord			GetColumnWidth(int column) const = 0;
	
	virtual bool			IsColumnResizable(int column) const;
	virtual void			SetColumnWidth(int column, TCoord width);
	
	virtual void			DrawCell(int row, int column, const TRect& cellBounds, TDrawContext& context) = 0;
	void					RedrawCell(int row, int column);

	virtual bool			FindCell(const TPoint& point, int& outRow, int& outColumn) const;

	virtual int				GetRowCount() const = 0;
	virtual int				GetColumnCount() const = 0;

	virtual void			GetCellBounds(int row, int column, TRect& bounds) const;

	virtual bool			AllowCellSelect(int row, int column, bool multiple);
	void					SelectCell(int row, int column);
	void					UnselectCell(int row, int column);
	void					SelectCellRange(TRect range, bool select, bool multiple);
	virtual void			SelectAll();
	void					UnselectAll();
	bool					CellSelected(int row, int column) const;
	void					SetLastClick(int row, int column);
	void					ClearLastClick();
	
	bool					GetFirstSelectedCell(int& row, int& column) const;

	virtual void			DoMouseDown(const TPoint& point, TMouseButton button, TModifierState state);
	virtual void			DoMouseUp(const TPoint& point, TMouseButton button, TModifierState state);
	virtual void			DoMouseMoved(const TPoint& point, TModifierState state);
	
	virtual bool			DoKeyDown(KeySym key, TModifierState state, const char* string);

	virtual bool			IsTargetable() const;

	virtual bool			LeftArrowKey(TModifierState state);
	virtual bool			RightArrowKey(TModifierState state);
	virtual bool			UpArrowKey(TModifierState state);
	virtual bool			DownArrowKey(TModifierState state);

	virtual void			CellDoubleClicked(int row, int column);

	virtual TCoord			GetScrollIncrement(TScrollDirection direction) const;
	virtual TCoord			GetPageIncrement(TScrollDirection direction) const;
	void					ScrollSelectionIntoView();

	void					GetVisibleCells(TRect& cells) const;

	inline bool				IsMultiSelect() const { return fMultiSelect; }
	inline void				SetMultiSelect(bool multiSelect) { fMultiSelect = multiSelect; }

protected:				
	virtual					~TGridView();

	void					ComputeContentSize();
	void					DrawCellRange(TDrawContext& context, const TRect& range, TRegion* clip = NULL);

protected:
	TRegion					fSelection;
	TPoint					fLastCellClick;
	bool					fMultiSelect;
	bool					fTrackingMouse;
	TPoint					fStartCell;			// starting cell for for mouse tracking
	TPoint					fTrackingCell;
	TMouseTrackingIdler*	fMouseTrackingIdler;
};

#endif // __TGridView__
