// ========================================================================================
//	TWindowPositioners.h	 	Copyright (C) 2001-2002 Mike Lockwood. All rights reserved.
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

#ifndef __TWindowPositioners__
#define __TWindowPositioners__

class TWindow;
class TRect;

void SizeParent(TWindow* window, 
				const TRect& oldParentBounds,
				const TRect& newParentBounds);

void SizeRelativeParent(TWindow* window, 
						const TRect& oldParentBounds,
						const TRect& newParentBounds);

void WidthParent(TWindow* window, 
				const TRect& oldParentBounds,
				const TRect& newParentBounds);

void HeightParent(TWindow* window, 
				const TRect& oldParentBounds,
				const TRect& newParentBounds);

void WidthRelativeParent(TWindow* window, 
				const TRect& oldParentBounds,
				const TRect& newParentBounds);

void HeightRelativeParent(TWindow* window, 
				const TRect& oldParentBounds,
				const TRect& newParentBounds);

void RightRelative(TWindow* window, 
				const TRect& oldParentBounds,
				const TRect& newParentBounds);

void BottomRelative(TWindow* window, 
				const TRect& oldParentBounds,
				const TRect& newParentBounds);

void BottomRightRelative(TWindow* window, 
				const TRect& oldParentBounds,
				const TRect& newParentBounds);

void RightScrollBar(TWindow* window, 
				const TRect& oldParentBounds,
				const TRect& newParentBounds);

void BottomScrollBar(TWindow* window, 
				const TRect& oldParentBounds,
				const TRect& newParentBounds);

void CenterHorizontal(TWindow* window, 
				const TRect& oldParentBounds,
				const TRect& newParentBounds);

void BottomRelativeCenterHorizontal(TWindow* window, 
				const TRect& oldParentBounds,
				const TRect& newParentBounds);
		
#endif // __TWindowPositioners__

