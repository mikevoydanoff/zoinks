// ========================================================================================
//	TWindowPositioners.cpp	   Copyright (C) 2001-2002 Mike Voydanoff. All rights reserved.
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

#include "TWindowPositioners.h"
#include "TWindow.h"


void SizeParent(TWindow* window, 
				const TRect& /*oldParentBounds*/,
				const TRect& newParentBounds)
{
	TRect	bounds(0, 0, newParentBounds.GetWidth(), newParentBounds.GetHeight());
	window->SetBounds(bounds);
}


void SizeRelativeParent(TWindow* window, 
						const TRect& oldParentBounds,
						const TRect& newParentBounds)
{
	TCoord deltaH = newParentBounds.GetWidth() - oldParentBounds.GetWidth();
	TCoord deltaV = newParentBounds.GetHeight() - oldParentBounds.GetHeight();

	TRect	bounds(window->GetBounds());
	bounds.right += deltaH;
	bounds.bottom += deltaV;

	window->SetBounds(bounds);
}


void WidthParent(TWindow* window, 
				const TRect& oldParentBounds,
				const TRect& newParentBounds)
{
	TRect bounds = window->GetBounds();
	bounds.left = 0;
	bounds.right = newParentBounds.GetWidth();

	window->SetBounds(bounds);
}


void HeightParent(TWindow* window, 
				const TRect& oldParentBounds,
				const TRect& newParentBounds)
{
	TRect bounds = window->GetBounds();
	bounds.top = 0;
	bounds.bottom = newParentBounds.GetHeight();

	window->SetBounds(bounds);
}


void WidthRelativeParent(TWindow* window, 
						const TRect& oldParentBounds,
						const TRect& newParentBounds)
{
	TCoord deltaH = newParentBounds.GetWidth() - oldParentBounds.GetWidth();

	TRect	bounds(window->GetBounds());
	bounds.right += deltaH;

	window->SetBounds(bounds);
}


void HeightRelativeParent(TWindow* window, 
						const TRect& oldParentBounds,
						const TRect& newParentBounds)
{
	TCoord deltaV = newParentBounds.GetHeight() - oldParentBounds.GetHeight();

	TRect	bounds(window->GetBounds());
	bounds.bottom += deltaV;

	window->SetBounds(bounds);
}


void RightRelative(TWindow* window, 
				const TRect& oldParentBounds,
				const TRect& newParentBounds)
{
	TRect bounds = window->GetBounds();
	bounds.Offset(newParentBounds.GetWidth() - oldParentBounds.GetWidth(), 0);

	window->SetBounds(bounds);
}


void BottomRelative(TWindow* window, 
				const TRect& oldParentBounds,
				const TRect& newParentBounds)
{
	TRect bounds = window->GetBounds();
	bounds.Offset(0, newParentBounds.GetHeight() - oldParentBounds.GetHeight());

	window->SetBounds(bounds);
}


void BottomRightRelative(TWindow* window, 
				const TRect& oldParentBounds,
				const TRect& newParentBounds)
{
	TRect bounds = window->GetBounds();
	bounds.Offset(newParentBounds.GetWidth() - oldParentBounds.GetWidth(), newParentBounds.GetHeight() - oldParentBounds.GetHeight());

	window->SetBounds(bounds);
}


void RightScrollBar(TWindow* window, 
				const TRect& oldParentBounds,
				const TRect& newParentBounds)
{
	TCoord deltaH = newParentBounds.GetWidth() - oldParentBounds.GetWidth();
	TCoord deltaV = newParentBounds.GetHeight() - oldParentBounds.GetHeight();

	TRect	bounds(window->GetBounds());

	bounds.left += deltaH;
	bounds.right += deltaH;
	bounds.bottom += deltaV;
	
	window->SetBounds(bounds);
}


void BottomScrollBar(TWindow* window, 
				const TRect& oldParentBounds,
				const TRect& newParentBounds)
{
	TCoord deltaH = newParentBounds.GetWidth() - oldParentBounds.GetWidth();
	TCoord deltaV = newParentBounds.GetHeight() - oldParentBounds.GetHeight();

	TRect	bounds(window->GetBounds());

	bounds.top += deltaV;
	bounds.bottom += deltaV;
	bounds.right += deltaH;
	
	window->SetBounds(bounds);
}


void CenterHorizontal(TWindow* window, 
				const TRect& oldParentBounds,
				const TRect& newParentBounds)
{
	TRect	bounds(window->GetBounds());

	TCoord deltaH = (newParentBounds.GetWidth() - bounds.GetWidth()) / 2 - bounds.left;

	bounds.left += deltaH;
	bounds.right += deltaH;

	window->SetBounds(bounds);
}


void BottomRelativeCenterHorizontal(TWindow* window, 
				const TRect& oldParentBounds,
				const TRect& newParentBounds)
{
	TRect	bounds(window->GetBounds());

	TCoord deltaH = (newParentBounds.GetWidth() - bounds.GetWidth()) / 2 - bounds.left;
	TCoord deltaV = newParentBounds.GetHeight() - oldParentBounds.GetHeight();

	bounds.left += deltaH;
	bounds.right += deltaH;
	bounds.top += deltaV;
	bounds.bottom += deltaV;

	window->SetBounds(bounds);
}

