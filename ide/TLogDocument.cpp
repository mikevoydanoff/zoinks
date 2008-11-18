// ========================================================================================
//	TLogDocument.cpp		 	Copyright (C) 2001-2003 Mike Lockwood. All rights reserved.
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

#include "TLogDocument.h"
#include "TLogDocumentOwner.h"
#include "TLogViewBehavior.h"
#include "TLineNumberBehavior.h"
#include "TProjectCommands.h"
#include "TSyntaxTextView.h"
#include "fw/TTextView.h"
#include "fw/TStatusBar.h"
#include "fw/TDocumentWindow.h"
#include "fw/TWindowPositioners.h"
#include "fw/TTextFindBehavior.h"
#include "fw/TApplication.h"
#include "fw/TMenuBar.h"
#include "fw/TWindowsMenu.h"
#include "fw/TScroller.h"
#include "fw/TFont.h"
#include "fw/TCommandID.h"

#include "fw/intl.h"

#include <X11/keysym.h>
#include <stdlib.h>

static TMenuItemRec sFileFormatMenu[] = 
{
	{ N_("Unix"), kUnixFormatCommandID },
	{ N_("Mac"), kMacFormatCommandID },
	{ N_("DOS"), kDOSFormatCommandID },
	{ "" }
};

static TMenuItemRec sFileMenu[] = 
{
	{ N_("New"), kNewCommandID, Mod1Mask, 'n' },
	{ N_("New Project"), kNewProjectCommandID, Mod1Mask|ShiftMask, 'n' },
	{ "-" },
	{ N_("Open..."), kOpenCommandID, Mod1Mask, 'o' },
	{ N_("Find File..."), kFindFileCommandID, Mod1Mask, 'd' },
	{ N_("Find Selection"), kFindSelectionCommandID, Mod1Mask|ShiftMask, 'd' },
	{ "-" },
	{ N_("Save"), kSaveCommandID, Mod1Mask, 's' },
	{ N_("Save As..."), kSaveAsCommandID },
	{ N_("Save Copy..."), kSaveCopyCommandID },
	{ N_("Save All"), kSaveAllCommandID,  Mod1Mask|ShiftMask, 's' },
	{ "-" },
	{ N_("File Format"), 0, 0, 0, sFileFormatMenu },
	{ N_("Wrap Lines"), kToggleLineWrapCommandID },	
	{ "-" },
	{ N_("Compare Files..."), kCompareFilesCommandID },
	{ "-" },
	{ N_("Close"), kCloseCommandID, Mod1Mask, 'w' },
	{ N_("Quit"), kQuitCommandID, Mod1Mask, 'q' },
	{ "" }
};

static TMenuItemRec sEditMenu[] = 
{
	{ N_("Copy"), kCopyCommandID, Mod1Mask, 'c' },
	{ N_("Select All"), kSelectAllCommandID, Mod1Mask, 'a' },
	{ "" }
};

static TMenuItemRec sSearchMenu[] =
{
	{ N_("Find..."), kFindCommandID, Mod1Mask, 'f' },
	{ N_("Find Previous"), kFindPreviousCommandID, ShiftMask|Mod1Mask, 'g' },
	{ N_("Find Next"), kFindNextCommandID, Mod1Mask, 'g' },
	{ N_("Find Selection Previous"), kFindPreviousSelectionCommandID, ShiftMask|Mod1Mask, 'h' },
	{ N_("Find Selection Next"), kFindNextSelectionCommandID, Mod1Mask, 'h' },
	{ "" }
};

static TMenuItemRec sProjectMenu[] =
{
	{ N_("Make"), kMakeCommandID, Mod1Mask, 'm' },
	{ N_("Stop Make"), kStopMakeCommandID, Mod1Mask, '.' },
	{ N_("Debug"), kDebugCommandID, Mod1Mask, 'r' },
	{ "-" },
	{ N_("Show Man Page..."), kShowManPageCommandID, Mod1Mask, 'i' },
	{ "" }
};

static TMenuItemRec sWindowsMenu[] =
{
	{ N_("About Zoinks..."), kAboutCommandID },
	{ "-" },
	{ "" }
};


TLogDocument::TLogDocument(const char* workingDirectory, bool doubleClickLines)
	:	TDocument(NULL),
		fOwner(NULL),
		fTextView(NULL),
		fWorkingDirectory(workingDirectory),
		fSourceFile(NULL),
		fAllowClose(true),
		fDoubleClickLines(doubleClickLines)
{
}


TLogDocument::TLogDocument(const TFile* sourceFile)
	:	TDocument(NULL),
		fOwner(NULL),
		fTextView(NULL),
		fWorkingDirectory(NULL),
		fSourceFile(NULL),
		fAllowClose(true),
		fDoubleClickLines(true)
{
	if (sourceFile)
		fSourceFile = new TFile(*sourceFile);
}


TLogDocument::~TLogDocument()
{
	delete fSourceFile;

	if (fOwner)
	{
		ASSERT(fOwner->GetLogDocument() == this);
		fOwner->SetLogDocument(NULL);
	}
}


void TLogDocument::Open(TDocumentWindow* window)
{	
	TFont* font = new TFont("-misc-*-medium-r-semicondensed-*-13-*-*-*-*-*-*-*,-misc-fixed-medium-r-*-*-13-*-*-*-*-*-*-*");

	TMenuBar* menuBar = MakeMenuBar(window);
	menuBar->SetWindowPositioner(WidthParent);

	TRect bounds;
	window->GetLocalBounds(bounds);
	bounds.top += menuBar->GetHeight();

	const TCoord kStatusBarHeight = 20;
	bounds.bottom -= kStatusBarHeight;
	
	TScroller* scroller = new TScroller(window, bounds, true, true);
	scroller->SetWindowPositioner(SizeRelativeParent);

	TTextView* textView = new TTextView(scroller, bounds, font, false);
	scroller->SetContainedView(textView);
	fTextView = textView;
	window->SetTarget(textView);
	if (TSyntaxTextView::DefaultLineWrap())
		textView->SetLineWrap(true);

	bounds.top = bounds.bottom;
	bounds.bottom += kStatusBarHeight;
	TStatusBar* statusBar = new TStatusBar(window, bounds, font);
	statusBar->SetWindowPositioner(BottomScrollBar);
	statusBar->InsertItem(5);

	TDocument::Open(window);

	TTextFindBehavior* findBehavior = new TTextFindBehavior;
	textView->AddBehavior(findBehavior);

	if (fDoubleClickLines)
	{
		TLogViewBehavior* logBehavior;

		if (fSourceFile)
			logBehavior = new TLogViewBehavior(fSourceFile);
		else
			logBehavior = new TLogViewBehavior(fWorkingDirectory);
			
		textView->AddBehavior(logBehavior);
	}
	
	TLineNumberBehavior* lineNumberBehavior = new TLineNumberBehavior(textView, statusBar, 0);
	textView->AddBehavior(lineNumberBehavior);
}


bool TLogDocument::AllowClose()
{
	if (fAllowClose)
		return TDocument::AllowClose();
	else
		return false;
}


TMenuBar* TLogDocument::MakeMenuBar(TWindow* window)
{
	TMenuBar* menuBar = new TMenuBar(window, TRect(0, 0, window->GetWidth(), 20));

	menuBar->AddMenu(_("File"), sFileMenu);
	menuBar->AddMenu(_("Edit"), sEditMenu);
	menuBar->AddMenu(_("Search"), sSearchMenu);
	menuBar->AddMenu(_("Project"), sProjectMenu);
	menuBar->AddMenu(new TWindowsMenu(_("Windows"), sWindowsMenu));

	return menuBar;
}


void TLogDocument::WriteToFile(TFile* file)
{
	ASSERT(fTextView);

	if (file->Exists())
		file->Open(false, false, true);
	else
		file->CreateAndOpen();

	file->Write(fTextView->GetText(), fTextView->GetTextLength());
	file->Close();
}


void TLogDocument::AppendText(const TChar* text, uint32 length, bool scrollDown)
{
	ASSERT(fTextView);
	fTextView->InsertText(fTextView->GetTextLength(), text, length);
	
	if (scrollDown)
		fTextView->ScrollSelectionIntoView(false, false);
}


// truncates specified number of characters from end
void TLogDocument::Truncate(uint32 chars)
{
	uint32 length =  fTextView->GetTextLength();
	
	if (chars > length)
		chars = length;

	fTextView->SetSelection(length - chars, length);
	fTextView->ReplaceSelection("", 0, false, false);
}


void TLogDocument::ClearText()
{
	fTextView->SetText("");
}


void TLogDocument::AppendStdout(const TChar* text, uint32 length, bool scrollDown)
{
	fStdoutBuffer.Append(text, length);
	
	// find last line break
	const char* string = fStdoutBuffer;
	const char* s = string + fStdoutBuffer.GetLength() - 1;
	
	while (s > string)
	{
		if (*s == '\n')
			break;
		else
			--s;
	}
	
	if (s > string)
	{
		AppendText(string, s - string + 1, scrollDown);
		fStdoutBuffer.Replace(0, s - string + 1, "");
	}
}


void TLogDocument::AppendStderr(const TChar* text, uint32 length, bool scrollDown)
{
	fStderrBuffer.Append(text, length);
	
		// find last line break
	const char* string = fStderrBuffer;
	const char* s = string + fStderrBuffer.GetLength() - 1;
	
	while (s > string)
	{
		if (*s == '\n')
			break;
		else
			--s;
	}
	
	if (s > string)
	{
		AppendText(string, s - string + 1, scrollDown);
		fStderrBuffer.Replace(0, s - string + 1, "");
	}
}


void TLogDocument::FlushBuffers(bool scrollDown)
{
	AppendText(fStdoutBuffer, fStdoutBuffer.GetLength(), scrollDown);
	fStdoutBuffer.SetEmpty();
	AppendText(fStderrBuffer, fStderrBuffer.GetLength(), scrollDown);
	fStderrBuffer.SetEmpty();
}


void TLogDocument::Show()
{
	ASSERT(fTextView);
	TTopLevelWindow* window = fTextView->GetTopLevelWindow();
	ASSERT(window);
	
	window->Show(true);
	window->Raise();
}


void TLogDocument::SetHideOnClose(bool hideOnClose)
{
	ASSERT(fTextView);
	TTopLevelWindow* window = fTextView->GetTopLevelWindow();
	ASSERT(window);
	window->SetHideOnClose(hideOnClose);
}


void TLogDocument::SetOwner(TLogDocumentOwner* owner)
{
	fOwner = owner;
}


