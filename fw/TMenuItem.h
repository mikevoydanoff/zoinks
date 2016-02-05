// ========================================================================================
//	TMenuItem.h		 		   Copyright (C) 2001-2002 Mike Voydanoff. All rights reserved.
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

#ifndef __TMenuItem__
#define __TMenuItem__

#include "TString.h"
#include "TWindow.h"
#include "TCommandID.h"


struct TMenuItemRec
{
	const TChar*	fTitle;
	TCommandID		fCommandID;
	TModifierState	fShortCutModifier;
	TChar			fShortCutKey;
	TMenuItemRec*	fSubMenu;
};


class TMenuItem
{
public:
							TMenuItem(const TChar* title, TCommandID commandID = kNoCommandID, 
										TModifierState shortCutModifier = 0, TChar shortCutKey = 0);
	virtual 				~TMenuItem();

	inline const TString& 	GetTitle() const { return fTitle; }
	inline TCommandID	 	GetCommandID() const { return fCommandID; }
	inline TModifierState	GetShortCutModifier() const { return fShortCutModifier; }	
	inline TModifierState	GetShortCutKey() const { return fShortCutKey; }	
	inline bool				IsEnabled() const { return fEnabled; }
	inline bool				IsChecked() const { return fChecked; }

	virtual TMenuItem*		FindShortCut(KeySym key, TModifierState state, const char* string);
	void					GetShortCutLabel(TString& shortCutString);

	void					Enable(bool enable, bool check = false);

	// used in menu setup
	virtual void			DisableAll();
	virtual void			EnableCommand(TCommandID command, bool check);

	virtual bool			IsSeparator() const;

protected:
	TString					fTitle;
	TCommandID				fCommandID;
	TModifierState			fShortCutModifier;
	TChar					fShortCutKey;
	bool					fEnabled;
	bool					fChecked;
};

#endif // __TMenuItem__
