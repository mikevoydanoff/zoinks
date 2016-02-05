// ========================================================================================
//	TTextFindBehavior.h		   Copyright (C) 2001-2002 Mike Voydanoff. All rights reserved.
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

#ifndef __TTextFindBehavior__
#define __TTextFindBehavior__

#include "TBehavior.h"
#include "TString.h"

class TTextView;
class TTextField;
class TCheckBox;
class TDialogWindow;
class TFindDialogBehavior;
class TSettingsFile;


// behavior to attach to TTextField
class TTextFindBehavior : public TBehavior
{
public:
								TTextFindBehavior();
	virtual						~TTextFindBehavior();

	virtual void				DoSetupMenu(TMenu* menu);
	virtual bool				DoCommand(TCommandHandler* sender, TCommandHandler* receiver, TCommandID command);
	
	inline bool					GetCaseSensitive() const { return sCaseSensitive; }
	inline void					SetCaseSensitive(bool caseSensitive) { sCaseSensitive = caseSensitive; }
	inline bool					GetForward() const { return sForward; }
	inline void					SetForward(bool forward) { sForward = forward; }
	inline bool					GetWrap() const { return sWrap; }
	inline void					SetWrap(bool wrap) { sWrap = wrap; }
	inline bool					GetWholeWord() const { return sWholeWord; }
	inline void					SetWholeWord(bool wrap) { sWholeWord = wrap; }
	
	static void					ReadFromSettingsFile(TSettingsFile* settingsFile);
	static void					WriteToSettingsFile(TSettingsFile* settingsFile);

protected:
	void						Search(const TChar* searchString);
	void						Replace(const TChar* searchString, const TChar* replaceString);
	void						ReplaceAll(const TChar* searchString, const TChar* replaceString);
	
	void						OpenDialog();
	bool						FindNext(bool forward);
	void						FindSelection(bool forward);
	void						ReplaceSelectionAndFind(bool forward);

	TTextView*					GetView();

protected:
	friend class TFindDialogBehavior;
	
	static TString				sSearchString;
	static TString				sReplaceString;
	static bool					sCaseSensitive;
	static bool					sForward;
	static bool					sWrap;
	static bool					sWholeWord;

	static TDialogWindow*		sDialog;
	static TTextField*			sSearchTextView;
	static TFindDialogBehavior*	sDialogBehavior;
};


// behavior to attach to our TDialogWindow
class TFindDialogBehavior : public TBehavior
{
public:
								TFindDialogBehavior(TTextField* searchTextView, TTextField* replaceTextView,
													TCheckBox* caseSensitiveBox, TCheckBox* backwardsBox, 
													TCheckBox* wrapBox, TCheckBox* wholeWordBox);
	virtual						~TFindDialogBehavior();

	virtual bool				DoCommand(TCommandHandler* sender, TCommandHandler* receiver, TCommandID command);
	virtual bool				DoKeyDown(KeySym key, TModifierState state, const char* string);

	inline TTextFindBehavior*	GetCurrentBehavior() const { return fTextFindBehavior; }
	inline void					SetCurrentBehavior(TTextFindBehavior* behavior) { fTextFindBehavior = behavior; }
		
protected:
	TTextField*					fSearchTextView;
	TTextField*					fReplaceTextView;
	TCheckBox*					fCaseSensitiveBox;
	TCheckBox*					fBackwardsBox;
	TCheckBox*					fWrapBox;
	TCheckBox*					fWholeWordBox;
	TTextFindBehavior*			fTextFindBehavior;
};


#endif // __TTextFindBehavior__
