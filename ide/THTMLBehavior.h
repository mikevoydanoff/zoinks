// ========================================================================================
//	THTMLBehavior.h			   Copyright (C) 2001-2002 Mike Voydanoff. All rights reserved.
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

#ifndef __THTMLBehavior__
#define __THTMLBehavior__

#include "fw/TBehavior.h"

class TLogDocument;
class TPopupMenu;

class THTMLBehavior : public TBehavior
{
public:
						THTMLBehavior();
	virtual				~THTMLBehavior();

	virtual void		DoSetupMenu(TMenu* menu);
	virtual bool		DoCommand(TCommandHandler* sender, TCommandHandler* receiver, TCommandID command);

	virtual bool		DoMouseDown(const TPoint& point, TMouseButton button, TModifierState state);


protected:
	bool				DoHTMLCommand(TCommandID command);
	bool				DoAttributeCommand(TCommandID command);
	bool				AddLineBreakTags();
	void				FixSpecialCharacters();
	void				InsertImage(const char* documentBase);

#ifdef HAVE_LIBIMLIB
	void				InsertThumbnail(const char* documentBase);
#endif

	void				WebLint();

protected:
	TLogDocument*		fLogDocument;
	TPopupMenu*			fCurrentPopup;
};

#endif // __THTMLBehavior__
