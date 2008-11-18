// ========================================================================================
//	TDirectoryListView.cpp	 	Copyright (C) 2001-2002 Mike Lockwood. All rights reserved.
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "FWCommon.h"

#include "TDirectoryListView.h"
#include "TDrawContext.h"
#include "TDirectory.h"
#include "TApplication.h"
#include "TCommonDialogs.h"
#include "TException.h"
#include "TPixmap.h"

#include "Pixmaps/FileIcon.xpm"
#include "Pixmaps/FolderIcon.xpm"
#include "Pixmaps/GrayFileIcon.xpm"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <X11/keysym.h>

#include "intl.h"


TDirectoryListView::TDirectoryListView(TWindow* parent, const TRect& bounds, const TChar* directoryPath, bool selectFiles)
	:	TTextListView(parent, bounds),
		fDirectory(NULL),
		fFileList(NULL),
		fRowHeight(0),
		fFileIcon(NULL),
		fFolderIcon(NULL),
		fSelectFiles(selectFiles)
{
	ASSERT(directoryPath);
	fColumnWidth = 10000;

	TDirectory* directory = NULL;
	directory = new TDirectory(directoryPath);

	if (!SetDirectory(directory))
	{
		delete directory;
		directory = new TDirectory(getenv("HOME"));
		SetDirectory(directory);
	}
	
	fFileIcon = new TPixmap(selectFiles ? FileIcon_xpm : GrayFileIcon_xpm);
	fFolderIcon = new TPixmap(FolderIcon_xpm);
}


TDirectoryListView::~TDirectoryListView()
{
	fDirectory->Close();
	delete fDirectory;

	delete fFileIcon;
	delete fFolderIcon;
}


bool TDirectoryListView::SetDirectory(TDirectory* directory)
{
	if (!directory)
		return false;
		
	if (directory != fDirectory)
	{
		// work around strange compiler warning here.
		TList<DirectoryItem>* fileList = NULL;
		fileList = new TList<DirectoryItem>((TListBase::CompareFunc)CompareItems, NULL, NULL);

		try
		{
			directory->Open();

			int	count = directory->GetFileCount();
			for (int i = 0; i < count; i++)
			{
				DirectoryItem* item = new DirectoryItem;
				item->index = i;
				directory->GetFile(i, item->name, item->isDirectory);
				fileList->Insert(item);
			}
		}
		catch (TSystemError* error)
		{
			TCommonDialogs::AlertDialog(strerror(error->GetError()), _("Error"), NULL);
			delete error;

			delete directory;
			fileList->DeleteAll();
			delete fileList;
			return false;
		}
	
		if (fDirectory)
		{
			fDirectory->Close();
			delete fDirectory;
			fDirectory = NULL;
		}
		fDirectory = directory;
		
		if (fFileList)
		{
			fFileList->DeleteAll();
			delete fFileList;
		}
		fFileList = fileList;
	}
		
	ScrollTo(0, 0);
	UnselectAll();
	ClearLastClick();

	if (IsVisible())
	{
		ComputeContentSize();
		Redraw();
	}

	HandleCommand(this, this, kValueChangedCommandID);
	HandleCommand(this, this, kFlushTypeSelectCommandID);
	
	return true;
}


TFile* TDirectoryListView::GetFile(int index)
{
	DirectoryItem* item = (*fFileList)[index];
	ASSERT(!item->isDirectory);
	return fDirectory->GetFile(item->index);
}


TDirectory* TDirectoryListView::GetDirectory(int index)
{
	DirectoryItem* item = (*fFileList)[index];
	ASSERT(item->isDirectory);
	return fDirectory->GetDirectory(item->index);
}


TCoord TDirectoryListView::GetRowHeight(int row) const
{
	return fRowHeight;
}


void TDirectoryListView::Create()
{
	TTextListView::Create();

	TDrawContext	context(this);
	TCoord ascent, height;
	context.MeasureText("W", 1, ascent, height);

	if (height < fFileIcon->GetHeight())
		height = fFileIcon->GetHeight();
	if (height < fFolderIcon->GetHeight())
		height = fFolderIcon->GetHeight();
		
	fRowHeight = height;

	ComputeContentSize();
}


void TDirectoryListView::GetText(int row, TString& outText)
{
	DirectoryItem* item = (*fFileList)[row];
	outText = item->name;
}


void TDirectoryListView::DrawCell(int row, int column, const TRect& cellBounds, TDrawContext& context)
{
	DirectoryItem* item = (*fFileList)[row];
	bool selected = CellSelected(row, column);

	TColor	savedForeColor(context.GetForeColor());
	TColor	savedBackColor(context.GetBackColor());

	if (selected)
	{
		if (context.GetDepth() < 8)
		{
			context.SetForeColor(fBackColor);
			context.SetBackColor(fForeColor);		
		}
		else
			context.SetBackColor(gApplication->GetHiliteColor());
	}

	TPixmap* pixmap = (item->isDirectory ? fFolderIcon : fFileIcon);

	TRect r(cellBounds.left, cellBounds.top, cellBounds.left + pixmap->GetWidth() + 4, cellBounds.top + pixmap->GetHeight());
	context.EraseRect(r);
	
	context.DrawPixmap(pixmap, TPoint(cellBounds.left + 2, cellBounds.top));

	TRect	textBounds(cellBounds);
	textBounds.left = r.right;

	if (context.GetDepth() >= 8)
	{
		if (fSelectFiles || item->isDirectory)
			context.SetForeColor(kBlackColor);
		else
			context.SetForeColor(kGrayColor);
	}
		
	context.DrawTextBox(item->name, item->name.GetLength(), textBounds, kTextAlignLeft);

	if (selected)
	{
		context.SetForeColor(savedForeColor);
		context.SetBackColor(savedBackColor);
	}
}


bool TDirectoryListView::DoKeyDown(KeySym key, TModifierState state, const char* string)
{
	switch (key)
	{
		case XK_Return:
		case XK_KP_Enter:
		{
			int row = GetFirstSelectedRow();
			
			if (row >= 0)
			{
				CellDoubleClicked(row, 0);
				return true;
			}

			break;
		}

		default:
			break;
	}	

	return TTextListView::DoKeyDown(key, state, string);
}


void TDirectoryListView::SelectAll()
{
	for (int row = 0; row < GetRowCount(); row++)
	{
		if (!CellSelected(row, 0))
		{
			DirectoryItem* item = (*fFileList)[row];

			if (!item->isDirectory)
				SelectCell(row, 0);
		}
	}
	
	DoCommand(this, this, kSelectionChangedCommandID);
}


int TDirectoryListView::GetRowCount() const
{
	return fDirectory->GetFileCount();
}


bool TDirectoryListView::AllowCellSelect(int row, int column, bool multiple)
{
	DirectoryItem* item = (*fFileList)[row];
	bool isDirectory = item->isDirectory;
	
	if (multiple)
	{
		// don't allow selecting if we already have a directory selected
		int testRow = GetFirstSelectedRow();
		if (testRow >= 0 && testRow != row && (*fFileList)[testRow]->isDirectory)
			return false;
	}
		
	if (fSelectFiles)
	{
		if (isDirectory && multiple)
		{
			// don't allow selecting directories if we have files selected
			for (int row = 0; row < GetRowCount(); row++)
			{
				if (CellSelected(row, 0) && !(*fFileList)[row]->isDirectory)
					return false;
			}
		}

		return true;
	}
	else
		return isDirectory;
}


void TDirectoryListView::CellDoubleClicked(int row, int column)
{
	DirectoryItem* item = (*fFileList)[row];

	if (item->isDirectory)
	{
			TDirectory* directory = fDirectory->GetDirectory(item->index);
			ASSERT(directory);
			if (directory)
				SetDirectory(directory);
	}
	else if (fSelectFiles)
	{
		HandleCommand(this, this, kOKCommandID);	// dismiss the dialog
	}
}


bool TDirectoryListView::DismissesDialog(TCommandID command) const
{
	return (command == kOKCommandID);
}


int TDirectoryListView::CompareItems(const DirectoryItem* item1, const DirectoryItem* item2, void* compareData)
{
	return strcasecmp(item1->name, item2->name);
}

