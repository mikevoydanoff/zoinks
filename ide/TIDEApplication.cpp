// ========================================================================================
//	TIDEApplication.cpp		   Copyright (C) 2001-2008 Mike Voydanoff. All rights reserved.
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

#include "TIDEApplication.h"
#include "TDiffDialogs.h"
#include "TDirectoryDiffDocument.h"
#include "TFileDiffDocument.h"
#include "TProjectCommands.h"
#include "TProjectDocument.h"
#include "TLogDocument.h"
#include "TTextDocument.h"
#include "fw/TButton.h"
#include "fw/TChildProcess.h"
#include "fw/TCommonDialogs.h"
#include "fw/TDialogWindow.h"
#include "fw/TDirectory.h"
#include "fw/TException.h"
#include "fw/TMenu.h"
#include "fw/TStaticText.h"
#include "fw/TTopLevelWindow.h"
#include "fw/TPixmap.h"
#include "fw/TWindowPositioners.h"

#ifdef ENABLE_DEBUGGER
#include "debugger/TDebuggerCommands.h"
#include "debugger/TExecutable.h"
#include "debugger/TProcess.h"
#include "debugger/TSymDocument.h"
#include "debugger/TThread.h"
#include "debugger/TThreadContext.h"
#include "debugger/TUnixLocalTarget.h"
#include "debugger/TVariable.h"
#include "debugger/TVariableListView.h"
#include "fw/TColumnResizer.h"
#include "fw/TDocumentWindow.h"
#include "fw/TMenuBar.h"
#include "fw/TScroller.h"
#include "fw/TTypeSelectBehavior.h"
#include "fw/TWindowsMenu.h"
#endif // ENABLE_DEBUGGER

#include "fw/intl.h"

#include <stdlib.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <X11/Xatom.h>

#include "Pixmaps/zoinks.xpm"

#ifdef ENABLE_DEBUGGER
TTarget* TIDEApplication::sLocalTarget = NULL;

// menus for variable window
static TMenuItemRec sFileMenu[] = 
{
	{ N_("New"), kNewCommandID, Mod1Mask, 'n' },
	{ N_("New Project"), kNewProjectCommandID, Mod1Mask|ShiftMask, 'n' },
	{ "-" },
	{ N_("Open..."), kOpenCommandID, Mod1Mask, 'o' },
	{ N_("Find File..."), kFindFileCommandID, Mod1Mask, 'd' },
	{ "-" },
	{ N_("Close"), kCloseCommandID, Mod1Mask, 'w' },
	{ N_("Close All Variable Windows"), kCloseAllVariableWindowsCommandID, Mod1Mask|ShiftMask, 'w' },
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
	{ "" }
};

static TMenuItemRec sWindowsMenu[] =
{
	{ N_("About Zoinks..."), kAboutCommandID },
	{ "-" },
	{ "" }
};

#endif // ENABLE_DEBUGGER


TIDEApplication::TIDEApplication(int argc, char **argv)
	:	TApplication(argc, argv),
		fOpenFileAtom(0),
		fLineNumberAtom(0)
{
#ifdef ENABLE_DEBUGGER
	sLocalTarget = new TUnixLocalTarget;
	sLocalTarget->AddListener(dynamic_cast<TTargetListener *>(this));
#endif
}


TIDEApplication::~TIDEApplication()
{
}


void TIDEApplication::Initialize()
{
	TApplication::Initialize();
	
	fOpenFileAtom = XInternAtom(fDisplay, "Zoinks_Open_File", false);
	fLineNumberAtom = XInternAtom(fDisplay, "Zoinks_Line_Number", false);
}


static Window sRelayWindow = 0;
static TString sRelayArguments;

void TIDEApplication::ProcessArgument(int& argIndex)
{
	long	lineNumber;
	const char* argument = fArguments[argIndex];
	
	if (sRelayWindow != 0)
	{
		sRelayArguments += argument;
		
		if (++argIndex == fArgCount)
		{
			XEvent		event;
			Display* display = GetDisplay();
			
			event.xclient.type = ClientMessage;
			event.xclient.serial = 0;
			event.xclient.send_event = True;
			event.xclient.display = display;
			event.xclient.window = sRelayWindow;
			event.xclient.message_type = fOpenFileAtom;
			event.xclient.format = 8;

			XChangeProperty(display, sRelayWindow, fOpenFileAtom, XA_STRING, 8, PropModeReplace, 
					(const unsigned char *)(const char *)sRelayArguments, sRelayArguments.GetLength());

			XSendEvent(display, sRelayWindow, False, 0, &event);
			XFlush(display);

			exit(0);
		}
		else
			sRelayArguments += " ";
	}
	else
	{
		if (sscanf(argument, "+%ld", &lineNumber) == 1)
		{
			// support for line number option
			TTextDocument::SetNextLineNumber(lineNumber);
			argIndex++;
		}
		else if (argIndex == 1 && strcmp(fArguments[0], "zoinksdiff") == 0)
		{
			const char* path1 = fArguments[argIndex++];
			const char* path2 = fArguments[argIndex++];
			DoDiff(path1, path2);
		}			
		else if (strcmp(argument, "-d") == 0)
		{
			// support for diff from command line
			argIndex++;
			const char* path1 = fArguments[argIndex++];
			const char* path2 = fArguments[argIndex++];
			DoDiff(path1, path2);
		}
		else if (strcmp(argument, "-window") == 0)
		{
			long windowID;
	
			if (argIndex == 1 &&	// -window must be first argument
				fArgCount > 3 &&	// must have arguments following the window ID
				sscanf(fArguments[2], "%ld", &windowID) == 1)
			{
				argIndex = 3;
				sRelayWindow = windowID;
			}
			else
				argIndex++;			
		}
		else if (strcmp(argument, "-man") == 0)
		{
			// support for displaying man page from command line
			argIndex++;
			
			ShowManPage(fArguments[argIndex++]);		
		}
		else
			TApplication::ProcessArgument(argIndex);
	}
}


void TIDEApplication::DoDiff(const char* path1, const char* path2)
{
	if (path1 && path2)
	{
		TFile	file1(path1);
		TFile	file2(path2);
		
		if (file1.IsDirectory() && file2.IsDirectory())
		{
			TDirectory	directory1(path1);
			TDirectory	directory2(path2);		
			TDirectoryDiffDocument* document = TDirectoryDiffDocument::CreateDocument(directory1, directory2);
	
			if (document)
				OpenWindowContext(document);
		}
		else
		{
			TFileDiffDocument* document = TFileDiffDocument::CreateDocument(file1, file2);
	
			if (document)
				OpenWindowContext(document);
		}
	}
}


TPixmap* TIDEApplication::GetIcon()
{
	return new TPixmap(zoinks_xpm);
}


void TIDEApplication::DoSetupMenu(TMenu* menu)
{
	menu->EnableCommand(kAboutCommandID);
	menu->EnableCommand(kNewProjectCommandID);
	menu->EnableCommand(kCloseAllTextWindowsCommandID);
	menu->EnableCommand(kShowManPageCommandID);
	menu->EnableCommand(kCompareFilesCommandID);

#ifdef ENABLE_DEBUGGER
	menu->EnableCommand(kClearAllBreakpointsCommandID);
#endif

	TApplication::DoSetupMenu(menu);
}


bool TIDEApplication::DoCommand(TCommandHandler* sender, TCommandHandler* receiver, TCommandID command)
{
	switch (command)
	{
		case kAboutCommandID:
			ShowAboutBox();
			return true;
			
		case kNewProjectCommandID:
			NewProject();
			return true;
			
		case kCloseAllTextWindowsCommandID:
			CloseAllTextWindows();
			return true;

		case kShowManPageCommandID:
			ShowManPage();
			return true;

		case kCompareFilesCommandID:
			CompareFiles();
			return true;

#ifdef ENABLE_DEBUGGER
		case kClearAllBreakpointsCommandID:
			ClearAllBreakpoints();
			return true;
#endif
			
		default:
			return TApplication::DoCommand(sender, receiver, command);
	}
}


void TIDEApplication::ShowAboutBox()
{
	TRect	dialogBounds(0, 0, 400, 170);
	TDialogWindow* dialog = new TDialogWindow(dialogBounds, _("About Zoinks"), true, NULL);

	TString	copyright(_("Zoinks "));
	copyright += VERSION;
	copyright += _(" (c) 2001-2016 Mike Voydanoff.");

	TStaticText* staticText = new TStaticText(dialog, TRect(20, 20, dialogBounds.GetWidth() - 20, 36), copyright, kTextAlignCenter);
	staticText->SetWindowPositioner(CenterHorizontal);	
	staticText = new TStaticText(dialog, TRect(20, 36, dialogBounds.GetWidth() - 20, 52), _("All Rights Reserved."), kTextAlignCenter);
	staticText->SetWindowPositioner(CenterHorizontal);	
	staticText = new TStaticText(dialog, TRect(20, 52, dialogBounds.GetWidth() - 20, 68), _("https://github.com/mikevoydanoff/zoinks"), kTextAlignCenter);
	staticText->SetWindowPositioner(CenterHorizontal);	
	staticText = new TStaticText(dialog, TRect(20, 90, dialogBounds.GetWidth() - 20, 106), _("Zoinks is released under the GNU General Public License."), kTextAlignCenter);
	staticText->SetWindowPositioner(CenterHorizontal);	
	staticText = new TStaticText(dialog, TRect(20, 106, dialogBounds.GetWidth() - 20, 122), _("See \"COPYING\" for details."), kTextAlignCenter);
	staticText->SetWindowPositioner(CenterHorizontal);	

	TCoord buttonWidth = 80;
	TRect buttonBounds;
	buttonBounds.left = (dialogBounds.GetWidth() - buttonWidth) / 2;
	buttonBounds.right = buttonBounds.left + buttonWidth;
	buttonBounds.bottom = dialogBounds.bottom - 20;
	buttonBounds.top = buttonBounds.bottom - 20;

	TButton* button = new TButton(dialog, buttonBounds, _("OK"), kOKCommandID, true);
	button->SetWindowPositioner(CenterHorizontal);
	button->SetDefault();
	
	dialog->Show(true);

	gApplication->ModalDialog(dialog);
	dialog->Close();
}


void TIDEApplication::NewProject()
{
	TDocument* document = TProjectDocument::CreateDocument(NULL);
	OpenWindowContext(document);
}


void TIDEApplication::CloseAllTextWindows()
{
	TListIterator<TWindowContext>	iter(fSubContexts);
	TWindowContext* context;
	while ((context = iter.Next()) != NULL)
	{
		TTextDocument* document = dynamic_cast<TTextDocument*>(context);
		
		if (document && document->AllowClose())
			document->Close();
	}
}


void TIDEApplication::ShowManPage()
{
	TString text;
	
	if (TCommonDialogs::TextEntryDialog(_("Show man page for:"), _("Show Man Page"), NULL, text))
		ShowManPage(text);
}
	
	
void TIDEApplication::ShowManPage(const char* query)
{
	TString	command("man -a ");
	command += query;
//	command += " | col -b";	// this seems to be broken on Vine Linux and possibly other systems

	TChildProcess* child = Execute(command, NULL, false, true);

	if (child)
	{
		TLogDocument* document = new TLogDocument(NULL, false);
		document->AddBehavior(new TProjectBehavior(NULL));
	
		TString title("man ");
		title += query;
		document->SetTitle(title);
		OpenWindowContext(document);
		char* data = NULL;
		long dataLength = 0;

		while (1)
		{
			char buffer[1000];
				
			ssize_t count = read(child->GetStdout(), buffer, sizeof(buffer) - 1);
			if (count > 0)
			{
				data = (char *)realloc(data, dataLength + count);
				memcpy(data + dataLength, buffer, count);
				dataLength += count;
			}
			else
				break;
		}
		
		// remove junk from the text
		char* src = data;
		char* dest = data;
		char* end = data + dataLength;
		
		while (src < end)
		{
			char ch = *src++;
			
			if (ch == 0x1B && *src == '[')
			{
				// skip weird sequences that start with 0x1B,'['
				src += 2;
				dataLength -= 2;

				while (dataLength > 1 && *src != 'm')
				{
				    src++;
				    dataLength--;
				}
			    src++;
			    dataLength--;
			}
			else if (ch == '\b')
			{
				// process backspaces
				dest--;
				dataLength -= 2;
			}
			else if (ch == '_')
			{
				// strip underscores
				dataLength--;
			}
			else
			{
				*dest++ = ch;
			}
		}

		if (data)
		{
			document->AppendText(data, dataLength, false);
			free(data);
		}

		// see if we got anything
		const TChar* output = document->GetText();
		STextOffset length = document->GetTextLength();
		bool gotText = false;
		
		for (STextOffset i = 0; !gotText && i < length; i++)
		{
			if (isalnum(output[i]))
				gotText = true;
		}
		
		if (!gotText)
		{
			TString	message(_("No man page for "));
			message += query;
			document->ClearText();
			document->AppendText(message, message.GetLength(), false);
		}

		delete child;
	}
}


void TIDEApplication::CompareFiles()
{
	// static, to save previous settings
	static TString		path1, path2;
	static bool			compareDirectories = false;
		
	if (TDiffDialogs::CompareFilesDialog(path1, path2, compareDirectories))
	{
		if (!compareDirectories)
		{
			TFile	file1(path1);
			TFile	file2(path2);
			
			if (!file1.IsDirectory() && !file2.IsDirectory())
			{
				TFileDiffDocument* document = TFileDiffDocument::CreateDocument(file1, file2);
		
				if (document)
					OpenWindowContext(document);
			}
			else
				compareDirectories = true;
		}
		
		if (compareDirectories)
		{
			TDirectory	directory1(path1);
			TDirectory	directory2(path2);		
			TDirectoryDiffDocument* document = TDirectoryDiffDocument::CreateDocument(directory1, directory2);
	
			if (document)
				OpenWindowContext(document);
		}
	}
}


TDocument* TIDEApplication::CreateDocument(TFile* file)
{
	TDocument* document;

#ifdef ENABLE_DEBUGGER	
	document = TSymDocument::CreateDocument(file);
	if (document)
		return document;
#endif

	if (file)
	{
		document = TProjectDocument::CreateDocument(file);
		if (document)
			return document;
	}

	return TApplication::CreateDocument(file);
}


void TIDEApplication::LoadSettings()
{
	TApplication::LoadSettings();
	TEditorTextView::ReadFromSettingsFile(fSettingsFile);
}


void TIDEApplication::SaveSettings()
{
	TEditorTextView::WriteToSettingsFile(fSettingsFile);

	TApplication::SaveSettings();
}


void TIDEApplication::InitDefaultSettings()
{
	TApplication::InitDefaultSettings();
	TEditorTextView::WriteToSettingsFile(fSettingsFile);
}

int TIDEApplication::GetSpacesPerTab(const char* filename)
{
	const char*	extension = TFile::GetFileExtension(filename);
	
	if (extension)
	{
		TString setting(TSyntaxTextView::kSpacesPerTab);
		setting += ".";
		setting += extension;
		return fSettingsFile->GetIntSetting(setting, -1);
	}
	else
		return -1;
}

#ifdef ENABLE_DEBUGGER
void TIDEApplication::ClearAllBreakpoints()
{
	TExecutable::ClearAllBreakpoints();
}


void TIDEApplication::MakeVariableWindow(TVariable* variable)
{
	ASSERT(variable);

	TDocumentWindow* window = new TDocumentWindow(TRect(0, 0, 500, 500), variable->GetName());
	window->SetWindowContext(this);
	TRect bounds = window->GetBounds();

	TMenuBar* menuBar = new TMenuBar(window, TRect(0, 0, window->GetWidth(), 20));

	menuBar->AddMenu(_("File"), sFileMenu);
	menuBar->AddMenu(_("Edit"), sEditMenu);
	TWindowsMenu* windowsMenu = new TWindowsMenu(_("Windows"), sWindowsMenu);
	menuBar->AddMenu(windowsMenu);
	menuBar->SetWindowPositioner(WidthParent);
	
	bounds.top = menuBar->GetBounds().bottom;

	TScroller* scroller = new TScroller(window, bounds, true, true);
	scroller->SetWindowPositioner(SizeRelativeParent);

	TVariableListView* variableListView = new TVariableListView(scroller, TRect(0, 0, 200, 200), variable->GetProcess(), true);
	scroller->SetContainedView(variableListView);
	variableListView->AddBehavior(new TTypeSelectBehavior);
	variableListView->AddBehavior(new TColumnResizer);
	window->SetTarget(variableListView);

	window->Show(true);
	
	// need to do this after the view is created
	TCoord width = (variableListView->GetWidth() - variableListView->GetColumnWidth(0)) / 2;
	variableListView->SetColumnWidth(1, width);
	variableListView->SetColumnWidth(2, width);
	
	variableListView->SetVariable(variable);
}
#endif // ENABLE_DEBUGGER


TFile* TIDEApplication::FindFile(const char* fileName, TDocument* baseDocument, TDocument* baseDocument2)
{
	ASSERT(baseDocument2 == NULL);

	return TApplication::FindFile(fileName, baseDocument, TProjectDocument::GetCurrentProject());
}


void TIDEApplication::HandleLeaderWindowEvent(XEvent& event)
{
	if (event.xclient.type == ClientMessage && 
		event.xclient.message_type == fOpenFileAtom)
	{
		Atom			actualType;
		int				actualFormat;
		unsigned long	actualItems;
		unsigned long	bytesAfter;
		unsigned char*	value;
		long 			lineNumber = -1;
	
		XGetWindowProperty(GetDisplay(), GetLeaderWindow(), fOpenFileAtom, 0, 10000, True, XA_STRING,
						&actualType, &actualFormat, &actualItems, &bytesAfter, &value);
			 
		const char* start = (const char *)value;

		// process arguments
		while (*start)
		{
			while (*start == ' ')
				start++;
			
			const char* end = start;
			
			while (*end && *end != ' ')
				end++;
				
			if (end != start)
			{				
				if (sscanf(start, "+%ld", &lineNumber) == 1)
				{
				}
				else
				{
					TString	path(start, end - start);
										
					TTextDocument* textDocument = dynamic_cast<TTextDocument*>(FindOrOpenDocument(path));
			
					if (textDocument)
					{
						TTopLevelWindow* window = textDocument->GetMainWindow();
						if (window)
							window->Raise();
						
						if (lineNumber > 0)
						{
							textDocument->GetTextView()->SelectLine(lineNumber - 1);
							lineNumber = -1;
						}
					}
				}	
			}
		
			start = end;
		}
	

		XFree(value);
	}
}


#ifdef ENABLE_DEBUGGER

void TIDEApplication::ProcessCreated(TProcess* process)
{
	process->AddListener(dynamic_cast<TProcessListener *>(this));
}


void TIDEApplication::ThreadCreated(TThread* thread)
{
	thread->AddListener(dynamic_cast<TThreadListener *>(this));
}


void TIDEApplication::ThreadSuspended(TThread* thread)
{
	// make sure we have a thread context
	TListIterator<TWindowContext> iter(fSubContexts);

	TWindowContext* context;
	bool found = false;
	while ((context = iter.Next()) != NULL && !found)
	{
		TThreadContext* threadContext = dynamic_cast<TThreadContext*>(context);
		if (threadContext && threadContext->GetThread() == thread)
			found = true;
	}

	if (!found)
	{
		TThreadContext* context = new TThreadContext(thread);
		AddSubContext(context);

//		ThreadSuspended will happen automatically since the TThreadContext will be added to the end of the listener list during iteration.
//		context->ThreadSuspended(thread);
	}
}

void TIDEApplication::ChildDied(pid_t pid, int status)
{
	TChildProcess* child = fChildren.Search(&pid);

	if (child)
		child->Terminated(status);
	else
		((TUnixLocalTarget *)sLocalTarget)->ProcessSignal(pid, status);
}

#endif // ENABLE_DEBUGGER

