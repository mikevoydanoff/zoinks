// ========================================================================================
//	TCommonDialogs.cpp		 	Copyright (C) 2001-2002 Mike Lockwood. All rights reserved.
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

#include "TCommonDialogs.h"
#include "TApplication.h"
#include "TBehavior.h"
#include "TButton.h"
#include "TDialogWindow.h"
#include "TDirectory.h"
#include "TDirectoryListView.h"
#include "TDrawContext.h"
#include "TFile.h"
#include "TScroller.h"
#include "TStaticText.h"
#include "TTabTargetBehavior.h"
#include "TTextField.h"
#include "TTypeSelectBehavior.h"
#include "TWindowPositioners.h"

#include "intl.h"

#include <stdlib.h>
#include <ctype.h>
#include <limits.h>

static TString	sCurrentDirectory(".");


class TOpenSaveFilesDialogBehavior : public TBehavior
{
public:
	enum DialogType
	{
 		kOpenFiles,
 		kChooseDirectory,
 		kSaveFile
 	};
 	
public:
							TOpenSaveFilesDialogBehavior(TDirectoryListView* directoryListView, TTextView* fileNameTextView, 
														 TStaticText* pathTextView, DialogType dialogType);
	virtual					~TOpenSaveFilesDialogBehavior();
	
	virtual bool			DoCommand(TCommandHandler* sender, TCommandHandler* receiver, TCommandID command);
	virtual bool			DoKeyDown(KeySym key, TModifierState state, const char* string);

private:
	TDirectoryListView*		fDirectoryListView;
	TTextView*				fFileNameTextView;
	TStaticText*			fPathTextView;
	DialogType				fDialogType;
};


TOpenSaveFilesDialogBehavior::TOpenSaveFilesDialogBehavior(TDirectoryListView* directoryListView, TTextView* fileNameTextView, 
														   TStaticText* pathTextView, DialogType dialogType)
	:	fDirectoryListView(directoryListView),
		fFileNameTextView(fileNameTextView),
		fPathTextView(pathTextView),
		fDialogType(dialogType)
{
}


TOpenSaveFilesDialogBehavior::~TOpenSaveFilesDialogBehavior()
{
}


bool TOpenSaveFilesDialogBehavior::DoCommand(TCommandHandler* sender, TCommandHandler* receiver, TCommandID command)
{
	switch (command)
	{
		case kParentDirectoryCommandID:
		{
			TDirectory* directory = fDirectoryListView->GetDirectory();
			
			if (directory)
				fDirectoryListView->SetDirectory(directory->GetParent());
				
			return true;
		}

		case kHomeDirectoryCommandID:
		{
			TDirectory* directory = new TDirectory(getenv("HOME"));
			fDirectoryListView->SetDirectory(directory);
				
			return true;
		}

		case kWorkingDirectoryCommandID:
		{
			TDirectory* directory = new TDirectory(".");
			fDirectoryListView->SetDirectory(directory);
				
			return true;
		}
		
		case kSelectionChangedCommandID:
		{
			if (fDialogType == kOpenFiles)
 			{
				TDialogWindow* dialog = dynamic_cast<TDialogWindow*>(fOwner);
				ASSERT(dialog);
				dialog->SetOKEnabled(fDirectoryListView->GetFirstSelectedRow() >= 0);
			}
			break;
		}
		
		case kValueChangedCommandID:
		{
			if (fPathTextView && sender == fDirectoryListView)
				fPathTextView->SetText(fDirectoryListView->GetDirectory()->GetPath());
			break;
		}
		
		case kDataModifiedCommandID:
		{
			if (sender && sender == fFileNameTextView)
			{
				TDialogWindow* dialog = dynamic_cast<TDialogWindow*>(fOwner);
				ASSERT(dialog);
				dialog->SetOKEnabled(fFileNameTextView->GetTextLength() > 0);
			}
		}
	}
	
	return false;
}


bool TOpenSaveFilesDialogBehavior::DoKeyDown(KeySym key, TModifierState state, const char* string)
{
	// Mod-U for parent directory
	if (key < 256 && tolower(key) == 'u' && (state & Mod1Mask))
		return DoCommand(fOwner, fOwner, kParentDirectoryCommandID);
	if (key < 256 && tolower(key) == 'h' && (state & Mod1Mask))
		return DoCommand(fOwner, fOwner, kHomeDirectoryCommandID);
	if (key < 256 && tolower(key) == 'w' && (state & Mod1Mask))
		return DoCommand(fOwner, fOwner, kWorkingDirectoryCommandID);

	return false;
}


TFile* TCommonDialogs::OpenFile(TWindow* owner)
{
	TList<TFile>* fileList = NULL;
	return DoOpenFiles(owner, false, fileList);
}


TList<TFile>* TCommonDialogs::OpenFiles(TWindow* owner)
{
	TList<TFile>* fileList = NULL;
	DoOpenFiles(owner, true, fileList);
	return fileList;
}


TFile* TCommonDialogs::DoOpenFiles(TWindow* owner, bool allowMultiple, TList<TFile>*& fileList)
{
	fileList = NULL;
	TFile* result = NULL;
	
	TRect		bounds(0, 0, 300, 300);

	TDialogWindow* dialog = new TDialogWindow(bounds, _("Open File..."), true, owner);

	TRect	r(bounds);
	r.Inset(20, 30, 120, 20);

	TScroller* scroller = new TScroller(dialog, r, false, true);
	scroller->SetBorder(1);
	scroller->SetBackColor(kWhiteColor);
	scroller->SetWindowPositioner(SizeRelativeParent);

	TDirectoryListView* listView = new TDirectoryListView(scroller, r, sCurrentDirectory, true);
	listView->SetMultiSelect(allowMultiple);
	scroller->SetContainedView(listView);
	dialog->SetTarget(listView);

	TRect	r1(20, 10, bounds.right - 20, 30);
	TStaticText* pathText = new TStaticText(dialog, r1, listView->GetDirectory()->GetPath());
	pathText->SetWindowPositioner(WidthRelativeParent);

	r.Set(r.right + 20, r.bottom - 20, bounds.right - 20, r.bottom);

	TButton* button = new TButton(dialog, r, _("Cancel"), kCancelCommandID, true);
	button->SetWindowPositioner(BottomRightRelative);

	r.Offset(0, -30);

	button = new TButton(dialog, r, _("OK"), kOKCommandID, true);
	button->SetWindowPositioner(BottomRightRelative);
	dialog->SetOKButton(button);
	button->SetDefault();

	r.Offset(0, scroller->GetBounds().top - r.top);

	button = new TButton(dialog, r, _("Up"), kParentDirectoryCommandID, false);
	button->SetWindowPositioner(RightRelative);

	r.Offset(0, r.GetHeight() + 10);
	button = new TButton(dialog, r, _("Home"), kHomeDirectoryCommandID, false);
	button->SetWindowPositioner(RightRelative);

	r.Offset(0, r.GetHeight() + 10);
	button = new TButton(dialog, r, _("Working Dir."), kWorkingDirectoryCommandID, false);
	button->SetWindowPositioner(RightRelative);

	dialog->AddBehavior(new TOpenSaveFilesDialogBehavior(listView, NULL, pathText, TOpenSaveFilesDialogBehavior::kOpenFiles));
	listView->AddBehavior(new TTypeSelectBehavior(false));

	dialog->Show(true);
	dialog->SetOKEnabled(false);

continue_dialog:
	TCommandID command = gApplication->ModalDialog(dialog);

	if (command == kOKCommandID)
	{
		int row = listView->GetFirstSelectedRow();
		if (row >= 0 && listView->IsDirectory(row))
		{
			// switch to directory
			listView->CellDoubleClicked(row, 0);
			// continue the modal dialog
			goto continue_dialog;
		}
		
		if (allowMultiple)
			fileList = new TList<TFile>;

		for (int i = 0; i < listView->GetRowCount(); i++)
		{
			if (listView->CellSelected(i, 0))
			{
				TFile* file = listView->GetFile(i);
				
				if (file)
				{
					if (allowMultiple)
						fileList->Insert(file);
					else
					{
						result = file;
						break;
					}
				}
			}
		}

		if (allowMultiple && fileList->GetSize() == 0)
		{
			delete fileList;
			fileList = NULL;
		}

		sCurrentDirectory = listView->GetDirectory()->GetPath();
	}

	dialog->Close();
	return result;
}


TDirectory* TCommonDialogs::ChooseDirectory(TWindow* owner)
{
	TDirectory* result = NULL;
	
	TRect		bounds(0, 0, 300, 300);

	TDialogWindow* dialog = new TDialogWindow(bounds, _("Choose Directory..."), true, owner);

	TRect	r(bounds);
	r.Inset(20, 30, 120, 20);

	TScroller* scroller = new TScroller(dialog, r, false, true);
	scroller->SetBorder(1);
	scroller->SetBackColor(kWhiteColor);
	scroller->SetWindowPositioner(SizeRelativeParent);

	TDirectoryListView* listView = new TDirectoryListView(scroller, r, sCurrentDirectory, false);
	scroller->SetContainedView(listView);
	dialog->SetTarget(listView);

	TRect	r1(20, 10, bounds.right - 20, 30);
	TStaticText* pathText = new TStaticText(dialog, r1, listView->GetDirectory()->GetPath());
	pathText->SetWindowPositioner(WidthRelativeParent);

	r.Set(r.right + 20, r.bottom - 20, bounds.right - 20, r.bottom);

	TButton* button = new TButton(dialog, r, _("Cancel"), kCancelCommandID, true);
	button->SetWindowPositioner(BottomRightRelative);

	r.Offset(0, -30);

	button = new TButton(dialog, r, _("OK"), kOKCommandID, true);
	button->SetWindowPositioner(BottomRightRelative);
	button->SetDefault();

	r.Offset(0, scroller->GetBounds().top - r.top);

	button = new TButton(dialog, r, _("Up"), kParentDirectoryCommandID, false);
	button->SetWindowPositioner(RightRelative);

	r.Offset(0, r.GetHeight() + 10);
	button = new TButton(dialog, r, _("Home"), kHomeDirectoryCommandID, false);
	button->SetWindowPositioner(RightRelative);

	r.Offset(0, r.GetHeight() + 10);
	button = new TButton(dialog, r, _("Working Dir."), kWorkingDirectoryCommandID, false);
	button->SetWindowPositioner(RightRelative);

	dialog->AddBehavior(new TOpenSaveFilesDialogBehavior(listView, NULL, pathText, TOpenSaveFilesDialogBehavior::kChooseDirectory));
	listView->AddBehavior(new TTypeSelectBehavior(false));

	dialog->Show(true);

	TCommandID command = gApplication->ModalDialog(dialog);

	if (command == kOKCommandID)
	{
		sCurrentDirectory = listView->GetDirectory()->GetPath();
		int row = listView->GetFirstSelectedRow();

		if (row >= 0)
		{
			result = listView->GetDirectory(row);
			ASSERT(result);
		}

		if (! result)
			result = new TDirectory(*listView->GetDirectory());
	}

	dialog->Close();
	return result;
}
	

TFile* TCommonDialogs::SaveFile(const char* path, TWindow* owner)
{
	TFile*  result = NULL;
	
	TRect		bounds(0, 0, 300, 300);

	TDialogWindow* dialog = new TDialogWindow(bounds, _("Save File As..."), true, owner);
	dialog->AddBehavior(new TTabTargetBehavior);

	TRect	r(bounds);
	r.Inset(20, 30, 120, 20 + 60);

	TScroller* scroller = new TScroller(dialog, r, false, true);
	scroller->SetBorder(1);
	scroller->SetBackColor(kWhiteColor);
	scroller->SetWindowPositioner(SizeRelativeParent);

	TDirectoryListView* listView = new TDirectoryListView(scroller, r, sCurrentDirectory, false);
	scroller->SetContainedView(listView);

	TRect	r1(20, 10, bounds.right - 20, 30);
	TStaticText* pathText = new TStaticText(dialog, r1, listView->GetDirectory()->GetPath());
	pathText->SetWindowPositioner(WidthRelativeParent);

	TRect r2(bounds);
	r2.Inset(20, scroller->GetBounds().bottom + 20, 20, 0);
	r2.bottom = r2.top + 16;
	TStaticText* staticText = new TStaticText(dialog, r2, _("Save File As..."));
	staticText->SetWindowPositioner(BottomRelative);

	r2.Offset(0, r2.GetHeight() + 10);
	r2.bottom = r2.top + 20;
	TTextField* textView = new TTextField(dialog, r2, TDrawContext::GetDefaultFont(), true);
	dialog->SetTarget(textView);
	textView->SetWindowPositioner(BottomScrollBar);	// bottom relative, width relative parent
	textView->SetFilterTabAndCR(true);

	r.Set(r.right + 20, r.bottom - 20, bounds.right - 20, r.bottom);

	TButton* button = new TButton(dialog, r, _("Cancel"), kCancelCommandID, true);
	button->SetWindowPositioner(BottomRightRelative);

	r.Offset(0, -30);

	button = new TButton(dialog, r, _("OK"), kOKCommandID, true);
	button->SetWindowPositioner(BottomRightRelative);
	dialog->SetOKButton(button);
	button->SetDefault();

	r.Offset(0, scroller->GetBounds().top - r.top);

	button = new TButton(dialog, r, _("Up"), kParentDirectoryCommandID, false);
	button->SetWindowPositioner(RightRelative);

	r.Offset(0, r.GetHeight() + 10);
	button = new TButton(dialog, r, _("Home"), kHomeDirectoryCommandID, false);
	button->SetWindowPositioner(RightRelative);

	r.Offset(0, r.GetHeight() + 10);
	button = new TButton(dialog, r, _("Working Dir."), kWorkingDirectoryCommandID, false);
	button->SetWindowPositioner(RightRelative);

	dialog->AddBehavior(new TOpenSaveFilesDialogBehavior(listView, textView, pathText, TOpenSaveFilesDialogBehavior::kSaveFile));
	listView->AddBehavior(new TTypeSelectBehavior(false));

	dialog->Show(true);
	dialog->SetOKEnabled(false);

tryAgain:
	TCommandID command = gApplication->ModalDialog(dialog);

	if (command == kOKCommandID)
	{
		TString	fileName;
		textView->GetText(fileName);

		if (fileName.GetLength() == 0)
		{
			gApplication->Beep();
			goto tryAgain;
		}

		result = new TFile(*listView->GetDirectory(), fileName);
		
		if (result->Exists())
		{
			if (result->IsDirectory())
			{
				delete result;
				result = NULL;
				AlertDialog(_("Can not overwrite a directory with a file"), _("Error"), dialog);
				goto tryAgain;
			}
			else
			{
				char	prompt[PATH_MAX + 50];
				
				sprintf(prompt, _("Replace existing file %s?"), (const char *)fileName);
				if (!ConfirmDialog(prompt, _("Replace File?"), dialog))
				{
					delete result;
					result = NULL;
					goto tryAgain;
				}
			}
		}
		
		sCurrentDirectory = listView->GetDirectory()->GetPath();
	}

	dialog->Close();
	return result;
}


void TCommonDialogs::AlertDialog(const TChar* prompt, const TChar* title, TWindow* owner, const TChar* okString)
{
	TRect	dialogBounds(0, 0, 300, 120);
	TDialogWindow* dialog = new TDialogWindow(dialogBounds, title, true, owner);

	/*TStaticText* staticText =*/ new TStaticText(dialog, TRect(20, 20, 280, 100), prompt);

	TCoord buttonWidth = 80;
	TRect buttonBounds;
	buttonBounds.left = (dialogBounds.GetWidth() - buttonWidth) / 2;
	buttonBounds.right = buttonBounds.left + buttonWidth;
	buttonBounds.bottom = dialogBounds.bottom - 20;
	buttonBounds.top = buttonBounds.bottom - 20;

	TButton* button = new TButton(dialog, buttonBounds, okString, kOKCommandID, true);
	button->SetWindowPositioner(BottomRelativeCenterHorizontal);
	button->SetDefault();

	dialog->Show(true);

	gApplication->ModalDialog(dialog);
	dialog->Close();
}


bool TCommonDialogs::ConfirmDialog(const TChar* prompt, const TChar* title, TWindow* owner,
								   const TChar* okString, const TChar* cancelString)
{
	TRect	dialogBounds(0, 0, 300, 120);
	TDialogWindow* dialog = new TDialogWindow(dialogBounds, title, true, owner);

	/*TStaticText* staticText =*/ new TStaticText(dialog, TRect(20, 20, 280, 100), prompt);
	
	TButton* button = new TButton(dialog, TRect(140, 80, 200, 100), cancelString, kCancelCommandID, true);
	button->SetWindowPositioner(BottomRightRelative);

	button = new TButton(dialog, TRect(220, 80, 280, 100), okString, kOKCommandID, true);
	button->SetWindowPositioner(BottomRightRelative);
	button->SetDefault();
	
	dialog->Show(true);

	TCommandID result = gApplication->ModalDialog(dialog);
	dialog->Close();
			
	return (result == kOKCommandID);
}


bool TCommonDialogs::YesNoDialog(const TChar* prompt, const TChar* title, TWindow* owner, bool& result, const TChar* yesString, const TChar* noString, const TChar* cancelString)
{
	result = false;

	TRect	dialogBounds(0, 0, 300, 120);
	TDialogWindow* dialog = new TDialogWindow(dialogBounds, title, true, owner);

	(void) new TStaticText(dialog, TRect(20, 20, 280, 100), prompt);
	
	TButton* button = new TButton(dialog, TRect(20, 80, 120, 100), noString, kNoCommandID, true);
	button->SetWindowPositioner(BottomRightRelative);

	button = new TButton(dialog, TRect(140, 80, 200, 100), cancelString, kCancelCommandID, true);
	button->SetWindowPositioner(BottomRightRelative);

	button = new TButton(dialog, TRect(220, 80, 280, 100), yesString, kOKCommandID, true);
	button->SetWindowPositioner(BottomRightRelative);
	button->SetDefault();

	dialog->Show(true);

	TCommandID dismisser = gApplication->ModalDialog(dialog);
	dialog->Close();
	
	if (dismisser == kCancelCommandID)
		return false;
	else if (dismisser == kOKCommandID)
		result = true;
		
	return true;
}


bool TCommonDialogs::TextEntryDialog(const TChar* prompt, const TChar* title, TWindow* owner, TString& text, TBehavior* textFilter,
										const TChar* okString, const TChar* cancelString)
{
	TRect	dialogBounds(0, 0, 300, 120);
	TDialogWindow* dialog = new TDialogWindow(dialogBounds, title, true, owner);

	(void) new TStaticText(dialog, TRect(20, 20, 280, 100), prompt);
	
	TTextField* textView = new TTextField(dialog, TRect(20, 40, 280, 60), TDrawContext::GetDefaultFont(), true);
	dialog->SetTarget(textView);
	textView->SetFilterTabAndCR(true);
	textView->SetWindowPositioner(WidthRelativeParent);
	if (textFilter)
		textView->AddBehavior(textFilter);

	TButton* button = new TButton(dialog, TRect(140, 80, 200, 100), cancelString, kCancelCommandID, true);
	button->SetWindowPositioner(BottomRightRelative);

	button = new TButton(dialog, TRect(220, 80, 280, 100), okString, kOKCommandID, true);
	button->SetWindowPositioner(BottomRightRelative);
	button->SetDefault();

	dialog->Show(true);

	// need to do this after window is constructed
	textView->InsertText(0, text, text.GetLength());
	textView->SelectAll();

	TCommandID result = gApplication->ModalDialog(dialog);

	if (result == kOKCommandID)
		textView->GetText(text);

	dialog->Close();
			
	return (result == kOKCommandID);
}


TDialogWindow* TCommonDialogs::CancelDialog(const TChar* prompt, const TChar* title, TWindow* owner, const TChar* cancelString)
{
	TRect	dialogBounds(0, 0, 300, 120);
	TDialogWindow* dialog = new TDialogWindow(dialogBounds, title, true, owner);

	(void) new TStaticText(dialog, TRect(20, 20, 280, 100), prompt);

	TCoord buttonWidth = 80;
	TRect buttonBounds;
	buttonBounds.left = (dialogBounds.GetWidth() - buttonWidth) / 2;
	buttonBounds.right = buttonBounds.left + buttonWidth;
	buttonBounds.bottom = dialogBounds.bottom - 20;
	buttonBounds.top = buttonBounds.bottom - 20;

	TButton* button = new TButton(dialog, buttonBounds, cancelString, kCancelCommandID, true);
	button->SetWindowPositioner(BottomRelativeCenterHorizontal);
		
	dialog->Show(true);

	return dialog;
}


bool TCommonDialogs::SaveChanges(const TChar* fileName, TWindow* owner, bool& save)
{
	char	prompt[PATH_MAX + 100];
	sprintf(prompt, _("Do you want to save your changes to file \"%s\" before closing?"), fileName);
	
	return YesNoDialog(prompt, _("Save Changes?"), owner, save, _("Save"), _("Don't Save"));
}


