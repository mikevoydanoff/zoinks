// ========================================================================================
//	TBehavior.h				   Copyright (C) 2001-2002 Mike Voydanoff. All rights reserved.
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

#ifndef __TBehavior__
#define __TBehavior__

#include "TCommandID.h"
#include "TWindow.h"


class TBehavior
{
public:
								TBehavior();
	virtual						~TBehavior();

	inline TCommandHandler*		GetOwner() const { return fOwner; }
	inline TBehavior*			GetNextBehavior() const { return fNextBehavior; }
	inline void					SetNextBehavior(TBehavior* nextBehavior) { fNextBehavior = nextBehavior; }
	virtual void				SetOwner(TCommandHandler* owner, TBehavior* nextBehavior);

	virtual void				DoSetupMenu(TMenu* menu);
	virtual bool				DoCommand(TCommandHandler* sender, TCommandHandler* receiver, TCommandID command);

	virtual bool				DoKeyDown(KeySym key, TModifierState state, const char* string);
	virtual bool				DoKeyUp(KeySym key, TModifierState state, const char* string);

	virtual bool				DoMouseDown(const TPoint& point, TMouseButton button, TModifierState state);
	virtual bool				DoMouseUp(const TPoint& point, TMouseButton button, TModifierState state);
	virtual bool				DoMouseMoved(const TPoint& point, TModifierState state);
	virtual bool				DoMouseEnter(const TPoint& point, TModifierState state);
	virtual bool				DoMouseLeave(const TPoint& point, TModifierState state);
	virtual bool				DoScrollWheel(bool down);

protected:	
	TCommandHandler*			fOwner;
	TBehavior*					fNextBehavior;
};

#endif // __TBehavior__
