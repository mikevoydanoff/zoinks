// ========================================================================================
//	TDiffDialogs.cpp		 		Copyright (C) 2003 Mike Voydanoff. All rights reserved.
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

#include "IDECommon.h"

#include "TDiffDialogs.h"
#include "TProjectCommands.h"
#include "TTextDocument.h"

#include "fw/TApplication.h"
#include "fw/TButton.h"
#include "fw/TCommonDialogs.h"
#include "fw/TDialogWindow.h"
#include "fw/TDirectory.h"
#include "fw/TDocumentWindow.h"
#include "fw/TFile.h"
#include "fw/TPopupButton.h"
#include "fw/TPopupMenu.h"
#include "fw/TRadio.h"
#include "fw/TStaticText.h"
#include "fw/TTabTargetBehavior.h"
#include "fw/TTextField.h"
#include "fw/TWindowPositioners.h"

#include "fw/intl.h"


bool TDiffDialogs::CompareFilesDialog(TString& path1, TString& path2, bool& compareDirectories)
{
	TRect	bounds(0, 0, 350 + 90, 160);
	TDialogWindow* window = new TDialogWindow(bounds,  _("Compare Files"), true, NULL);
	
	TRect	r(bounds.left + 10, bounds.top + 10, bounds.right - 90 - 90, bounds.top + 30);
	TStaticText* staticText = new TStaticText(window, r, _("File or Directory 1:"));

	r.Offset(0, 20);
	TTextField* file1TextField = new TTextField(window, r, TDrawContext::GetDefaultFont(), true);
	file1TextField->SetFilterTabAndCR(true);
	file1TextField->SetWindowPositioner(WidthRelativeParent);
	
	TRect r2(r.right + 10 + 90, r.top, r.right + 10 + 60 + 90, r.top + 20);
	TButton* button = new TButton(window, r2, _("Choose"), kChoosePath1CommandID);
	button->SetWindowPositioner(RightRelative);

	r.Offset(0, 30);
	staticText = new TStaticText(window, r, _("File or Directory 2:"));

	r.Offset(0, 20);
	TTextField* file2TextField = new TTextField(window, r, TDrawContext::GetDefaultFont(), true);
	file2TextField->SetFilterTabAndCR(true);
	file2TextField->SetWindowPositioner(WidthRelativeParent);

	TRect r3(r.right + 10 + 90, r.top, r.right + 10 + 60 + 90, r.top + 20);
	button = new TButton(window, r3, _("Choose"), kChoosePath2CommandID);
	button->SetWindowPositioner(RightRelative);

	button = new TButton(window, TRect(bounds.right - 160, bounds.bottom - 40, bounds.right - 100, bounds.bottom - 20), _("Cancel"), kCancelCommandID, true);
	button->SetWindowPositioner(BottomRightRelative);

	button = new TButton(window, TRect(bounds.right - 80, bounds.bottom - 40, bounds.right - 20, bounds.bottom - 20), _("OK"), kOKCommandID, true);
	button->SetWindowPositioner(BottomRightRelative);
	button->SetDefault();

	TRect r4(r.right + 20, bounds.top + 30, r.right + 20 + 60, bounds.top + 50);
	TTextDocumentsPopupMenu* popupMenu1 = new TTextDocumentsPopupMenu;
	TPopupButton* popupButton = new TPopupButton(window, r4, _("Windows"), popupMenu1);
	popupButton->SetWindowPositioner(RightRelative);
	
	r4.Offset(0, 50);
	TTextDocumentsPopupMenu* popupMenu2 = new TTextDocumentsPopupMenu;
	popupButton = new TPopupButton(window, r4, _("Windows"), popupMenu2);
	popupButton->SetWindowPositioner(RightRelative);
	
	r.Offset(0, 30);
	r.bottom = r.top + 20;
	r.right = bounds.right - 160;	// left edge of cancel button
	
	TRadio* radio1 = new TRadio( window, r, _("Compare Files"), kDoCompareFilesCommandID);
	r.Offset(0, 20);
	TRadio* radio2 = new TRadio( window, r, _("Compare Directories"), kDoCompareDirectoriesCommandID);

	TDiffDialogBehavior* behavior = new TDiffDialogBehavior(file1TextField, file2TextField, popupMenu1, popupMenu2, radio1, radio2, compareDirectories);
	window->AddBehavior(behavior);
	window->AddBehavior(new TTabTargetBehavior);
	window->SetTarget(file1TextField);

	file1TextField->SetText(path1);
	file2TextField->SetText(path2);

	window->Show(true);

	TCommandID result = gApplication->ModalDialog(window);
	
	file1TextField->GetText(path1);
	file2TextField->GetText(path2);

	compareDirectories = behavior->CompaareDirectories();
	
	// tweak - if both paths have trailing '/' then do a directory compare
	bool trailingSlash1 = (path1.GetLength() > 0 && path1[path1.GetLength() - 1] == '/');
	bool trailingSlash2 = (path2.GetLength() > 0 && path2[path2.GetLength() - 1] == '/');
	if (trailingSlash1 && trailingSlash2)
		compareDirectories = true;

	if (compareDirectories)
	{
		// make sure we have a trailing '/'
		if (!trailingSlash1)
			path1 += "/";
		if (!trailingSlash2)
			path2 += "/";	
	}

	window->Close();
			
	return (result == kOKCommandID);
}


TTextDocumentsPopupMenu::TTextDocumentsPopupMenu()
{
}


TTextDocumentsPopupMenu::~TTextDocumentsPopupMenu()
{
}


void TTextDocumentsPopupMenu::PreDisplayMenu()
{
	int32 size = fItemList.GetSize();
	
	while (size > 0)
	{
		fItemList.DeleteAt(size - 1);
		size--;
	}
	
	TMenu::PreDisplayMenu();

	TListIterator<TTopLevelWindow> iter(TTopLevelWindow::GetWindowList());
	TTopLevelWindow* window;

	while ((window = iter.Next()) != NULL)
	{
		TDocumentWindow* documentWindow = dynamic_cast<TDocumentWindow *>(window);
		if (window)
		{
			TTextDocument* document = dynamic_cast<TTextDocument *>(documentWindow->GetDocument());
			if (document)
			{	
				const TFile& file = document->GetFile();
				
				if (file.IsSpecified())
				{
					TMenuItem* item = new TMenuItem(file.GetPath());
					item->Enable(true);
					AddItem(item);
				}
			}
		}
	}
	
	if (fItemList.GetSize() == 0)
	{
		TMenuItem* item = new TMenuItem(_("No Windows"));
		AddItem(item);
	}
}


TDiffDialogBehavior::TDiffDialogBehavior(TTextField* file1TextField, TTextField* file2TextField, 
										TTextDocumentsPopupMenu* file1PopupMenu, TTextDocumentsPopupMenu* file2PopupMenu,
										TRadio*	compareFilesRadio, TRadio* compareDirectoriesRadio,
										bool compareDirectories)
	:	fFile1TextField(file1TextField),
		fFile2TextField(file2TextField),
		fFile1PopupMenu(file1PopupMenu),
		fFile2PopupMenu(file2PopupMenu),
		fCompareFilesRadio(compareFilesRadio),
		fCompareDirectoriesRadio(compareDirectoriesRadio),
		fCompareDirectories(compareDirectories)
{
		compareFilesRadio->SetOn(!compareDirectories);
		compareDirectoriesRadio->SetOn(compareDirectories);
}


TDiffDialogBehavior::~TDiffDialogBehavior()
{
}


bool TDiffDialogBehavior::DoCommand(TCommandHandler* sender, TCommandHandler* receiver, TCommandID command)
{
	switch (command)
	{
		case kChoosePath1CommandID:
		case kChoosePath2CommandID:
		{
			TTextField* textField = (command == kChoosePath1CommandID ? fFile1TextField : fFile2TextField);
			
			if (fCompareDirectories)
			{
				TDirectory* directory = TCommonDialogs::ChooseDirectory(NULL);
				if (directory)
				{
					textField->SetText(directory->GetPath());		
					delete directory;
				}
			}
			else
			{
				TFile* file = TCommonDialogs::OpenFile(NULL);
				if (file)
				{
					textField->SetText(file->GetPath());
					delete file;
				}
			}
				
			return true;
		}
		
		case kSelectionChangedCommandID:
		{
			TTextDocumentsPopupMenu*	popupMenu = NULL;
			TTextField*					textField = NULL;
			
			if (sender == fFile1PopupMenu)
			{
				popupMenu = fFile1PopupMenu;
				textField = fFile1TextField;
			}
			else if (sender == fFile2PopupMenu)
			{
				popupMenu = fFile2PopupMenu;
				textField = fFile2TextField;
			}
			
			if (textField)
			{
				TMenuItem* menuItem = popupMenu->GetSelectedItem();
				
				if (menuItem)
					textField->SetText(menuItem->GetTitle());
			}

			return false;
		}

		case kDoCompareFilesCommandID:
			fCompareDirectories = false;		
			fCompareDirectoriesRadio->SetOn(false);
			return true;

		case kDoCompareDirectoriesCommandID:
			fCompareDirectories = true;
			fCompareFilesRadio->SetOn(false);
			return true;
		
		default:
			return false;
	}
}
