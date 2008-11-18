// ========================================================================================
//	TColumnResizer.h		 	Copyright (C) 2001-2002 Mike Lockwood. All rights reserved.
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

#ifndef __TColumnResizer__
#define __TColumnResizer__

#include "TBehavior.h"
#include "TGridView.h"


class TColumnResizer : public TBehavior
{
public:
								TColumnResizer();
	virtual						~TColumnResizer();

	virtual bool				DoMouseDown(const TPoint& point, TMouseButton button, TModifierState state);
	virtual bool				DoMouseUp(const TPoint& point, TMouseButton button, TModifierState state);
	virtual bool				DoMouseMoved(const TPoint& point, TModifierState state);

protected:
	inline TGridView*			GetGridView() const { return dynamic_cast<TGridView*>(fOwner); }
	
	int							FindColumnBoundary(const TPoint& point) const;
	void						GetTrackingRect(const TPoint& point, TRect& rect);
	void						DrawTrackingRect(const TRect& rect);

protected:
	TPoint						fStartMouse;
	TPoint						fLastMouse;
	int							fCurrentColumn;		// column we are over, or -1 for none
	bool						fTracking;
};

#endif // __TColumnResizer__
