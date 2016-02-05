// ========================================================================================
//	TTextFindBehavior.cpp	   Copyright (C) 2001-2002 Mike Voydanoff. All rights reserved.
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

#include "TTextFindBehavior.h"
#include "TTextField.h"
#include "TApplication.h"
#include "TMenu.h"
#include "TDialogWindow.h"
#include "TStaticText.h"
#include "TButton.h"
#include "TCheckBox.h"
#include "TWindowPositioners.h"
#include "TDrawContext.h"
#include "TTabTargetBehavior.h"

#include "intl.h"

#include <X11/keysym.h>


TString					TTextFindBehavior::sSearchString;
TString					TTextFindBehavior::sReplaceString;
bool					TTextFindBehavior::sCaseSensitive = false;
bool					TTextFindBehavior::sForward = true;
bool					TTextFindBehavior::sWrap = true;
bool					TTextFindBehavior::sWholeWord = false;

TDialogWindow*			TTextFindBehavior::sDialog = NULL;
TTextField*				TTextFindBehavior::sSearchTextView = NULL;

TFindDialogBehavior*	TTextFindBehavior::sDialogBehavior = NULL;

const char kCaseSensitiveSetting[]	= "TextSearchCaseSensitive";
const char kForwardSetting[]		= "TextSearchForward";
const char kWrapSetting[]			= "TextSearchWrap";
const char kWholeWordSetting[]		= "TextSearchWholeWord";


TTextFindBehavior::TTextFindBehavior()
{
}


TTextFindBehavior::~TTextFindBehavior()
{
	if (sDialogBehavior && sDialogBehavior->GetCurrentBehavior() == this)
		sDialogBehavior->SetCurrentBehavior(NULL);
}


void TTextFindBehavior::DoSetupMenu(TMenu* menu)
{
	TTextView* view = GetView();

	if (view)
	{
		menu->EnableCommand(kFindCommandID);

		if (sSearchString.GetLength() > 0)
		{
			menu->EnableCommand(kFindPreviousCommandID);
			menu->EnableCommand(kFindNextCommandID);
		}

		TString	selection;
		view->GetSelectedText(selection);

		if (selection.GetLength() > 0)
		{
			menu->EnableCommand(kFindPreviousSelectionCommandID);
			menu->EnableCommand(kFindNextSelectionCommandID);

			if (Tstrcmp(selection, sSearchString) == 0 ||
				 (!sCaseSensitive && Tstrcasecmp(selection, sSearchString) == 0))
			{
				menu->EnableCommand(kReplacePreviousCommandID);
				menu->EnableCommand(kReplaceNextCommandID);
			} 
		}
	}
}


bool TTextFindBehavior::DoCommand(TCommandHandler* sender, TCommandHandler* receiver, TCommandID command)
{
	switch (command)
	{
		case kFindCommandID:
			OpenDialog();
			return true;

		case kFindPreviousCommandID:
		case kFindNextCommandID:
		{
			bool forward = (command == kFindNextCommandID);
			if (!sForward)
				forward = !forward;
			FindNext(forward);
			return true;
		}

		case kFindPreviousSelectionCommandID:
		case kFindNextSelectionCommandID:
		{
			bool forward = (command == kFindNextSelectionCommandID);
			if (!sForward)
				forward = !forward;
			FindSelection(forward);
			return true;
		}

		case kReplacePreviousCommandID:
		case kReplaceNextCommandID:
		{
			bool forward = (command == kReplaceNextCommandID);
			if (!sForward)
				forward = !forward;
			ReplaceSelectionAndFind(forward);
			return true;
		}
	}

	return false;
}



void TTextFindBehavior::ReadFromSettingsFile(TSettingsFile* settingsFile)
{
	sCaseSensitive = settingsFile->GetBoolSetting(kCaseSensitiveSetting, sCaseSensitive);
	sForward = settingsFile->GetBoolSetting(kForwardSetting, sForward);
	sWrap = settingsFile->GetBoolSetting(kWrapSetting, sWrap);
	sWholeWord = settingsFile->GetBoolSetting(kWholeWordSetting, sWholeWord);
}


void TTextFindBehavior::WriteToSettingsFile(TSettingsFile* settingsFile)
{
	settingsFile->SetBoolSetting(kCaseSensitiveSetting, sCaseSensitive);
	settingsFile->SetBoolSetting(kForwardSetting, sForward);
	settingsFile->SetBoolSetting(kWrapSetting, sWrap);
	settingsFile->SetBoolSetting(kWholeWordSetting, sWholeWord);
}


void TTextFindBehavior::Search(const TChar* searchString)
{
	sSearchString = searchString;
	FindNext(sForward);
}


void TTextFindBehavior::Replace(const TChar* searchString, const TChar* replaceString)
{
	sSearchString = searchString;
	sReplaceString = replaceString;

	TTextView* view = GetView();
	TString	selection;
	view->GetSelectedText(selection);

	if (! (Tstrcmp(selection, searchString) == 0 ||
			(!sCaseSensitive && Tstrcasecmp(selection, searchString) == 0)))
	{
		if (!FindNext(sForward))
			return;	// find unsuccessful
	}

	view->ReplaceSelection(sReplaceString, sReplaceString.GetLength(), true, false);
}


void TTextFindBehavior::ReplaceAll(const TChar* searchString, const TChar* replaceString)
{
	sSearchString = searchString;
	sReplaceString = replaceString;

	bool			replacedOne = false;

	if (sSearchString.GetLength() > 0)
	{
		STextOffset		start, end, beginStart, beginEnd, lastStart;
		bool			didWrap = false;
		int				replaceDelta = sReplaceString.GetLength() - sSearchString.GetLength();

		TTextView* view = GetView();
		view->GetSelection(beginStart, beginEnd);
		lastStart = beginStart;
		
		while (view->FindString(searchString, sCaseSensitive, true, true, sWholeWord))
		{
			view->GetSelection(start, end);
			
			if (!replacedOne)
			{
				beginStart = start;
				replacedOne = true;
			}
		
			if (!didWrap && start < lastStart)
				didWrap = true;
			else
				lastStart = start;

			if (start >= beginStart && didWrap)
				break;

			view->ReplaceSelection(sReplaceString, sReplaceString.GetLength(), true, true);	
			
			if (didWrap)
				beginStart += replaceDelta;	// adjust beginning of replace operation by the replace delta
		}
	}

	if (!replacedOne)
		gApplication->Beep();
}


void TTextFindBehavior::OpenDialog()
{
	if (! sDialog)
	{
		TRect	bounds(0, 0, 400, 180);

		sDialog = new TDialogWindow(bounds, _("Find"), false, NULL);
		sDialog->SetHideOnClose(true);
		
		bounds.Inset(20, 20, 20, 20);

		TRect r(bounds);
		r.right = r.left + 100;
		r.bottom = r.top + 16;
		new TStaticText(sDialog, r, _("Find:"));

		TRect r2(bounds.right - 80, bounds.top, bounds.right, bounds.top + 20);
		TButton* button = new TButton(sDialog, r2, _("Find"), kFindCommandID, false);
		button->SetWindowPositioner(RightRelative);
		button->SetDefault();

		r2.top = r2.bottom + 10;
		r2.bottom = r2.top + 20;
		button = new TButton(sDialog, r2, _("Replace"), kReplaceCommandID, false);
		button->SetWindowPositioner(RightRelative);

		r2.top = r2.bottom + 10;
		r2.bottom = r2.top + 20;
		button = new TButton(sDialog, r2, _("Replace All"), kReplaceAllCommandID, false);
		button->SetWindowPositioner(RightRelative);

		bounds.top += r.GetHeight() + 10;
		bounds.right -= r2.GetWidth() + 10;
		bounds.bottom = bounds.top + 20;
	
		TTextField* searchTextView = new TTextField(sDialog, bounds, TDrawContext::GetDefaultFont(), true);
		sDialog->SetTarget(searchTextView);
		searchTextView->SetWindowPositioner(WidthRelativeParent);
		searchTextView->SetFilterTabAndCR(true);
		sSearchTextView = searchTextView;

		bounds.top = bounds.bottom + 10;
		bounds.bottom = bounds.top + 16;
		new TStaticText(sDialog, bounds, _("Replace:"));

		bounds.top = bounds.bottom + 10;
		bounds.bottom = bounds.top + 20;

		TTextField* replaceTextView = new TTextField(sDialog, bounds, TDrawContext::GetDefaultFont(), true);
		replaceTextView->SetWindowPositioner(WidthRelativeParent);
		replaceTextView->SetFilterTabAndCR(true);
		
		TCoord checkWidth = bounds.GetWidth()/2;
		bounds.top = bounds.bottom + 10;
		bounds.bottom = bounds.top + 20;
		bounds.right = bounds.left + checkWidth;
		TCheckBox* caseSensitiveBox = new TCheckBox(sDialog, bounds, _("Case Sensitive"));
		caseSensitiveBox->SetChecked(GetCaseSensitive());
		
		TRect bounds2(bounds);			
		bounds2.top = bounds2.bottom;
		bounds2.bottom = bounds2.top + 20;
		TCheckBox* backwardsBox = new TCheckBox(sDialog, bounds2, _("Search Backwards"));
		backwardsBox->SetChecked(!GetForward());
		
		bounds.left = bounds.right + 10;
		bounds.right += checkWidth;
		TCheckBox* wrapBox = new TCheckBox(sDialog, bounds, _("Wrap"));
		wrapBox->SetChecked(GetWrap());

		bounds2.left = bounds2.right + 10;
		bounds2.right += checkWidth;
		TCheckBox* wholeWordBox = new TCheckBox(sDialog, bounds2, _("Whole Word"));
		wholeWordBox->SetChecked(sWholeWord);

		ASSERT(!sDialogBehavior);
		sDialogBehavior = new TFindDialogBehavior(searchTextView, replaceTextView, caseSensitiveBox, backwardsBox, wrapBox, wholeWordBox);
		sDialog->AddBehavior(sDialogBehavior);

		sDialog->AddBehavior(new TTabTargetBehavior);
	}

	sDialog->Show(true);
	sDialog->Raise();
	sSearchTextView->SetTarget();
	sSearchTextView->SelectAll();
	sDialogBehavior->SetCurrentBehavior(this);
}


bool TTextFindBehavior::FindNext(bool forward)
{
	if (sSearchString.GetLength() > 0)
	{
		TTextView* view = GetView();
		if (view->FindString(sSearchString, sCaseSensitive, forward, sWrap, sWholeWord))
		{
//			TTopLevelWindow* window = view->GetTopLevelWindow();
//			if (window)
//				window->Raise();
	
			return true;
		}
		else
			gApplication->Beep();
	}

	return false;
}


void TTextFindBehavior::FindSelection(bool forward)
{
	TTextView* view = GetView();
	STextOffset start, end;
	view->GetSelection(start, end);
	
	if (start < end)
	{
		view->GetSelectedText(sSearchString);
		if (!view->FindString(sSearchString, sCaseSensitive, forward, sWrap, sWholeWord))
			gApplication->Beep();
	}
}


void TTextFindBehavior::ReplaceSelectionAndFind(bool forward)
{
	TTextView* view = GetView();
	STextOffset start, end;
	view->GetSelection(start, end);
	
	if (start < end)
	{
		view->ReplaceSelection(sReplaceString, sReplaceString.GetLength(), true, true);
		if (!view->FindString(sSearchString, sCaseSensitive, forward, sWrap, sWholeWord))
			gApplication->Beep();
	}
}


TTextView* TTextFindBehavior::GetView()
{
	TTextView* view = dynamic_cast<TTextView*>(fOwner);
	ASSERT(view);
	return view;
}


TFindDialogBehavior::TFindDialogBehavior(TTextField* searchTextView, TTextField* replaceTextView, 
										TCheckBox* caseSensitiveBox, TCheckBox* backwardsBox, 
										TCheckBox* wrapBox, TCheckBox* wholeWordBox)
	:	fSearchTextView(searchTextView),
		fReplaceTextView(replaceTextView),
		fCaseSensitiveBox(caseSensitiveBox),
		fBackwardsBox(backwardsBox),
		fWrapBox(wrapBox),
		fWholeWordBox(wholeWordBox),
		fTextFindBehavior(NULL)
{
	ASSERT(searchTextView);
}


TFindDialogBehavior::~TFindDialogBehavior()
{
	ASSERT(TTextFindBehavior::sDialogBehavior == this);
	TTextFindBehavior::sDialogBehavior = NULL;
}


bool TFindDialogBehavior::DoCommand(TCommandHandler* sender, TCommandHandler* receiver, TCommandID command)
{
	if (fTextFindBehavior)
	{
		switch (command)
		{
			case kFindCommandID:
			{
				TString		searchString;

				fSearchTextView->GetText(searchString);
				fTextFindBehavior->Search(searchString);

				return true;
			}

			case kReplaceCommandID:
			{
				TString		searchString, replaceString;

				fSearchTextView->GetText(searchString);
				fReplaceTextView->GetText(replaceString);
				fTextFindBehavior->Replace(searchString, replaceString);

				return true;
			}

			case kReplaceAllCommandID:
			{
				TString		searchString, replaceString;

				fSearchTextView->GetText(searchString);
				fReplaceTextView->GetText(replaceString);
				fTextFindBehavior->ReplaceAll(searchString, replaceString);
				return true;
			}

			case kFocusAcquiredCommandID:
				fSearchTextView->SetText(const_cast<TChar*>((const TChar*)fTextFindBehavior->sSearchString));
				if (fSearchTextView == fSearchTextView->GetTopLevelWindow()->GetTarget())
				{
					if (!fSearchTextView->IsPastingText())
						fSearchTextView->SelectAll();
				}
				else
					fSearchTextView->SetSelection(fSearchTextView->GetTextLength());

				fReplaceTextView->SetText(const_cast<TChar*>((const TChar*)fTextFindBehavior->sReplaceString));
				if (fReplaceTextView == fReplaceTextView->GetTopLevelWindow()->GetTarget())
				{
					if (!fReplaceTextView->IsPastingText())
						fReplaceTextView->SelectAll();
				}
				else
					fReplaceTextView->SetSelection(fReplaceTextView->GetTextLength());

				return false;

			case kFocusLostCommandID:
				// update search/replace strings
				fSearchTextView->GetText(fTextFindBehavior->sSearchString);
				fReplaceTextView->GetText(fTextFindBehavior->sReplaceString);
				return false;
				
			case kValueChangedCommandID:
				if (sender == fCaseSensitiveBox)
				{
					fTextFindBehavior->SetCaseSensitive(fCaseSensitiveBox->IsChecked());
					return true;
				}
				else if (sender == fBackwardsBox)
				{
					fTextFindBehavior->SetForward(!fBackwardsBox->IsChecked());
					return true;
				}
				else if (sender == fWrapBox)
				{
					fTextFindBehavior->SetWrap(fWrapBox->IsChecked());
					return true;
				}
				else if (sender == fWholeWordBox)
				{
					fTextFindBehavior->SetWholeWord(fWholeWordBox->IsChecked());
					return true;			
				}
				else
					return false;
		}
	}
	
	return false;
}


bool TFindDialogBehavior::DoKeyDown(KeySym key, TModifierState state, const char* string)
{
	if (fTextFindBehavior && (key == XK_Return || key == XK_KP_Enter))
		return DoCommand(fOwner, fOwner, kFindCommandID);
	else
		return false;
}


