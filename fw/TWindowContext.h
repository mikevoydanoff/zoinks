// ========================================================================================
//	TWindowContext.h		 	Copyright (C) 2001-2002 Mike Lockwood. All rights reserved.
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

#ifndef __TWindowContext__
#define __TWindowContext__

#include "TList.h"
#include "TCommandHandler.h"

class TTopLevelWindow;
class TDocumentWindow;
class TString;

class TWindowContext : public TCommandHandler
{
public:
									TWindowContext(TCommandHandler* nextHandler);
	virtual							~TWindowContext();

	virtual void					Open(TDocumentWindow* window);
	virtual void					Close();

									// return true if we can close the window context
	virtual bool					AllowClose();

	virtual TTopLevelWindow*		GetMainWindow();

	virtual void					AddWindow(TTopLevelWindow* window);
	virtual void					RemoveWindow(TTopLevelWindow* window);

	virtual void					AddSubContext(TWindowContext* context);
	virtual void					RemoveSubContext(TWindowContext* context);
	virtual bool					CloseSubContexts();

	virtual void					GetTitle(TString& title) const = 0;

	inline TList<TWindowContext>&	GetSubContextList() { return fSubContexts; }

protected:
	TList<TTopLevelWindow>			fWindowList;
	TWindowContext*					fParentContext;
	TList<TWindowContext>			fSubContexts;
	bool							fClosing;
};


#endif // __TWindowContext__
