// ========================================================================================
//	TDirectoryDiffDocument.h		Copyright (C) 2003 Mike Voydanoff. All rights reserved.
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

#ifndef __TDirectoryDiffDocument__
#define __TDirectoryDiffDocument__

#include "fw/TDirectory.h"
#include "fw/TList.h"
#include "fw/TWindowContext.h"

class TDocumentWindow;
class TMenuBar;
class TWindow;
class TDirectoryDiffListView;


class TDirectoryDiffDocument : public TWindowContext
{
public:
	static TDirectoryDiffDocument*	CreateDocument(const TDirectory& directory1, const TDirectory& directory2);

protected:
							TDirectoryDiffDocument(const TDirectory& directory1, const TDirectory& directory2);
	virtual					~TDirectoryDiffDocument();

	virtual void			Open(TDocumentWindow* window);

	virtual void			GetTitle(TString& title) const;

	void					DoCompareDirectories(TDirectory& directory1, TDirectory& directory2);
	void					ComputeDiffs();

	TMenuBar*				MakeMenuBar(TWindow* window);

	virtual void			DoSetupMenu(TMenu* menu);
	virtual bool			DoCommand(TCommandHandler* sender, TCommandHandler* receiver, TCommandID command);

protected:
	TDirectory				fDirectory1;
	TDirectory				fDirectory2;
	TDirectoryDiffListView*	fDirectoryDiffListView;
	TList<TString>			fDiffList;		// each string is a path relative to base directory

};

#endif // __TDirectoryDiffDocument__
