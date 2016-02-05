// ========================================================================================
//	TTopLevelWindow.h		   Copyright (C) 2001-2002 Mike Voydanoff. All rights reserved.
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

#ifndef __TTopLevelWindow__
#define __TTopLevelWindow__

#include "TWindow.h"

class TMenuBar;
class TFont;
class TPixmap;


class TTopLevelWindow : public TWindow
{
public:
								TTopLevelWindow(const TRect& bounds, const TChar* title);				

	virtual TTopLevelWindow*	GetTopLevelWindow();

	virtual void				Create();
	virtual void				DoClose();
	virtual void				Show(bool doShow);

	inline const TString& 		GetTitle() const { return fTitle; }
	void						SetTitle(const TChar* title);

	inline TMenuBar*			GetMenuBar() const { return fMenuBar; }
	inline void					SetMenuBar(TMenuBar* menuBar) { fMenuBar = menuBar; }

	void						DispatchKeyEvent(XEvent& event);

	inline TWindow*				GetTarget() const { return fTarget; }
	void						SetTarget(TWindow* target);
	inline void					TargetDied() { fTarget = NULL; }

	inline void					SetHideOnClose(bool hideOnClose) {fHideOnClose = hideOnClose; }

	void						GotFocus(TWindow* window);
	void						LostFocus(TWindow* window);
	
	inline bool					HasFocusedWindow() const { return (fTarget && fTarget->HasFocus()); }
	
	void						SetIcon(TPixmap* icon);
	static inline void			SetDefaultIcon(TPixmap* icon) { sDefaultIcon = icon; };

	static inline TList<TTopLevelWindow>& GetWindowList() { return sWindowList; }
	
protected:
	virtual						~TTopLevelWindow();
	void						DoSetIcon(TPixmap* icon);
	
protected:
	TString						fTitle;
	TMenuBar*					fMenuBar;
	TWindow*					fTarget;			// window that should get keyboard focus
	bool						fHideOnClose;
	
	TPixmap*					fIcon;
	static TPixmap*				sDefaultIcon;

	static TList<TTopLevelWindow>	sWindowList;
};

#endif // __TTopLevelWindow__
