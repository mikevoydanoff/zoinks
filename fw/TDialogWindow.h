// ========================================================================================
//	TDialogWindow.h			   Copyright (C) 2001-2002 Mike Voydanoff. All rights reserved.
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

#ifndef __TDialogWindow__
#define __TDialogWindow__

#include "TTopLevelWindow.h"

class TButton;


class TDialogWindow : public TTopLevelWindow
{
public:
								TDialogWindow(const TRect& bounds, const TChar* title, bool modal, TWindow* owner);
	
	virtual void				Create();
	virtual void				DoClose();

	virtual bool				DoKeyDown(KeySym key, TModifierState state, const char* string);
	virtual bool				DoCommand(TCommandHandler* sender, TCommandHandler* receiver, TCommandID command);

	inline TDialogWindow*		GetNextDialog() const { return fNextDialog; }
	inline void					SetNextDialog(TDialogWindow* dialog) { fNextDialog = dialog; }
	
	inline bool					IsDismissed() const { return fDismissed; }
	inline void					SetDismissed() { fDismissed = true; }
	
	inline bool					IsOKEnabled() const { return fOKEnabled; }
	void						SetOKEnabled(bool okEnabled);

	inline TButton*				GetOKButton() const { return fOKButton; }
	inline void					SetOKButton(TButton* okButton) { fOKButton = okButton; }

protected:
	virtual						~TDialogWindow();

protected:
	TWindow*					fOwner;
	TDialogWindow*				fNextDialog;		// linked list for TApplication
	TButton*					fOKButton;
	bool						fDismissed;
	bool						fModal;
	bool						fOKEnabled;
};


#endif // __TDialogWindow__
