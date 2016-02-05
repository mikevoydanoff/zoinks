// ========================================================================================
//	TRadio.h		 		   Copyright (C) 2001-2002 Mike Voydanoff. All rights reserved.
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

#ifndef __TRadio__
#define __TRadio__

#include "TWindow.h"
#include "TCommandID.h"


class TRadio : public TWindow
{
public:
								TRadio(TWindow* parent, const TRect& bounds, const TChar* title, TCommandID commandID);
	virtual 					~TRadio();
	
	inline bool					IsOn() const { return fOn; }
	void						SetOn(bool on);

	inline bool					IsEnabled() const { return fEnabled; }
	inline void					SetEnabled(bool enabled);

protected:
	virtual void				Draw(TRegion* clip);
	virtual void				DoMouseDown(const TPoint& point, TMouseButton button, TModifierState state);
	virtual void				DoMouseUp(const TPoint& point, TMouseButton button, TModifierState state);
	virtual void				DoMouseMoved(const TPoint& point, TModifierState state);

protected:
	TString						fTitle;
	TCommandID					fCommandID;
	bool						fOn;
	bool						fPressed;
	bool						fEnabled;
	bool						fTrackingMouse;
};

#endif // __TRadio__
