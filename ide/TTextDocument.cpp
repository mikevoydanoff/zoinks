// ========================================================================================
//	TTextDocument.cpp		 	Copyright (C) 2001-2009 Mike Lockwood. All rights reserved.
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

#include "TTextDocument.h"
#include "TEditorTextView.h"
#include "TFunctionsMenu.h"
#include "TIDEApplication.h"
#include "TLineNumberBehavior.h"
#include "TFilePathBehavior.h"
#include "TProjectCommands.h"
#include "TProjectDocument.h"
#include "THTMLCommands.h"
#include "THTMLBehavior.h"
#include "TTeXCommands.h"
#include "TTeXBehavior.h"

#include "fw/TStatusBar.h"
#include "fw/TDocumentWindow.h"
#include "fw/TWindowPositioners.h"
#include "fw/TApplication.h"
#include "fw/TMenuBar.h"
#include "fw/TWindowsMenu.h"
#include "fw/TScroller.h"
#include "fw/TFont.h"
#include "fw/TCommandID.h"
#include "fw/TTextFindBehavior.h"

#include "fw/intl.h"

#include <X11/keysym.h>
#include <stdlib.h>


long TTextDocument::sNextLineNumber = -1;


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
	{ N_("Show HTML Menu"), kToggleHTMLMenuCommandID },	
//	{ N_("Show TeX Menu"), kToggleTeXMenuCommandID },	
	{ "-" },
	{ N_("Compare Files..."), kCompareFilesCommandID },
	{ "-" },
	{ N_("Close"), kCloseCommandID, Mod1Mask, 'w' },	
	{ N_("Close All Text Windows"), kCloseAllTextWindowsCommandID, Mod1Mask|ShiftMask, 'w' },
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

static TMenuItemRec sHeadingsMenu[] =
{
	{ N_("Heading 1"), kH1TagCommandID, ControlMask, '1' },
	{ N_("Heading 2"), kH2TagCommandID, ControlMask, '2' },
	{ N_("Heading 3"), kH3TagCommandID, ControlMask, '3' },
	{ N_("Heading 4"), kH4TagCommandID, ControlMask, '4' },
	{ N_("Heading 5"), kH5TagCommandID, ControlMask, '5' },
	{ N_("Heading 6"), kH6TagCommandID, ControlMask, '6' },
	{ "" }
};

static TMenuItemRec sTablesMenu[] =
{
	{ N_("Table"), kTABLETagCommandID, ControlMask, 't' },
	{ N_("Table Row"), kTRTagCommandID, ControlMask, 'r' },
	{ N_("Table Cell"), kTDTagCommandID, ControlMask, 'd' },
	{ N_("Table Header"), kTHTagCommandID, ControlMask|ShiftMask, 'h' },
	{ N_("Caption"), kCAPTIONTagCommandID },
	{ "" }
};

static TMenuItemRec sListsMenu[] =
{
	{ N_("Ordered List"), kOLTagCommandID, ControlMask, 'o' },
	{ N_("Unordered List"), kULTagCommandID, ControlMask, 'u' },
	{ N_("List Item"), kLITagCommandID, ControlMask, 'l' },
	{ N_("Definition List"), kDLTagCommandID, ControlMask|ShiftMask, 'l' },
	{ N_("Term"), kDTTagCommandID, ControlMask|ShiftMask, 't' },
	{ N_("Definition"), kDDTagCommandID, ControlMask|ShiftMask, 'd' },
	{ "" }
};

static TMenuItemRec sHTMLMenu[] =
{
	{ N_("New HTML Document"), kHTMLTagCommandID },
	{ N_("Anchor"), kATagCommandID, ControlMask, 'a' },
	{ N_("Paragraph"), kPTagCommandID, ControlMask, 'p' },
	{ N_("Div"), kDIVTagCommandID, ControlMask, 'v' },
	{ N_("Line Break"), kBRTagCommandID, ControlMask|ShiftMask, 'b' },
	{ N_("Horizontal Rule"), kHRTagCommandID, ControlMask, 'h' },
	{ N_("Align Center"), kAlignCenterCommandID, ControlMask|ShiftMask, 'c' },
	{ N_("Align Right"), kAlignRightCommandID, ControlMask|ShiftMask, 'r' },
	{ N_("Block Quote"), kBLOCKQUOTETagCommandID },
	{ N_("Comment"), kCommentTagCommandID, ControlMask, 'c' },
	{ "-" },
	{ N_("Bold"), kBTagCommandID, ControlMask, 'b' },
	{ N_("Italic"), kITagCommandID, ControlMask, 'i' },
	{ "-" },
	{ N_("Headings"), 0, 0, 0, sHeadingsMenu },
	{ N_("Tables"), 0, 0, 0, sTablesMenu },
	{ N_("Lists"), 0, 0, 0, sListsMenu },
	{ "-" },
	{ N_("Fix Special Characters"), kFixSpecialCharsCommandID },
	{ N_("Insert Image"), kIMGTagCommandID, ControlMask|ShiftMask, 'i' },
#ifdef HAVE_LIBIMLIB	
	{ N_("Insert Thumbnail"), kThumbnailCommandID },
#endif
	{ N_("Web Lint"), kWebLintCommandID },
	{ "" }
};

static TMenuItemRec sTeXMenu[] =
{
	{ N_("New TeX Document"), kTexDocumentCommandID },
	{ N_("Definition"), kTexDefinitionCommandID, ControlMask, 'd' },
	{ N_("Lemma"), kTexLemmaCommandID, ControlMask, 'l' },
	{ N_("Theorem"), kTexTheoremCommandID, ControlMask, 't' },
	{ N_("Proof"), kTexProofCommandID, ControlMask, 'p' },
	{ N_("Corollary"), kTexCorollaryCommandID, ControlMask, 'c' },
	{ "-" },
	{ N_("Math"), kTexMathCommandID, ControlMask, 'm' },
	{ N_("Emphasis"), kTexEmphasisCommandID, ControlMask, 'e' },
	{ "-" },
	{ N_("For All"), kTexForAllCommandID, ControlMask|ShiftMask, 'a' },
	{ N_("Exists"), kTexExistsCommandID, ControlMask|ShiftMask, 'e' },
	{ N_("Element of"), kTeXElementCommandID, ControlMask, 'i' },
	{ N_("Center Dot"), kTeXCenterDotCommandID, ControlMask, '.' },
	{ "-" },
	{ N_("Generate DVI"), kTexDVICommandID, ControlMask, 'g' },
	{ N_("View in xdvi"), kTexViewXDVICommandID, ControlMask, 'v' },
	{ N_("Generate PDF"), kTexGeneratePDFCommandID },
	{ "" }
};


TTextDocument::TTextDocument(TFile* file)
	:	TDocument(file),
		fTextView(NULL),
		fFunctionsMenu(NULL),
		fHTMLMenu(NULL),
		fTeXMenu(NULL),
		fWindowsMenu(NULL),
		fMenuBar(NULL),
		fLineEndingsChanged(false)
{
	AddBehavior(new TProjectBehavior(NULL));
	
	// register for file changed notification
	gApplication->AddNotifyFileChanged(this);

}


TTextDocument::~TTextDocument()
{
	gApplication->RemoveNotifyFileChanged(this);
}


TDocument* TTextDocument::CreateDocument(TFile* file)
{
	return new TTextDocument(file);
}


void TTextDocument::Open(TDocumentWindow* window)
{
	TFont* font = new TFont("-misc-*-medium-r-semicondensed-*-13-*-*-*-*-*-*-*,-misc-fixed-medium-r-*-*-13-*-*-*-*-*-*-*");

	TMenuBar* menuBar = MakeMenuBar(window);
	menuBar->SetWindowPositioner(WidthParent);
	fMenuBar = menuBar;

	TRect bounds;
	window->GetLocalBounds(bounds);
	bounds.top += menuBar->GetHeight();

	const TCoord kStatusBarHeight = 20;
	bounds.bottom -= kStatusBarHeight;
	
	TScroller* scroller = new TScroller(window, bounds, true, true);
	scroller->SetWindowPositioner(SizeRelativeParent);

	TEditorTextView* textView = new TEditorTextView(scroller, bounds, font);
	scroller->SetContainedView(textView);
	fTextView = textView;
	window->SetTarget(textView);	
	if (TSyntaxTextView::DefaultLineWrap())
		textView->SetLineWrap(true);

	bounds.top = bounds.bottom;
	bounds.bottom += kStatusBarHeight;
	TStatusBar* statusBar = new TStatusBar(window, bounds, font);
	statusBar->SetWindowPositioner(BottomScrollBar);
	statusBar->InsertItem(5);	// for line number
	statusBar->InsertItem(100);	// for file path

	TDocument::Open(window);

	THTMLBehavior* htmlBehavior = new THTMLBehavior;
	textView->AddBehavior(htmlBehavior);

	TTeXBehavior* texBehavior = new TTeXBehavior;
	textView->AddBehavior(texBehavior);

	TTextFindBehavior* findBehavior = new TTextFindBehavior;
	textView->AddBehavior(findBehavior);

	TLineNumberBehavior* lineNumberBehavior = new TLineNumberBehavior(textView, statusBar, 0);
	textView->AddBehavior(lineNumberBehavior);

	TFilePathBehavior* filePathBehavior = new TFilePathBehavior(this, statusBar, 1);
	window->AddBehavior(filePathBehavior);
	
	if (sNextLineNumber > 0)
	{
		textView->SelectLine(sNextLineNumber - 1);
		sNextLineNumber = -1;
	}
}


TMenuBar* TTextDocument::MakeMenuBar(TWindow* window)
{
	TMenuBar* menuBar = new TMenuBar(window, TRect(0, 0, window->GetWidth(), 20));

	menuBar->AddMenu(_("File"), sFileMenu);
	menuBar->AddMenu(_("Edit"), sEditMenu);
	menuBar->AddMenu(_("Search"), sSearchMenu);
	menuBar->AddMenu(_("Project"), sProjectMenu);
	fWindowsMenu = new TWindowsMenu(_("Windows"), sWindowsMenu);
	menuBar->AddMenu(fWindowsMenu);

	return menuBar;
}


void TTextDocument::ReadFromFile(TFile* file)
{
	ASSERT(fTextView);

	file->Open(true, true, false);
	uint32 length = file->GetFileSize();
	
	char* data = (length > 0 ? (char *)malloc(length) : 0);
	ASSERT(data || length == 0);

	if (data)
	{
		file->Read(data, length);
		fTextView->SetText(data, length, true);
	}

	file->Close();
	
	// clear undo/redo, in case file was reloaded
	fTextView->ClearUndoRedo();
}


void TTextDocument::WriteToFile(TFile* file)
{
	ASSERT(fTextView);

	if (file->Exists())
		file->Open(false, false, true);
	else
		file->CreateAndOpen();

	file->Write(fTextView->GetText(), fTextView->GetTextLength());
	file->Close();
	
	fTextView->TextSaved();		// record undo/redo index at this point
	fLineEndingsChanged = false;
	
	if (Tstrcmp(gApplication->GetSettingsFilePath(), file->GetPath()) == 0)
		gApplication->ReloadSettings();
}


void TTextDocument::SetTitle(const TChar* title)
{
	TDocument::SetTitle(title);
	
	const char*	extension = TFile::GetFileExtension(title);
	
	ELanguage language = GetFileLanguage(extension);

	if (language == kLanguageC || language == kLanguageCPlusPlus || language == kLanguageJava)
		AddFunctionsMenu();
	else
		RemoveFunctionsMenu();

	fTextView->SetUseSyntaxHiliting(language != kLanguageNone);
	fTextView->SetLanguage(language);
	
	if (language == kLanguageHTML)
	{
		AddHTMLMenu();
	}
	else if (language == kLanguageTeX)
	{
		AddTeXMenu();	
	}
	else
	{
		RemoveHTMLMenu();
		RemoveTeXMenu();
	}
	
	// set spaces per tab based on file extension
	int spacesPerTab = ((TIDEApplication *)gApplication)->GetSpacesPerTab(title);
	if (spacesPerTab >= 0)
		fTextView->SetSpacesPerTab(spacesPerTab);
}



bool TTextDocument::IsModified() const
{
	return (fModified && fTextView->NeedsSaving()) || fLineEndingsChanged;
}


void TTextDocument::ShowLine(int line)
{
	if (fTextView)
		fTextView->SelectLine(line - 1);
}


void TTextDocument::DoSetupMenu(TMenu* menu)
{
	menu->EnableCommand(kToggleHTMLMenuCommandID, (fHTMLMenu != NULL));
	menu->EnableCommand(kToggleTeXMenuCommandID, (fTeXMenu != NULL));

	TDocument::DoSetupMenu(menu);
}


bool TTextDocument::DoCommand(TCommandHandler* sender, TCommandHandler* receiver, TCommandID command)
{
	if (command == kDataModifiedCommandID && sender == fTextView)
	{
		SetModified(true);
		return true;
	}
	else if (command == kLineEndingsChangedCommandID)
	{
		fLineEndingsChanged = true;
		return true;
	}
	else if (command == kToggleHTMLMenuCommandID)
	{
		ELanguage language = kLanguageNone;
	
		if (fHTMLMenu)
		{
			RemoveHTMLMenu();
			language = kLanguageNone;
		}
		else
		{
			RemoveTeXMenu();
			AddHTMLMenu();
			language = kLanguageHTML;
		}
		
		fTextView->SetUseSyntaxHiliting(language != kLanguageNone);
		fTextView->SetLanguage(language);

		return true;
	}
	else if (command == kToggleTeXMenuCommandID)
	{
		ELanguage language = kLanguageNone;
	
		if (fTeXMenu)
		{
			RemoveTeXMenu();
		}
		else
		{
			RemoveHTMLMenu();
			AddTeXMenu();
		}
		
		fTextView->SetUseSyntaxHiliting(language != kLanguageNone);
		fTextView->SetLanguage(language);

		return true;
	}
	else
		return TDocument::DoCommand(sender, receiver, command);
}


static const char* kHeaderExtensions[] = { "h", "H", "hpp", "HPP", NULL };
static const char* kSourceExtensions[] = { "c", "C", "cc", "cp", "cpp", "cc", NULL };

bool TTextDocument::DoKeyDown(KeySym key, TModifierState state, const char* string)
{
	// support for the "Andy Feature"
	// open matching header/source file with Alt-Tab or Alt-tilde
	if ((key == XK_Tab || key == XK_grave || key == XK_T || key == XK_t) && (state & Mod1Mask))
	{
		if (fFile.IsSpecified())
		{
			const TChar* extension = fFile.GetFileExtension();
			bool isHeader = false;
			bool isSource = false;

			const char** extensions = kHeaderExtensions;
			while (*extensions)
			{
				if (strcmp(*extensions++, extension) == 0)
				{
					isHeader = true;
					break;
				}
			}

			if (!isHeader)
			{
				extensions = kSourceExtensions;
				while (*extensions)
				{
					if (strcmp(*extensions++, extension) == 0)
					{
						isSource = true;
						break;
					}
				}
			}

			extensions = NULL;
			if (isHeader)
				extensions = kSourceExtensions;
			else if (isSource)
				extensions = kHeaderExtensions;

			if (extensions)
			{
				TFile* newFile = new TFile(fFile);
				const char** textExtension = extensions;

				while (*textExtension)
				{
					newFile->SetFileExtension(*textExtension++);
					
					if (newFile->Exists())
					{
						gApplication->OpenFile(newFile);
						break;
					}
				}

				delete newFile;
				
				// if file not found in same directory, search open windows instead.
				TListIterator<TTopLevelWindow>	iter(TTopLevelWindow::GetWindowList());
				TTopLevelWindow* window;
				
				while ((window = iter.Next()) != NULL)
				{
					TDocumentWindow* documentWindow = dynamic_cast<TDocumentWindow *>(window);
					if (window)
					{
						TTextDocument* document = dynamic_cast<TTextDocument *>(documentWindow->GetDocument());
						if (document)
						{
							textExtension = extensions;
			
							while (*textExtension)
							{
								TFile	file(fFile);
								
								file.SetFileExtension(*textExtension++);
								
								if (Tstrcmp(file.GetFileName(), document->GetFile().GetFileName()) == 0)
								{
									window->Raise();

									// jump out of all loops
									return true;
								}
							}
						}
					}
				}
			}
		}
		
		return true;
	}

	return TDocument::DoKeyDown(key, state, string);
}


void TTextDocument::AddFunctionsMenu()
{
	if (! fFunctionsMenu)
	{
		ASSERT(fMenuBar);
		fFunctionsMenu = new TFunctionsMenu(fTextView, _("Functions"));
		fMenuBar->AddMenu(fFunctionsMenu, fWindowsMenu);
	}
}


void TTextDocument::RemoveFunctionsMenu()
{
	if (fFunctionsMenu)
	{
		ASSERT(fMenuBar);
		fMenuBar->RemoveMenu(fFunctionsMenu);
		delete fFunctionsMenu;
		fFunctionsMenu = NULL;
	}
}


void TTextDocument::AddHTMLMenu()
{
	if (! fHTMLMenu)
	{
		ASSERT(fMenuBar);
		fHTMLMenu = fMenuBar->AddMenu(_("HTML"), sHTMLMenu, fWindowsMenu);
	}
}


void TTextDocument::RemoveHTMLMenu()
{
	if (fHTMLMenu)
	{
		ASSERT(fMenuBar);
		fMenuBar->RemoveMenu(fHTMLMenu);
		delete fHTMLMenu;
		fHTMLMenu = NULL;
	}
}


void TTextDocument::AddTeXMenu()
{
	if (! fTeXMenu)
	{
		ASSERT(fMenuBar);
		fTeXMenu = fMenuBar->AddMenu(_("TeX"), sTeXMenu, fWindowsMenu);
	}
}


void TTextDocument::RemoveTeXMenu()
{
	if (fTeXMenu)
	{
		ASSERT(fMenuBar);
		fMenuBar->RemoveMenu(fTeXMenu);
		delete fTeXMenu;
		fTeXMenu = NULL;
	}
}

