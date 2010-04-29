// ========================================================================================
//	TGridView.cpp			 	Copyright (C) 2001-2008 Mike Lockwood. All rights reserved.
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

#include "TGridView.h"
#include "TDrawContext.h"
#include "TRegion.h"

#include <X11/keysym.h>


TGridView::TGridView(TWindow* parent, const TRect& bounds)
	:	TView(parent, bounds),
		fLastCellClick(-1, -1),
		fMultiSelect(false),
		fTrackingMouse(false),
		fMouseTrackingIdler(NULL)
{
}


TGridView::~TGridView()
{
	delete fMouseTrackingIdler;
}


void TGridView::Create()
{
	TView::Create();

	ComputeContentSize();
}


void TGridView::Draw(TRegion* clip)
{
	// draw outside our content area
	TView::Draw(clip);

	TDrawContext context(this, clip);

	TRect	r(0, 0, GetColumnCount(), GetRowCount());
	DrawCellRange(context, r, clip);
}


bool TGridView::IsColumnResizable(int column) const
{
	return false;
}


void TGridView::SetColumnWidth(int column, TCoord width)
{
	ASSERT(0);	// need to override in your subclass!
}


void TGridView::RedrawCell(int row, int column)
{
	TRect bounds;
	
	GetCellBounds(row, column, bounds);
	RedrawRect(bounds);
}


void TGridView::DrawCellRange(TDrawContext& context, const TRect& range, TRegion* clip)
{
	if (range.top < range.bottom && range.left < range.right)
	{
		TRect	bounds;
		GetCellBounds(range.top, range.left, bounds);

		TCoord v = bounds.top;
		
		for (int row = range.top; row < range.bottom; row++)
		{
			TCoord h = bounds.left;
			TCoord height = GetRowHeight(row);

			for (int column = range.left; column < range.right; column++)
			{
				TCoord width = GetColumnWidth(column);
				TRect	bounds(h, v, h + width, v + height);
				bounds.Offset(-fScroll.h, -fScroll.v);
				
				if (!clip || clip->Intersects(bounds))
				{
					bounds.Offset(fScroll.h, fScroll.v);
					DrawCell(row, column, bounds, context);
				}

				h += width;
			}

			v += height;
		}
	}
}


void TGridView::GetCellBounds(int row, int column, TRect& bounds) const
{
	// inefficient - but you can override if you want

	ASSERT(row >= 0 && row < GetRowCount());
	ASSERT(column >= 0 && column < GetColumnCount());	

	TCoord h = 0;
	TCoord v = 0;

	int i;
	for (i = 0; i < row; i++)
		v += GetRowHeight(i);
		
	for (i = 0; i < column; i++)
		h += GetColumnWidth(i);

	bounds.Set(h, v, h + GetColumnWidth(column), v + GetRowHeight(row));
}


void TGridView::ComputeContentSize()
{
	int rows = GetRowCount();
	int columns = GetColumnCount();

	if (rows > 0 && columns > 0)
	{
		TRect bounds;
		GetCellBounds(rows - 1, columns - 1, bounds);
		SetContentSize(TPoint(bounds.right, bounds.bottom));
	}
	else
		SetContentSize(gZeroPoint);
}


bool TGridView::AllowCellSelect(int row, int column, bool multiple)
{
	return true;
}


void TGridView::SelectCell(int row, int column)
{
	TRect	r(column, row, column + 1, row + 1);

	SelectCellRange(r, true, false);
}


void TGridView::UnselectCell(int row, int column)
{
	TRect	r(column, row, column + 1, row + 1);

	SelectCellRange(r, false, false);
}


void TGridView::SetLastClick(int row, int column)
{
	fLastCellClick.Set(column, row);
}

void TGridView::ClearLastClick()
{
	fLastCellClick.Set(-1, -1);
}

void TGridView::SelectCellRange(TRect range, bool select, bool multiple)
{
	int row, column;
	for (row = range.top; row < range.bottom; row++)
	{
		for (column = range.left; column < range.right; column++)
		{
			TRect	r(column, row, column + 1, row + 1);
			
			if (select)
			{
				if (AllowCellSelect(row, column, multiple))
					fSelection.Union(r);
			}
			else
			{
				TRegion	temp(r);
				fSelection.Difference(&temp);
			}
		}
	}

	if (IsCreated())
	{
		TDrawContext	context(this);
		DrawCellRange(context, range);
	}
}


void TGridView::SelectAll()
{
	for (int row = 0; row < GetRowCount(); row++)
	{
		for (int column = 0; column < GetColumnCount(); column++)
		{
			if (!CellSelected(row, column))
				SelectCell(row, column);
		}
	}
	
	DoCommand(this, this, kSelectionChangedCommandID);
}


void TGridView::UnselectAll()
{
	for (int row = 0; row < GetRowCount(); row++)
	{
		for (int column = 0; column < GetColumnCount(); column++)
		{
			if (CellSelected(row, column))
				UnselectCell(row, column);
		}
	}
	
	DoCommand(this, this, kSelectionChangedCommandID);
}


bool TGridView::CellSelected(int row, int column) const
{
	return fSelection.Contains(TPoint(column, row));
}


bool TGridView::GetFirstSelectedCell(int& row, int& column) const
{	
	for (int i = 0; i < GetRowCount(); i++)
	{
		for (int j = 0; j < GetColumnCount(); j++)
		{
			if (CellSelected(i, j))
			{
				row = i;
				column = j;
				return true;
			}
		}
	}
	
	row = column = -1;
	return false;
}


bool TGridView::FindCell(const TPoint& point, int& outRow, int& outColumn) const
{
	if (point.h < 0 || point.h >= fContentSize.h ||
		point.v < 0 || point.v >= fContentSize.v)
		return false;

	TCoord h = 0;
	TCoord v = 0;

	for (int column = 0; column < GetColumnCount(); column++)
	{
		h += GetColumnWidth(column);
		if (h > point.h)
		{
			outColumn = column;
			break;
		}
	}

	for (int row = 0; row < GetRowCount(); row++)
	{
		v += GetRowHeight(row);
		if (v > point.v)
		{
			outRow = row;
			break;
		}
	}

	return true;
}


void TGridView::DoMouseDown(const TPoint& point, TMouseButton button, TModifierState state)
{
	if (button == kLeftButton)
	{
		int row, column;

		if (FindCell(point, row, column))
		{
			if (GetClickCount() == 1)
			{
				if (fMultiSelect && (state & ControlMask))
				{
					if (CellSelected(row, column))
						UnselectCell(row, column);
					else
					{
						TRect	r(column, row, column + 1, row + 1);
						SelectCellRange(r, true, true);
					}
				}
				else if (fMultiSelect && (state & ShiftMask) && fLastCellClick.h >= 0 && fLastCellClick.v >= 0)
				{
					TRect	r(fLastCellClick, TPoint(column, row));
					r.right++;
					r.bottom++;
					SelectCellRange(r, true, true);
				}
				else
				{
					UnselectAll();
					SelectCell(row, column);
				}

				DoCommand(this, this, kSelectionChangedCommandID);
				fLastCellClick.Set(column, row);
			}
			else if (GetClickCount() == 2 && AllowCellSelect(row, column, false))
			{
				CellDoubleClicked(row, column);
			}
			
			fTrackingMouse = true;
			fTrackingCell.v = row;
			fTrackingCell.h = column;
			fStartCell = fTrackingCell;
			
			ASSERT(fMouseTrackingIdler == NULL);
			fMouseTrackingIdler = new TMouseTrackingIdler(this);
		}
	}
}


void TGridView::DoMouseUp(const TPoint& point, TMouseButton button, TModifierState state)
{
	if (button == kLeftButton)
	{
		fTrackingMouse = false;
		
		if (fMouseTrackingIdler)
		{
			delete fMouseTrackingIdler;
			fMouseTrackingIdler = NULL;
		}
	}
}


void TGridView::DoMouseMoved(const TPoint& point, TModifierState state)
{
	if (fTrackingMouse)
	{
		int row, column;

		if (FindCell(point, row, column) &&
			(fTrackingCell.v != row || fTrackingCell.h != column))

		{
			if (fMultiSelect)
			{
				TRect	r1(TPoint(fStartCell.h, fTrackingCell.v), TPoint(fTrackingCell.h, row));
				TRect	r2(TPoint(fTrackingCell.h, fStartCell.v), TPoint(column, row));
								
				r1.right++;
				r1.bottom++;
				r2.right++;
				r2.bottom++;
				
				bool r1On = ((fStartCell.v <= fTrackingCell.v && fTrackingCell.v <= row) ||
							 (fStartCell.v >= fTrackingCell.v && fTrackingCell.v >= row));
				bool r2On = ((fStartCell.h <= fTrackingCell.h && fTrackingCell.h <= column) ||
							 (fStartCell.h >= fTrackingCell.h && fTrackingCell.h >= column));
				
				SelectCellRange(r1, r1On, true);
				SelectCellRange(r2, r2On, true);
			}
			else
			{
				UnselectAll();
				SelectCell(row, column);
			}
			
			TRect bounds;
			GetCellBounds(row, column, bounds);
			ScrollIntoView(bounds);
			
			fTrackingCell.v = row;
			fTrackingCell.h = column;

			DoCommand(this, this, kSelectionChangedCommandID);
		}
	}
}


bool TGridView::DoKeyDown(KeySym key, TModifierState state, const char* string)
{
	switch (key)
	{
		case XK_Left:
			if (LeftArrowKey(state))
			{
				ScrollSelectionIntoView();
				DoCommand(this, this, kSelectionChangedCommandID);
			}
			return true;

		case XK_Right:
			if (RightArrowKey(state))
			{
				ScrollSelectionIntoView();
				DoCommand(this, this, kSelectionChangedCommandID);
			}
			return true;

		case XK_Up:
			if (UpArrowKey(state))
			{
				ScrollSelectionIntoView();
				DoCommand(this, this, kSelectionChangedCommandID);
			}
			return true;
			
		case XK_Down:
			if (DownArrowKey(state))
			{
				ScrollSelectionIntoView();
				DoCommand(this, this, kSelectionChangedCommandID);
			}
			return true;

		case XK_Page_Up:
			HandleCommand(this, this, kPageUpCommandID);
			return true;
			
		case XK_Page_Down:
			HandleCommand(this, this, kPageDownCommandID);
			return true;

		default:
			break;
	}	

	return TView::DoKeyDown(key, state, string);
}


bool TGridView::IsTargetable() const
{
	return true;
}


bool TGridView::LeftArrowKey(TModifierState state)
{
	if (fLastCellClick.h < 0 || fLastCellClick.v < 0)
		fLastCellClick.Set(0, 0);

	if (GetColumnCount() > 1)
	{
		if (fLastCellClick.h > 0)
			--fLastCellClick.h;
		else
			fLastCellClick.h = GetColumnCount() - 1;

		if (!(fMultiSelect && (state & ShiftMask)))
			UnselectAll();

		SelectCell(fLastCellClick.v, fLastCellClick.h);
	}

	return true;
}


bool TGridView::RightArrowKey(TModifierState state)
{
	if (fLastCellClick.h < 0 || fLastCellClick.v < 0)
		fLastCellClick.Set(-1, 0);

	if (GetColumnCount() > 1)
	{
		if (fLastCellClick.h < GetColumnCount() - 1)
			++fLastCellClick.h;
		else
			fLastCellClick.h = 0;

		if (!(fMultiSelect && (state & ShiftMask)))
			UnselectAll();

		SelectCell(fLastCellClick.v, fLastCellClick.h);
	}

	return true;
}


bool TGridView::UpArrowKey(TModifierState state)
{
	if (fLastCellClick.h < 0 || fLastCellClick.v < 0)
		fLastCellClick.Set(0, 0);

	int rowCount = GetRowCount();
	if (rowCount > 0)
	{
		--fLastCellClick.v;
		if (fLastCellClick.v < 0 || fLastCellClick.v >= rowCount)
			fLastCellClick.v = rowCount - 1;

		if (!(fMultiSelect && (state & ShiftMask)))
			UnselectAll();

		SelectCell(fLastCellClick.v, fLastCellClick.h);
	}

	return true;
}


bool TGridView::DownArrowKey(TModifierState state)
{
	if (fLastCellClick.h < 0 || fLastCellClick.v < 0)
		fLastCellClick.Set(0, -1);

	if (GetRowCount() > 0)
	{
		if (fLastCellClick.v < GetRowCount() - 1)
			++fLastCellClick.v;
		else
			fLastCellClick.v = 0;

		if (!(fMultiSelect && (state & ShiftMask)))
			UnselectAll();

		SelectCell(fLastCellClick.v, fLastCellClick.h);
	}

	return true;
}


void TGridView::CellDoubleClicked(int row, int column)
{
}


TCoord TGridView::GetScrollIncrement(TScrollDirection direction) const
{
	TRect	bounds;
	TRect	cells;
	GetVisibleCells(cells);

	switch (direction)
	{
		case kScrollLeft:
			if (cells.left > 0)
			{
				GetCellBounds(cells.top, cells.left - 1, bounds);
				return bounds.left - fScroll.h;
			}
			else
				return -fScroll.h;
				
		case kScrollRight:
			if (cells.right < GetColumnCount() - 1)
			{
				GetCellBounds(cells.top, cells.left + 1, bounds);
				return bounds.left - fScroll.h;
			}
			else
				return fContentSize.h - fBounds.GetWidth() - fScroll.h;

		case kScrollUp:
			if (cells.top > 0)
			{
				GetCellBounds(cells.top - 1, cells.left, bounds);
				return bounds.top - fScroll.v;
			}
			else
				return -fScroll.v;

		case kScrollDown:
			if (cells.bottom < GetRowCount() - 1)
			{
				GetCellBounds(cells.top + 1, cells.left, bounds);
				return bounds.top - fScroll.v;
			}
			else
				return fContentSize.v - fBounds.GetHeight() - fScroll.v;
	}

	ASSERT(0);	// shouldn't get here, but make the compiler happy
	return 0;
}


TCoord TGridView::GetPageIncrement(TScrollDirection direction) const
{
	TRect	bounds;
	TRect 	cells;
	GetVisibleCells(cells);

	switch (direction)
	{
		case kScrollLeft:
		{
			TCoord h = fScroll.h - GetWidth();
			if (h > 0)
			{
				int row, column;
				
				if (FindCell(TPoint(h, fScroll.v), row, column) && column < GetColumnCount() - 1)
				{
					GetCellBounds(row, column + 1, bounds);
					return bounds.right - fScroll.h;
				}
				else
					return -GetWidth();
			}
			else 
				return -fScroll.h;
		}
				
		case kScrollRight:
			GetCellBounds(cells.top, cells.right, bounds);
			if (bounds.left - fScroll.h > 0) 
				return bounds.left - fScroll.h;
			else
				return GetWidth();

		case kScrollUp:
		{
			TCoord v = fScroll.v - GetHeight();
			if (v > 0)
			{
				int row, column;
				
				if (FindCell(TPoint(fScroll.h, fScroll.v - GetHeight()), row, column) && row < GetRowCount() - 1)
				{
					GetCellBounds(row + 1, column, bounds);
					return bounds.bottom - fScroll.v;
				}
				else
					return -GetHeight();
			}
			else
				return -fScroll.v;
		}

		case kScrollDown:
			if (cells.bottom == 0)
				return 0;
			GetCellBounds(cells.bottom - 1, cells.left, bounds);
			if (bounds.top - fScroll.v > 0)
				return bounds.top - fScroll.v;
			else
				return GetHeight();
	}

	ASSERT(0);	// shouldn't get here, but make the compiler happy
	return 0;
}


void TGridView::ScrollSelectionIntoView()
{
	// only works with single selection!
	for (int i = 0; i < GetRowCount(); i++)
		for (int j = 0; j < GetColumnCount(); j++)
			if (CellSelected(i, j))
			{
				TRect bounds;
				GetCellBounds(i, j, bounds);
				ScrollIntoView(bounds);
				
				break;
			}
}


void TGridView::GetVisibleCells(TRect& cells) const
{
	TRect bounds;
	GetVisibleBounds(bounds);
	cells.Set(-1, -1, -1, -1);
	
	TCoord h = 0;
	TCoord v = 0;

	for (int column = 0; column < GetColumnCount(); column++)
	{
		h += GetColumnWidth(column);
		if (h > bounds.left && cells.left == -1)
		{
			cells.left = column;
		}
		else if (h > bounds.right && cells.right == -1)
		{
			cells.right = column;
			break;
		}
	}

	for (int row = 0; row < GetRowCount(); row++)
	{
		v += GetRowHeight(row);
		if (v > bounds.top && cells.top == -1)
		{
			cells.top = row;
		}
		else if (v > bounds.bottom && cells.bottom == -1)
		{
			cells.bottom = row;
			break;
		}
	}

	if (cells.right == -1)
		cells.right = GetColumnCount() - 1;
	if (cells.bottom == -1)
		cells.bottom = GetRowCount() - 1;
}

