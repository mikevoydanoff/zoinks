// ========================================================================================
//	TEditorTextView.cpp		 	Copyright (C) 2001-2002 Mike Lockwood. All rights reserved.
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

#include "IDECommon.h"

#include "TEditorTextView.h"
#include "fw/TCursor.h"
#include "fw/TDrawContext.h"

#include <X11/cursorfont.h>


TCursor* TEditorTextView::sLeftArrowCursor = NULL;



TEditorTextView::TEditorTextView(TWindow* parent, const TRect& bounds, TFont* font)
	:	TSyntaxTextView(parent, bounds, font, true),
		fLeftColumnWidth(12),
		fMouseInLeftColumn(false),
		fTrackingLeftClick(false)
{
	fInset.left += fLeftColumnWidth;
	fLayout->SetInset(TPoint(fInset.left, fInset.top));
}	


TEditorTextView::~TEditorTextView()
{
}


void TEditorTextView::Create()
{
	TSyntaxTextView::Create();

	if (! sLeftArrowCursor)
		sLeftArrowCursor = new TCursor(XC_right_ptr);

	if (fModifiable && HasFocus())
		EnableIdling(true);
}


void TEditorTextView::DoMouseDown(const TPoint& point, TMouseButton button, TModifierState state)
{
	if (button == kLeftButton && point.h - fScroll.h < fLeftColumnWidth)
	{
		uint32 line = fLayout->VertOffsetToLine(point.v);
		fTrackingLine = line;

		const TChar* lineText;
		STextOffset lineLength;
		TCoord vertOffset, lineAscent, lineHeight;
		fLayout->GetLine(line, lineText, lineLength, vertOffset, lineAscent, lineHeight);

		if (point.v < vertOffset + lineHeight)
		{
			STextOffset offset = fLayout->LineToOffset(line);
			fTrackingLineEnd = offset + lineLength;
			SetSelection(offset, fTrackingLineEnd);
			fSelectionAnchor = offset;
			fSelectionAnchorEnd = fTrackingLineEnd;
		}

		fTrackingLeftClick = true;
	}
	else
		TSyntaxTextView::DoMouseDown(point, button, state);
}


void TEditorTextView::DoMouseUp(const TPoint& point, TMouseButton button, TModifierState state)
{
	if (fTrackingLeftClick)
		fTrackingLeftClick = false;
	else
		TSyntaxTextView::DoMouseUp(point, button, state);
}


void TEditorTextView::DoMouseMoved(const TPoint& point, TModifierState state)
{
	bool inLeftColumn = (point.h - fScroll.h < fLeftColumnWidth);

	if (fCursorHidden || fMouseInLeftColumn != inLeftColumn)
	{
		SetCursor(inLeftColumn ? sLeftArrowCursor : sCursor);
		fMouseInLeftColumn = inLeftColumn;
		fCursorHidden = false;
	}

	if (fTrackingLeftClick)
	{
		uint32 line = fLayout->VertOffsetToLine(point.v);
	
		const TChar* lineText;
		STextOffset lineLength;
		TCoord vertOffset, lineAscent, lineHeight;
		fLayout->GetLine(line, lineText, lineLength, vertOffset, lineAscent, lineHeight);

		if (line > fTrackingLine)
			SetSelection(fLayout->LineToOffset(fTrackingLine), fLayout->LineToOffset(line) + lineLength);
		else
			SetSelection(fLayout->LineToOffset(line), fTrackingLineEnd);
			
		ScrollSelectionIntoView(true, false);
	}

	TSyntaxTextView::DoMouseMoved(point, state);
}


void TEditorTextView::Draw(TRegion* clip)
{
	TSyntaxTextView::Draw(clip);

	TDrawContext	context(this, clip);
	TRect			r(0, 0, fLeftColumnWidth, (fContentSize.v > fBounds.bottom ? fContentSize.v : fBounds.bottom));
	// compensate for scrolling, so we will draw on the left edge even if we are scrolled
	r.Offset(fScroll.h, 0);
	
	// restrain r to avoid overflowing X11 16 bit coordinates.
	r.top = fScroll.v;
	r.bottom = r.top + GetHeight();
	
	context.SetForeColor(kLightGrayColor);
	context.PaintRect(r);
	
	context.SetForeColor(kDarkGrayColor);
	context.DrawLine(r.right, r.top, r.right, r.bottom);
}
