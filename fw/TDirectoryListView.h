// ========================================================================================
//	TDirectoryListView.h	   Copyright (C) 2001-2002 Mike Voydanoff. All rights reserved.
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

#ifndef __TDirectoryListView__
#define __TDirectoryListView__

#include "TTextListView.h"
#include "TList.h"
#include "TString.h"

class TDirectory;
class TPixmap;
class TFile;

class TDirectoryListView : public TTextListView
{
	class DirectoryItem
	{
		TString		name;
		bool		isDirectory;
		int			index;
		
		friend class TDirectoryListView;
	};
	
public:
								TDirectoryListView(TWindow* parent, const TRect& bounds, const TChar* directoryPath,
													 bool selectFiles);

	virtual TCoord				GetRowHeight(int row) const;
	virtual void				DrawCell(int row, int column, const TRect& cellBounds, TDrawContext& context);
	virtual bool				DoKeyDown(KeySym key, TModifierState state, const char* string);
	virtual void				SelectAll();

	virtual int					GetRowCount() const;

	virtual void				Create();

	virtual void				GetText(int row, TString& outText);

	inline TDirectory*			GetDirectory() const { return fDirectory; }
	bool						SetDirectory(TDirectory* directory);
	inline bool					IsDirectory(int index) const { return (*fFileList)[index]->isDirectory; }
	TFile*						GetFile(int index);
	TDirectory*					GetDirectory(int index);

	virtual void				CellDoubleClicked(int row, int column);

protected:				
	virtual						~TDirectoryListView();

	virtual bool				AllowCellSelect(int row, int column, bool multiple);
	virtual bool				DismissesDialog(TCommandID command) const;

	static int 					CompareItems(const DirectoryItem* item1, const DirectoryItem* item2, void* compareData);

protected:
	TDirectory*					fDirectory;
	TList<DirectoryItem>*		fFileList;
	TCoord						fRowHeight;
	TPixmap*					fFileIcon;
	TPixmap*					fFolderIcon;
	bool						fSelectFiles;
};

#endif // __TDirectoryListView__
