// ========================================================================================
//	TCommandHandler.h		 	Copyright (C) 2001-2002 Mike Lockwood. All rights reserved.
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

#ifndef __TCommandHandler__
#define __TCommandHandler__

#include "TCommandID.h"
#include "TGeometry.h"

#include <X11/X.h>

class TBehavior;
class TMenu;
class TDocument;


enum TMouseButton
{
	kLeftButton = 1,
	kMiddleButton = 2,
	kRightButton = 3
};


typedef unsigned int TModifierState;

class TCommandHandler
{
public:
								TCommandHandler(TCommandHandler* nextHandler);
	virtual						~TCommandHandler();

	void						AddBehavior(TBehavior* behavior);
	void						RemoveBehavior(TBehavior* behavior);

	virtual void				HandleSetupMenu(TMenu* menu);
	virtual void				DoSetupMenu(TMenu* menu);

	virtual bool				HandleCommand(TCommandHandler* sender, TCommandHandler* receiver, TCommandID command);
	virtual bool				DoCommand(TCommandHandler* sender, TCommandHandler* receiver, TCommandID command);

	virtual bool				HandleKeyDown(KeySym key, TModifierState state, const char* string);
	virtual bool				DoKeyDown(KeySym key, TModifierState state, const char* string);

	virtual bool				HandleKeyUp(KeySym key, TModifierState state, const char* string);
	virtual bool				DoKeyUp(KeySym key, TModifierState state, const char* string);

	virtual void				HandleMouseDown(const TPoint& point, TMouseButton button, TModifierState state);
	virtual void				DoMouseDown(const TPoint& point, TMouseButton button, TModifierState state);
	virtual void				HandleMouseUp(const TPoint& point, TMouseButton button, TModifierState state);
	virtual void				DoMouseUp(const TPoint& point, TMouseButton button, TModifierState state);
	virtual void				HandleMouseMoved(const TPoint& point, TModifierState state);
	virtual void				DoMouseMoved(const TPoint& point, TModifierState state);
	virtual void				HandleMouseEnter(const TPoint& point, TModifierState state);
	virtual void				DoMouseEnter(const TPoint& point, TModifierState state);
	virtual void				HandleMouseLeave(const TPoint& point, TModifierState state);
	virtual void				DoMouseLeave(const TPoint& point, TModifierState state);
	virtual void				HandleScrollWheel(bool down);
	virtual void				DoScrollWheel(bool down);

	virtual bool				DismissesDialog(TCommandID command) const;

	inline TCommandHandler*		GetNextHandler() const { return fNextHandler; }
	inline void					SetNextHandler(TCommandHandler* nextHandler) { fNextHandler = nextHandler; }

	TDocument*					GetDocument();

protected:
	TCommandHandler*			fNextHandler;
	TBehavior*					fBehaviors;
};


#endif // __TCommandHandler__
