// ========================================================================================
//	TDiffDialogs.h			 	Copyright (C) 2001-2002 Mike Lockwood. All rights reserved.
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

#ifndef __TDiffDialogs__
#define __TDiffDialogs__

#include "fw/TBehavior.h"
#include "fw/TPopupMenu.h"

class TDirectory;
class TFile;
class TRadio;
class TTextField;


class TDiffDialogs
{
public:
	static bool				CompareFilesDialog(TString& path1, TString& path2, bool& compareDirectories);
};


class TTextDocumentsPopupMenu : public TPopupMenu
{
public:
								TTextDocumentsPopupMenu();
	virtual						~TTextDocumentsPopupMenu();
	
	virtual void				PreDisplayMenu();
};


// behavior to attach to our TDialogWindow
class TDiffDialogBehavior : public TBehavior
{
public:
								TDiffDialogBehavior(TTextField* file1TextField, TTextField* file2TextField, 
													TTextDocumentsPopupMenu* file1PopupMenu, TTextDocumentsPopupMenu* file2PopupMenu,
													TRadio*	compareFilesRadio, TRadio* compareDirectoriesRadio,
													bool compareDirectories);
	virtual						~TDiffDialogBehavior();

	virtual bool				DoCommand(TCommandHandler* sender, TCommandHandler* receiver, TCommandID command);
	
	inline bool					CompaareDirectories() const { return fCompareDirectories; }
		
protected:
	TTextField*					fFile1TextField;
	TTextField*					fFile2TextField;
	TTextDocumentsPopupMenu*	fFile1PopupMenu;
	TTextDocumentsPopupMenu*	fFile2PopupMenu;
	TRadio*						fCompareFilesRadio;
	TRadio*						fCompareDirectoriesRadio;
	bool						fCompareDirectories;
	
};


#endif // __TDiffDialogs__
