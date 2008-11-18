// ========================================================================================
//	TCommonDialogs.h		 	Copyright (C) 2001-2002 Mike Lockwood. All rights reserved.
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

#ifndef __TCommonDialogs__
#define __TCommonDialogs__

#include "TList.h"

class TFile;
class TDirectory;
class TWindow;
class TString;
class TBehavior;
class TDialogWindow;


class TCommonDialogs
{
public:
	static TFile*			OpenFile(TWindow* owner);
	static TList<TFile>*	OpenFiles(TWindow* owner);
	static TDirectory*		ChooseDirectory(TWindow* owner);

	static TFile*			SaveFile(const char* path, TWindow* owner);

	static void				AlertDialog(const TChar* prompt, const TChar* title, TWindow* owner,
										const TChar* okString = "OK");

	static bool				ConfirmDialog(const TChar* prompt, const TChar* title, TWindow* owner,
										const TChar* okString = "OK", 
										const TChar* cancelString = "Cancel");

	static bool				YesNoDialog(const TChar* prompt, const TChar* title, TWindow* owner, bool& result,
										const TChar* yesString = "Yes", 
										const TChar* noString = "No", 
										const TChar* cancelString = "Cancel");
										
	static bool				TextEntryDialog(const TChar* prompt, const TChar* title, TWindow* owner, TString& text, 
											TBehavior* textFilter = NULL,
											const TChar* okString = "OK", 
											const TChar* cancelString = "Cancel");
											
	static TDialogWindow*	CancelDialog(const TChar* prompt, const TChar* title, TWindow* owner,
										const TChar* cancelString = "Cancel");


	static bool				SaveChanges(const TChar* fileName, TWindow* owner, bool& save);
	
private:
	static TFile*			DoOpenFiles(TWindow* owner, bool allowMultiple, TList<TFile>*& fileList);
};


#endif // __TCommonDialogs__
