// ========================================================================================
//	TColumnResizer.cpp		 	Copyright (C) 2001-2002 Mike Lockwood. All rights reserved.
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

#include "TColumnResizer.h"
#include "TDrawContext.h"
#include "TFWCursors.h"

const TCoord kColumnSlop = 2;


TColumnResizer::TColumnResizer()
	:	fCurrentColumn(-1),
		fTracking(false)
{
}


TColumnResizer::~TColumnResizer()
{
}


bool TColumnResizer::DoMouseDown(const TPoint& point, TMouseButton button, TModifierState state)
{
	if (fCurrentColumn >= 0)
	{
		if (fTracking)
			fTracking = false;
		else
		{
			TRect	rect;
			
			GetTrackingRect(point, rect);
			DrawTrackingRect(rect);
		
			fStartMouse = fLastMouse = point;
			fTracking = true;
		}

		return true;
	}
	
	return false;
}


bool TColumnResizer::DoMouseUp(const TPoint& point, TMouseButton button, TModifierState state)
{
	if (fTracking)
	{
		TRect	rect;

		GetTrackingRect(fLastMouse, rect);
		DrawTrackingRect(rect);
		
		if (fStartMouse.h != point.h)
		{
			// resize the column
			
			TGridView* gridView = GetGridView();
			TCoord width = gridView->GetColumnWidth(fCurrentColumn);
			gridView->SetColumnWidth(fCurrentColumn, width + point.h - fStartMouse.h);
		}

		fTracking = false;
		return true;
	}
	else
		return false;
}


bool TColumnResizer::DoMouseMoved(const TPoint& point, TModifierState state)
{
	if (fTracking)
	{
		TRect	rect;

		GetTrackingRect(fLastMouse, rect);
		DrawTrackingRect(rect);
		GetTrackingRect(point, rect);
		DrawTrackingRect(rect);
						
		fLastMouse = point;
	}
	else
	{
		int column = FindColumnBoundary(point);
		
		if (column != fCurrentColumn)
		{
			TGridView* gridView = GetGridView();
			
			if (column >= 0)
				gridView->SetCursor(GetVertSplitterCursor());
			else
				gridView->SetCursor(TWindow::GetDefaultCursor());
		
			fCurrentColumn = column;
		}
	}
	
	return false;
}


int TColumnResizer::FindColumnBoundary(const TPoint& point) const
{
	int result = -1;
	TCoord horiz = 0;
	
	TGridView* gridView = GetGridView();
	ASSERT(gridView);
	
	TRect contentBounds;
	gridView->GetContentBounds(contentBounds);
	
	if (point.v >= contentBounds.top && point.v < contentBounds.bottom)
	{
		int columnCount = gridView->GetColumnCount() - 1;
		
		for (int i = 0; i < columnCount; i++)
		{
			horiz += gridView->GetColumnWidth(i);
			
			TCoord distance = horiz - point.h;
			if (distance < kColumnSlop && distance > -kColumnSlop)
			{
				if (gridView->IsColumnResizable(i))
					result = i;
	
				break;
			}
		}
	}
	
	return result;
}


void TColumnResizer::GetTrackingRect(const TPoint& point, TRect& rect)
{
	TGridView* gridView = GetGridView();
	gridView->GetLocalBounds(rect);

	rect.left = point.h;
	rect.right = rect.left + 1;

	if (rect.left < 0)
		rect.Offset(-rect.left, 0);
	else if (rect.right > gridView->GetWidth())
		rect.Offset(gridView->GetWidth() - rect.right, 0);
}


void TColumnResizer::DrawTrackingRect(const TRect& rect)
{	
	TDrawContext context(GetGridView());
	context.SetForeColor(kMediumGrayColor);
	
	context.ClipSubWindows(false);
	context.SetPenMode(kXorMode);
	context.PaintRect(rect);
}

