// ========================================================================================
//	TFileDiffDocument.cpp		 	 Copyright (C) 2003 Mike Lockwood. All rights reserved.
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

#include "TFileDiffDocument.h"
#include "TDiffListView.h"
#include "TProjectCommands.h"
#include "TLineNumberBehavior.h"
#include "TFilePathBehavior.h"
#include "GNU_diff_analyze.h"


#include "fw/TApplication.h"
#include "fw/TCommandID.h"
#include "fw/TCommonDialogs.h"
#include "fw/TDocumentWindow.h"
#include "fw/TFont.h"
#include "fw/TMenuBar.h"
#include "fw/TPanelWindow.h"
#include "fw/TPixmap.h"
#include "fw/TPixmapButton.h"
#include "fw/TScroller.h"
#include "fw/TSplitter.h"
#include "fw/TStatusBar.h"
#include "fw/TTextFindBehavior.h"
#include "fw/TWindowsMenu.h"
#include "fw/TWindowPositioners.h"

#include "fw/intl.h"

#include <X11/keysym.h>
#include <stdlib.h>
#include <limits.h>

#include "Pixmaps/LeftArrow.xpm"
#include "Pixmaps/LeftArrowDisabled.xpm"
#include "Pixmaps/RightArrow.xpm"
#include "Pixmaps/RightArrowDisabled.xpm"


static TPixmap*	sLeftArrow = NULL;
static TPixmap*	sLeftArrowDisabled = NULL;
static TPixmap*	sRightArrow = NULL;
static TPixmap*	sRightArrowDisabled = NULL;


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
	{ "-" },
	{ N_("File Format"), 0, 0, 0, sFileFormatMenu },
	{ "-" },
	{ N_("Compare Files..."), kCompareFilesCommandID },
	{ "-" },
	{ N_("Close"), kCloseCommandID, Mod1Mask, 'w' },	
	{ N_("Quit"), kQuitCommandID, Mod1Mask, 'q' },
	{ "" }
};

static TMenuItemRec sEditMenu[] = 
{
	{ N_("Undo"), kUndoCommandID, Mod1Mask, 'z' },
	{ N_("Redo"), kRedoCommandID, Mod1Mask|ShiftMask, 'z' },
	{ "-" },
	{ N_("Cut"), kCutCommandID, Mod1Mask, 'x' },
	{ N_("Copy"), kCopyCommandID, Mod1Mask, 'c' },
	{ N_("Paste"), kPasteCommandID, Mod1Mask, 'v' },
	{ N_("Clear"), kClearCommandID, Mod1Mask, 'k' },
	{ "-" },
	{ N_("Select All"), kSelectAllCommandID, Mod1Mask, 'a' },
	{ N_("Shift Left"), kShiftLeftCommandID, Mod1Mask, '[' },
	{ N_("Shift Right"), kShiftRightCommandID, Mod1Mask, ']' },
	{ N_("Balance Selection"), kBalanceSelectionCommandID, Mod1Mask, 'b' },
	{ "" }
};

static TMenuItemRec sSearchMenu[] =
{
	{ N_("Find and Replace..."), kFindCommandID, Mod1Mask, 'f' },
	{ N_("Find Previous"), kFindPreviousCommandID, ShiftMask|Mod1Mask, 'g' },
	{ N_("Find Next"), kFindNextCommandID, Mod1Mask, 'g' },
	{ N_("Find Selection Previous"), kFindPreviousSelectionCommandID, ShiftMask|Mod1Mask, 'h' },
	{ N_("Find Selection Next"), kFindNextSelectionCommandID, Mod1Mask, 'h' },
	{ "-" },
	{ N_("Replace and Find Previous"), kReplacePreviousCommandID, ShiftMask|Mod1Mask, 'l' },
	{ N_("Replace and Find Next"), kReplaceNextCommandID, Mod1Mask, 'l' },
	{ "-" },
	{ N_("Goto Line..."), kGotoLineCommandID, Mod1Mask, ',' },
	{ "" }
};

static TMenuItemRec sCompareMenu[] =
{
	{ N_("Copy to Left"), kCopyToLeftCommandID, ShiftMask|Mod1Mask, '<' },
	{ N_("Copy to Right"), kCopyToRightCommandID, ShiftMask|Mod1Mask, '>' },
	{ N_("Copy All Changes to Left"), kCopyAllToLeftCommandID },
	{ N_("Copy All Changes to Right"), kCopyAllToRightCommandID },
	{ N_("Recalculate Differences"), kRecalcDiffsCommandID },
	{ "" }
};


static TMenuItemRec sWindowsMenu[] =
{
	{ N_("About Zoinks..."), kAboutCommandID },
	{ "-" },
	{ "" }
};


TFileDiffDocument::TFileDiffDocument(const TFile& file1, const TFile& file2)
	:	TWindowContext(gApplication),
		fFile1(file1),
		fFile2(file2),
		fTextView1(NULL),
		fTextView2(NULL),
		fTextLayout1(NULL),
		fTextLayout2(NULL),
		fDiffListView(NULL),
		fLeftButton(NULL),
		fRightButton(NULL)
{
	gApplication->AddSubContext(this);
}


TFileDiffDocument::~TFileDiffDocument()
{
}


TFileDiffDocument* TFileDiffDocument::CreateDocument(const TFile& file1, const TFile& file2)
{
	return new TFileDiffDocument(file1, file2);
}


void TFileDiffDocument::Open(TDocumentWindow* window)
{
	TFont* font = new TFont("-misc-*-medium-r-semicondensed-*-13-*-*-*-*-*-*-*,-misc-fixed-medium-r-*-*-13-*-*-*-*-*-*-*");

	TMenuBar* menuBar = MakeMenuBar(window);
	menuBar->SetWindowPositioner(WidthParent);

	TRect bounds;
	window->GetLocalBounds(bounds);
	bounds.top += menuBar->GetHeight();

	const TCoord kStatusBarHeight = 20;
	
	TSplitter* splitter1 = new TSplitter(window, bounds, false, false, 0.75);
	splitter1->SetWindowPositioner(SizeRelativeParent);

	TRect	top(bounds.left, bounds.top, bounds.right, bounds.bottom - (bounds.bottom - bounds.top) / 4);
	TRect	bottom(bounds.left, top.bottom, bounds.right, bounds.bottom);
	
	TSplitter* splitter2 = new TSplitter(splitter1, top, true, false);

	TRect left(0, 0, bounds.GetWidth() / 2, top.GetHeight());
	TRect right(bounds.GetWidth() / 2, 0, bounds.GetWidth(), top.GetHeight());
	
	TWindow* leftContainer = new TWindow(splitter2, left);
	TWindow* rightContainer = new TWindow(splitter2, right);
	
	left.bottom -= kStatusBarHeight;

	TScroller* scroller1 = new TScroller(leftContainer, left, true, true);
	TScroller* scroller2 = new TScroller(rightContainer, left, true, true);
	scroller1->SetWindowPositioner(SizeRelativeParent);
	scroller2->SetWindowPositioner(SizeRelativeParent);
	
	left.top = left.bottom;
	left.bottom += kStatusBarHeight;
	
	TStatusBar* statusBar1 = new TStatusBar(leftContainer, left, font);
	statusBar1->SetWindowPositioner(BottomScrollBar);
	statusBar1->InsertItem(5);	// for line number
	statusBar1->InsertItem(70);	// for file path

	TStatusBar* statusBar2 = new TStatusBar(rightContainer, left, font);
	statusBar2->SetWindowPositioner(BottomScrollBar);
	statusBar2->InsertItem(5);	// for line number
	statusBar2->InsertItem(70);	// for file path

	TWindow* bottomContainer = new TWindow(splitter1, bottom);

	// adjust bottom to container's coordinates
	bottom.right = bottom.GetWidth();
	bottom.bottom = bottom.GetHeight();
	bottom.left = bottom.top = 0;

	TRect panelRect(bottom.left, bottom.top, bottom.right, bottom.top + 40);
	bottom.top = panelRect.bottom;
	TPanelWindow* panelWindow = new TPanelWindow(bottomContainer, panelRect);
	panelWindow->SetWindowPositioner(WidthRelativeParent);
	
	if (!sLeftArrow)
		sLeftArrow = new TPixmap(LeftArrow_xpm);
	if (!sLeftArrowDisabled)
		sLeftArrowDisabled = new TPixmap(LeftArrowDisabled_xpm);
	if (!sRightArrow)
		sRightArrow = new TPixmap(RightArrow_xpm);
	if (!sRightArrowDisabled)
		sRightArrowDisabled = new TPixmap(RightArrowDisabled_xpm);

	TRect buttonBounds(8, 8, 32, 32);
	fLeftButton = new TPixmapButton(panelWindow, buttonBounds, sLeftArrow, sLeftArrowDisabled, kCopyToLeftCommandID);
	buttonBounds.left += 34;
	buttonBounds.right += 34;
	fRightButton = new TPixmapButton(panelWindow, buttonBounds, sRightArrow, sRightArrowDisabled, kCopyToRightCommandID);
	
	// start off disabled
	fLeftButton->SetEnabled(false);
	fRightButton->SetEnabled(false);
	
	TScroller* scroller3 = new TScroller(bottomContainer, bottom, true, true);
	scroller3->SetWindowPositioner(SizeRelativeParent);

	fTextView1 = new TDiffTextView(scroller1, bounds, font);
	scroller1->SetContainedView(fTextView1);

	fTextView2 = new TDiffTextView(scroller2, bounds, font);
	scroller2->SetContainedView(fTextView2);
	
	fDiffListView = new TDiffListView(scroller3, bottom, fDiffList);
	scroller3->SetContainedView(fDiffListView);

	// do this last so everything resizes properly
	splitter2->SetChildren(leftContainer, rightContainer);
	splitter1->SetChildren(splitter2, bottomContainer);

	window->SetTarget(fDiffListView);
	window->SetWindowContext(this);

	fTextView1->AddBehavior(new TTextFindBehavior);
	fTextView2->AddBehavior(new TTextFindBehavior);
	
	fTextLayout1 = fTextView1->GetTextLayout();
	fTextLayout2 = fTextView2->GetTextLayout();

	fTextLayout1->SetLineChangeCallbacks(LinesInsertedProc, LinesDeletedProc, this);
	fTextLayout2->SetLineChangeCallbacks(LinesInsertedProc, LinesDeletedProc, this);

	TLineNumberBehavior* lineNumberBehavior = new TLineNumberBehavior(fTextView1, statusBar1, 0);
	fTextView1->AddBehavior(lineNumberBehavior);
	
	lineNumberBehavior = new TLineNumberBehavior(fTextView2, statusBar2, 0);
	fTextView2->AddBehavior(lineNumberBehavior);
	
	statusBar1->SetStatus(fFile1.GetPath(), 1);
	statusBar2->SetStatus(fFile2.GetPath(), 1);

	ReadFile(&fFile1, fTextView1);
	ReadFile(&fFile2, fTextView2);
	
	ELanguage language1 = GetFileLanguage(fFile1.GetFileExtension());
	ELanguage language2 = GetFileLanguage(fFile2.GetFileExtension());
	
	// use file2 language for file1 if file1 language is none, and vice versa
	if (language1 == kLanguageNone && language2 != kLanguageNone)
		language1 = language2;
	else if (language2 == kLanguageNone && language1 != kLanguageNone)
		language2 = language1;

	fTextView1->SetLanguage(language1);
	fTextView1->SetUseSyntaxHiliting(language1 != kLanguageNone);
	fTextView2->SetLanguage(language2);
	fTextView2->SetUseSyntaxHiliting(language2 != kLanguageNone);
	
	ComputeDiffs();
}


void TFileDiffDocument::ReadFile(TFile* file, TTextView* textView)
{
	ASSERT(textView);

	file->Open(true, false, false);
	uint32 length = file->GetFileSize();
	
	char* data = (length > 0 ? (char *)malloc(length) : 0);
	ASSERT(data || length == 0);

	if (data)
	{
		file->Read(data, length);
		textView->SetText(data, length, true);
	}

	file->Close();
}


void TFileDiffDocument::WriteFile(TFile* file, TTextView* textView)
{
	ASSERT(textView);

	file->Open(false, false, true);
	file->Write(textView->GetText(), textView->GetTextLength());
	file->Close();
}


void TFileDiffDocument::Save()
{
	if (LeftModified())
	{
		WriteFile(&fFile1, fTextView1);
		fModified1 = false;
	}

	if (RightModified())
	{
		WriteFile(&fFile2, fTextView2);
		fModified2 = false;
	}
}


static bool LinesAreEqual(uint32 leftLine, TTextLayout* leftLayout, uint32 rightLine, TTextLayout* rightLayout)
{
	STextOffset length1, length2;

	const TChar* line1 = leftLayout->GetLineText(leftLine, length1);	
	const TChar* line2 = rightLayout->GetLineText(rightLine, length2);	

	if (length1 == length2)
		return (Tstrncmp(line1, line2, length1) == 0);
	else
		return false;
}


typedef struct EquivClass
{
	uint32 			line;
	TTextLayout* 	layout;
};

void TFileDiffDocument::ComputeDiffs()
{
	uint32 leftLineCount = fTextView1->GetLineCount();
	uint32 rightLineCount = fTextView2->GetLineCount();
	int nextEquiv = 0;	// next equivalence class ID
	uint32 i;
	
	// left and right file equivalence class tables
	int* leftEquiv = new int[leftLineCount];
	int* rightEquiv = new int[rightLineCount];
	
	TDynamicArray<EquivClass> equivArray;
	
	// compute equivalence classes for lines in left file
	TTextLayout* layout = fTextLayout1;
	for (i = 0; i < leftLineCount; i++)
	{
		bool foundEquiv = false;

		// check previous equivalence classes
		for (int j = 0; j < nextEquiv; j++)
		{
			EquivClass& equiv = equivArray[j];
			if (LinesAreEqual(i, layout, equiv.line, equiv.layout))
			{
				leftEquiv[i] = j;
				foundEquiv = true;
				break;
			}
		}
		
		if (!foundEquiv)
		{
			// create a new equivalence class
			EquivClass	equiv;
			equiv.line = i;
			equiv.layout = layout;
			equivArray.InsertLast(equiv);
			leftEquiv[i] = nextEquiv++;
		}
	}

	// compute equivalence classes for lines in right file
	layout = fTextLayout2;
	for (i = 0; i < rightLineCount; i++)
	{
		bool foundEquiv = false;

		// check previous equivalence classes
		for (int j = 0; j < nextEquiv; j++)
		{
			EquivClass& equiv = equivArray[j];
			if (LinesAreEqual(i, layout, equiv.line, equiv.layout))
			{
				rightEquiv[i] = j;
				foundEquiv = true;
				break;
			}
		}
		
		if (!foundEquiv)
		{
			// create a new equivalence class
			EquivClass	equiv;
			equiv.line = i;
			equiv.layout = layout;
			equivArray.InsertLast(equiv);
			rightEquiv[i] = nextEquiv++;
		}
	}
	
	file_data files[2];
	memset(&files, 0, sizeof(files));
	
	files[0].buffered_lines = leftLineCount;
	files[1].buffered_lines = rightLineCount;
	files[0].equivs = leftEquiv;
	files[1].equivs = rightEquiv;
	files[0].equiv_max = nextEquiv;
	files[1].equiv_max = nextEquiv;
	
	diff_2_files(files, AddChange, this);
	
	delete[] leftEquiv;
	delete[] rightEquiv;
}


void TFileDiffDocument::AddChange(int line0, int line1, int deleted, int inserted, void* userData)
{
	TFileDiffDocument* self = (TFileDiffDocument *)userData;
	TDynamicArray<TDiffRec>& diffList = self->fDiffList;
	
	TDiffRec	diff;
	
	diff.leftStart = line0;
	diff.rightStart = line1;
	diff.leftEnd = line0 + deleted;
	diff.rightEnd = line1 + inserted;
	diff.enabled = true;
	
	diffList.InsertFirst(diff);
}


void TFileDiffDocument::GetTitle(TString& title) const
{
	char buffer[PATH_MAX + 100];
	
	sprintf(buffer, _("Compare Files: %s, %s"), fFile1.GetFileName(), fFile2.GetFileName());
	title = buffer;
}


TMenuBar* TFileDiffDocument::MakeMenuBar(TWindow* window)
{
	TMenuBar* menuBar = new TMenuBar(window, TRect(0, 0, window->GetWidth(), 20));

	menuBar->AddMenu(_("File"), sFileMenu);
	menuBar->AddMenu(_("Edit"), sEditMenu);
	menuBar->AddMenu(_("Search"), sSearchMenu);
	menuBar->AddMenu(_("Compare"), sCompareMenu);
	TWindowsMenu* windowsMenu = new TWindowsMenu(_("Windows"), sWindowsMenu);
	menuBar->AddMenu(windowsMenu);

	return menuBar;
}


void TFileDiffDocument::DoSetupMenu(TMenu* menu)
{
	if (LeftModified() || RightModified())
		menu->EnableCommand(kSaveCommandID);

	if (fDiffList.GetSize() > 0)
	{
		int row = fDiffListView->GetFirstSelectedRow();
		
		if (row >= 0)
		{
			const TDiffRec& diff = fDiffList[row];
	
			if (diff.enabled)
			{
				menu->EnableCommand(kCopyToLeftCommandID);
				menu->EnableCommand(kCopyToRightCommandID);
			}
		}
	}

	menu->EnableCommand(kCopyAllToLeftCommandID);
	menu->EnableCommand(kCopyAllToRightCommandID);
	menu->EnableCommand(kRecalcDiffsCommandID);
	
	TWindowContext::DoSetupMenu(menu);
}


bool TFileDiffDocument::DoCommand(TCommandHandler* sender, TCommandHandler* receiver, TCommandID command)
{
	switch (command)
	{
		case kSaveCommandID:
			Save();
			return true;

		case kDataModifiedCommandID:
			if (sender == fTextView1)
			{
				fModified1 = true;
				return true;
			}
			else if (sender == fTextView2)
			{
				fModified2 = true;
				return true;
			}
			break;

		case kSelectionChangedCommandID:
			if (sender == fDiffListView)
			{
				int row, column;
				
				if (fDiffListView->GetFirstSelectedCell(row, column))
				{
					const TDiffRec&	diff = fDiffList[row];
					
					fTextView1->SelectLineRange(diff.leftStart, diff.leftEnd);
					fTextView2->SelectLineRange(diff.rightStart, diff.rightEnd);

					fLeftButton->SetEnabled(diff.enabled);
					fRightButton->SetEnabled(diff.enabled);				
				}
				else
				{
					fLeftButton->SetEnabled(false);
					fRightButton->SetEnabled(false);				
				}
				
				return true;
			}
			break;
			
		case kCopyToLeftCommandID:
			CopyToLeft();
			return true;

		case kCopyToRightCommandID:
			CopyToRight();
			return true;
	
		case kCopyAllToLeftCommandID:
			CopyAllToLeft();
			return true;

		case kCopyAllToRightCommandID:
			CopyAllToRight();
			return true;

		case kRecalcDiffsCommandID:
			RecalcDiffs();
			fDiffListView->SetTarget();	// reset target to diff list
			return true;
	}

	return TWindowContext::DoCommand(sender, receiver, command);
}


bool TFileDiffDocument::AllowClose()
{
	if (LeftModified())
	{
		bool save;

		if (!TCommonDialogs::SaveChanges(fFile1.GetFileName(), GetMainWindow(), save))
			return false;

		if (save)
		{
			WriteFile(&fFile1, fTextView1);
			fModified1 = false;
		}
	}

	if (RightModified())
	{
		bool save;

		if (!TCommonDialogs::SaveChanges(fFile2.GetFileName(), GetMainWindow(), save))
			return false;

		if (save)
		{
			WriteFile(&fFile2, fTextView2);
			fModified2 = false;
		}
	}

	return true;
}


void TFileDiffDocument::CopyToLeft()
{
	int row = fDiffListView->GetFirstSelectedRow();
	TDiffRec& diff = fDiffList[row];

	ASSERT(diff.enabled);

	fTextView1->SelectLineRange(diff.leftStart, diff.leftEnd);
	fTextView2->SelectLineRange(diff.rightStart, diff.rightEnd);
	
	STextOffset srcStart, srcEnd;
	fTextView2->GetSelection(srcStart, srcEnd);

	fTextView1->ReplaceSelection(fTextView2->GetText() + srcStart, srcEnd - srcStart, true, false);
	
	diff.enabled = false;
	fDiffListView->RedrawCell(row, 0);

	fLeftButton->SetEnabled(false);
	fRightButton->SetEnabled(false);
}


void TFileDiffDocument::CopyToRight()
{
	int row = fDiffListView->GetFirstSelectedRow();
	TDiffRec& diff = fDiffList[row];
	
	ASSERT(diff.enabled);

	fTextView1->SelectLineRange(diff.leftStart, diff.leftEnd);
	fTextView2->SelectLineRange(diff.rightStart, diff.rightEnd);
	
	STextOffset srcStart, srcEnd;
	fTextView1->GetSelection(srcStart, srcEnd);

	fTextView2->ReplaceSelection(fTextView1->GetText() + srcStart, srcEnd - srcStart, true, false);

	diff.enabled = false;
	fDiffListView->RedrawCell(row, 0);

	fLeftButton->SetEnabled(false);
	fRightButton->SetEnabled(false);
}


void TFileDiffDocument::CopyAllToLeft()
{
	fTextView1->SelectAll();
	fTextView1->ReplaceSelection(fTextView2->GetText(), fTextView2->GetTextLength(), true, false);
	
	fDiffList.RemoveAll();
	fDiffListView->DiffListChanged();

	fLeftButton->SetEnabled(false);
	fRightButton->SetEnabled(false);
}


void TFileDiffDocument::CopyAllToRight()
{
	fTextView2->SelectAll();
	fTextView2->ReplaceSelection(fTextView1->GetText(), fTextView1->GetTextLength(), true, false);

	fDiffList.RemoveAll();
	fDiffListView->DiffListChanged();

	fLeftButton->SetEnabled(false);
	fRightButton->SetEnabled(false);
}


void TFileDiffDocument::RecalcDiffs()
{
	fDiffList.RemoveAll();
	ComputeDiffs();
	
	fDiffListView->DiffListChanged();
}


void TFileDiffDocument::LinesInsertedProc(TTextLayout* layout, uint32 line, uint32 count, void* clientData)
{
	TFileDiffDocument* self = (TFileDiffDocument*)clientData;
	ASSERT(layout == self->fTextLayout1 || layout == self->fTextLayout2);
	
	bool left = (layout == self->fTextLayout1);
	TDynamicArray<TDiffRec>& diffList = self->fDiffList;

	for (uint32 i = 0; i < diffList.GetSize(); i++)
	{
		TDiffRec& diffRec = diffList[i];
		
		uint32& diffStart = (left ? diffRec.leftStart : diffRec.rightStart);
		uint32& diffEnd = (left ? diffRec.leftEnd : diffRec.rightEnd);
		
		if (line <= diffEnd)
		{
			diffEnd += count;

			if (line <= diffStart)
				diffStart += count;
		}
	}
}


void TFileDiffDocument::LinesDeletedProc(TTextLayout* layout, uint32 line, uint32 count, void* clientData)
{
	TFileDiffDocument* self = (TFileDiffDocument *)clientData;
	ASSERT(layout == self->fTextLayout1 || layout == self->fTextLayout2);

	bool left = (layout == self->fTextLayout1);
	TDynamicArray<TDiffRec>& diffList = self->fDiffList;

	for (uint32 row = 0; row < diffList.GetSize(); row++)
	{
		TDiffRec& diffRec = diffList[row];
		
		uint32& diffStart = (left ? diffRec.leftStart : diffRec.rightStart);
		uint32& diffEnd = (left ? diffRec.leftEnd : diffRec.rightEnd);
		
		if (line + count <= diffStart)
		{
			diffStart -= count;
			diffEnd -= count;
		}
		else if (line < diffEnd)
		{
			diffRec.enabled = false;
			self->fDiffListView->RedrawCell(row, 0);
		}
	}
}
