// ========================================================================================
//	TProjectDocument.cpp		Copyright (C) 2001-2003 Mike Lockwood. All rights reserved.
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

#include "TProjectDocument.h"
#include "TLogDocument.h"
#include "TProjectCommands.h"
#include "fw/TApplication.h"
#include "fw/TButton.h"
#include "fw/TCheckBox.h"
#include "fw/TChildProcess.h"
#include "fw/TCommonDialogs.h"
#include "fw/TDirectory.h"
#include "fw/TDocumentWindow.h"
#include "fw/TDrawContext.h"
#include "fw/TException.h"
#include "fw/TFont.h"
#include "fw/TMenuBar.h"
#include "fw/TStaticText.h"
#include "fw/TTabTargetBehavior.h"
#include "fw/TTextField.h"
#include "fw/TTopLevelWindow.h"
#include "fw/TWindowPositioners.h"
#include "fw/TWindowsMenu.h"

#include "fw/intl.h"

#ifdef ENABLE_DEBUGGER
#include "debugger/TDebuggerCommands.h"
#include "debugger/TSymDocument.h"
#endif

#include <unistd.h>
#include <stdlib.h>

// for IsProjectFile to work correctly, these strings should be the same length.
const char kOldProjectFileSignature[]		= "#@!? VoodooMonkey Project";
const char kProjectFileSignature[]			= "#@!?- Zoinks Project File";

const char kMakePathSetting[] 				= "MakePath";
const char kMakeCommandSetting[] 			= "MakeCommand";
const char kExternalDebuggerCommandSetting[] = "ExternalDebuggerCommand";
const char kMakeBeforeDebugSetting[]		= "MakeBeforeDebug";

#ifdef ENABLE_DEBUGGER
const char kDebugPathSetting[] 				= "DebugPath";
const char kDebugArgumentsSetting[]			= "DebugCommand";
const char kUseExternalDebuggerSetting[]	= "UseExternalDebugger";
#endif


TProjectDocument* TProjectDocument::sCurrentProject = NULL;

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


TProjectDocument::TProjectDocument(TFile* file)
	:	TDocument(file),
		TLogDocumentOwner(true),
		fSettingsFile(kProjectFileSignature),
#ifdef ENABLE_DEBUGGER
		fUseExternalDebugger(false),
#endif
		fMakeBeforeDebug(true),
		fMakePathText(NULL),
		fMakeCommandText(NULL),
		fExternalDebuggerCommandText(NULL),
#ifdef ENABLE_DEBUGGER
		fDebugPathText(NULL),
		fDebugArgumentsText(NULL),
		fUseExternalDebuggerBox(NULL),
#endif
		fMakeBeforeDebugBox(NULL),
		fMakeChild(NULL)
#ifdef ENABLE_DEBUGGER
		, fSymDocument(NULL)
#endif
{
	SetIdleFrequency(250);
	
	if (!file)
		SetModified(true);
		
	sCurrentProject = this;
}


TProjectDocument::~TProjectDocument()
{
	if (sCurrentProject == this)
		sCurrentProject = NULL;
	
#ifdef ENABLE_DEBUGGER
	if (fSymDocument)
	{
		ASSERT(fSymDocument->GetProjectDocument() == this);
		fSymDocument->SetProjectDocument(NULL);
	}
#endif
}


bool TProjectDocument::IsProjectFile(TFile* file)
{
	bool result = false;
	ASSERT(file);
	
	uint32 fileLength = file->GetFileSize();
	
	if (fileLength < strlen(kProjectFileSignature))
		return false;
			
	try
	{
		file->Open(true, false, false);
	
		char	header[100];

		file->Read(header, strlen(kProjectFileSignature));
		
		if (strncmp(header, kProjectFileSignature, strlen(kProjectFileSignature)) == 0 ||
			strncmp(header, kOldProjectFileSignature, strlen(kOldProjectFileSignature)) == 0)
			result = true;
	}
	catch (...)
	{
	}

	file->Close();

	return result;
}


TDocument* TProjectDocument::CreateDocument(TFile* file)
{
	if (!file || IsProjectFile(file))
		return new TProjectDocument(file);
	else
		return NULL;
}


void TProjectDocument::SetCurrentProject(TProjectDocument* project)
{
	// look for other projects
	if (! project)
	{
		TListIterator<TWindowContext>	iter(gApplication->GetSubContextList());
		TWindowContext* context;
		while ((context = iter.Next()) != NULL)
		{
			TProjectDocument* document = dynamic_cast<TProjectDocument*>(context);
			
			if (document && document != sCurrentProject)
			{
				project = document;
				break;
			}
		}	
	
	}
	
	sCurrentProject = project;
}


void TProjectDocument::Open(TDocumentWindow* window)
{
	window->SetBackColor(kLightGrayColor);

	TMenuBar* menuBar = MakeMenuBar(window);
	menuBar->SetWindowPositioner(WidthParent);

	TRect bounds;
	window->GetLocalBounds(bounds);
	bounds.top += menuBar->GetHeight();

	TRect	r(bounds.left + 10, bounds.top + 10, bounds.right - 100, bounds.top + 30);
	TStaticText* staticText = new TStaticText(window, r, _("Build Directory:"));

	r.Offset(0, 20);
	fMakePathText = new TTextField(window, r, TDrawContext::GetDefaultFont(), true);
	fMakePathText->SetFilterTabAndCR(true);
	fMakePathText->SetWindowPositioner(WidthRelativeParent);

	TRect r2(r.right + 10, r.top, r.right + 10 + 60, r.top + 20);
	TButton* button = new TButton(window, r2, _("Choose"), kChooseMakePathCommandID);
	button->SetWindowPositioner(RightRelative);

	r.Offset(0, 30);
	staticText = new TStaticText(window, r, _("Build Command:"));

	r.Offset(0, 20);
	fMakeCommandText = new TTextField(window, r, TDrawContext::GetDefaultFont(), true);
	fMakeCommandText->SetFilterTabAndCR(true);
	fMakeCommandText->SetWindowPositioner(WidthRelativeParent);

#ifdef ENABLE_DEBUGGER
	r.Offset(0, 30);
	staticText = new TStaticText(window, r, _("Executable to Debug:"));

	r.Offset(0, 20);
	fDebugPathText = new TTextField(window, r, TDrawContext::GetDefaultFont(), true);
	fDebugPathText->SetFilterTabAndCR(true);
	fDebugPathText->SetWindowPositioner(WidthRelativeParent);

	r2.Set(r.right + 10, r.top, r.right + 10 + 60, r.top + 20);
	button = new TButton(window, r2, _("Choose"), kChooseDebugPathCommandID);
	button->SetWindowPositioner(RightRelative);

	r.Offset(0, 30);
	staticText = new TStaticText(window, r, _("Debug Executable Arguments:"));

	r.Offset(0, 20);
	fDebugArgumentsText = new TTextField(window, r, TDrawContext::GetDefaultFont(), true);
	fDebugArgumentsText->SetFilterTabAndCR(true);
	fDebugArgumentsText->SetWindowPositioner(WidthRelativeParent);
#endif // ENABLE_DEBUGGER

	r.Offset(0, 30);

#ifdef ENABLE_DEBUGGER
	staticText = new TStaticText(window, r, _("External Debugger Command:"));
#else
	staticText = new TStaticText(window, r, _("Debugger Command:"));
#endif

	r.Offset(0, 20);
	fExternalDebuggerCommandText = new TTextField(window, r, TDrawContext::GetDefaultFont(), true);
	fExternalDebuggerCommandText->SetFilterTabAndCR(true);
	fExternalDebuggerCommandText->SetWindowPositioner(WidthRelativeParent);

	r.Offset(0, 30);

#ifdef ENABLE_DEBUGGER
	fUseExternalDebuggerBox = new TCheckBox(window, r, _("Use External Debugger"));
	r.Offset(0, 20);
#endif // ENABLE_DEBUGGER
	
	fMakeBeforeDebugBox = new TCheckBox(window, r, _("Rebuild before debugging"));

	fMakePathText->SetText(fMakePath);
	fMakeCommandText->SetText(fMakeCommand);
	fExternalDebuggerCommandText->SetText(fExternalDebuggerCommand);

#ifdef ENABLE_DEBUGGER
	fDebugPathText->SetText(fDebugPath);
	fDebugArgumentsText->SetText(fDebugArguments);
	fUseExternalDebuggerBox->SetChecked(fUseExternalDebugger);
#endif

	fMakeBeforeDebugBox->SetChecked(fMakeBeforeDebug);

	bounds = window->GetBounds();
	bounds.bottom = r.bottom + 10;
	window->SetBounds(bounds);
	window->AddBehavior(new TTabTargetBehavior);
	
	TDocument::Open(window);

	window->SetTarget(fMakePathText);
}


TMenuBar* TProjectDocument::MakeMenuBar(TWindow* window)
{
	TMenuBar* menuBar = new TMenuBar(window, TRect(0, 0, window->GetWidth(), 20));

	menuBar->AddMenu(_("File"), sFileMenu);
	menuBar->AddMenu(_("Edit"), sEditMenu);
	menuBar->AddMenu(_("Project"), sProjectMenu);
	menuBar->AddMenu(new TWindowsMenu(_("Windows"), sWindowsMenu));

	return menuBar;
}



void TProjectDocument::ReadFromFile(TFile* file)
{
	file->Open(true, false, false);
	fSettingsFile.ReadFromFile(file);
	file->Close();

	const char* value = fSettingsFile.GetStringSetting(kMakePathSetting);
	if (value)
		SetMakePath(value);
	value = fSettingsFile.GetStringSetting(kMakeCommandSetting);
	if (value)
		SetMakeCommand(value);
	value = fSettingsFile.GetStringSetting(kExternalDebuggerCommandSetting);
	if (value)
		SetExternalDebuggerCommand(value);
#ifdef ENABLE_DEBUGGER
	value = fSettingsFile.GetStringSetting(kDebugPathSetting);
	if (value)
		SetDebugPath(value);
	value = fSettingsFile.GetStringSetting(kDebugArgumentsSetting);
	if (value)
		SetDebugArguments(value);
	SetUseExternalDebugger(fSettingsFile.GetBoolSetting(kUseExternalDebuggerSetting, fUseExternalDebugger));
#endif

	SetMakeBeforeDebug(fSettingsFile.GetBoolSetting(kMakeBeforeDebugSetting, fMakeBeforeDebug));
}


void TProjectDocument::WriteToFile(TFile* file)
{
	fSettingsFile.SetStringSetting(kMakePathSetting, fMakePath);
	fSettingsFile.SetStringSetting(kMakeCommandSetting, fMakeCommand);
	fSettingsFile.SetStringSetting(kExternalDebuggerCommandSetting, fExternalDebuggerCommand);
	
#ifdef ENABLE_DEBUGGER
	fSettingsFile.SetStringSetting(kDebugPathSetting, fDebugPath);
	fSettingsFile.SetStringSetting(kDebugArgumentsSetting, fDebugArguments);
	fSettingsFile.SetBoolSetting(kUseExternalDebuggerSetting, fUseExternalDebugger);
#endif

	fSettingsFile.SetBoolSetting(kMakeBeforeDebugSetting, fMakeBeforeDebug);

	if (file->Exists())
		file->Open(false, false, true);
	else
		file->CreateAndOpen();

	fSettingsFile.WriteToFile(file);

	file->Close();
}


void TProjectDocument::Make()
{
	ASSERT(!fMakeChild);
	
#ifdef ENABLE_DEBUGGER
	if (fSymDocument)
	{
		if (!fSymDocument->AllowClose())
			ThrowUserCancelled();
	
		fSymDocument->Close();
		fSymDocument = NULL;
	}
#endif

	gApplication->SaveAll();
	fDebugAfterMake = false;
		
	TString path;
	
	// check for absolute path
	const char* test = fMakePath;
	while (*test == ' ')
		test++;
	bool absolutePath = (*test == '/');
	
	if (!absolutePath && fFile.IsSpecified())
	{
		fFile.GetDirectory(path);
		path += fMakePath;
		TFile::NormalizePath(path);
	}
	else
		path = fMakePath;
		
	fBuildStartTime = gApplication->GetCurrentTime();
	fMakeChild = gApplication->Execute(fMakeCommand, path, true, true);

	if (!fLogDocument)
	{
		// compute path for compiler output
		TString	path;

		if (fFile.IsSpecified())
		{
			fFile.GetDirectory(path);
			path += fMakePath;
		}
		else
			path = fMakePath;

		TFile::NormalizePath(path);
		
		TLogDocument* logDocument = new TLogDocument(path);

		TString title;
		GetTitle(title);
		
		// strip ".proj" suffix
		if (strcmp((const char *)title + title.GetLength() - 5 , ".proj") == 0)
			title.Truncate(title.GetLength() - 5);
		
		title += _(" Build Output");
		logDocument->SetTitle(title);
		gApplication->OpenWindowContext(logDocument);
		logDocument->SetAllowClose(false);
		logDocument->SetHideOnClose(true);
		
		logDocument->AddBehavior(new TProjectBehavior(this));
		
		SetLogDocument(logDocument);
	}
	else
	{
		fLogDocument->ClearText();
		fLogDocument->Show();
	}

	EnableIdling(true);
}


void TProjectDocument::StopMake()
{
	ASSERT(fMakeChild);
	fMakeChild->Terminate();
	
	if (fLogDocument)
	{
		fLogDocument->FlushBuffers(true);
		
		const char* done = _("\n\nBuild Aborted.");
		fLogDocument->AppendText(done, strlen(done), true);
		fLogDocument->SetAllowClose(true);
	}

	delete fMakeChild;
	fMakeChild = NULL;
	EnableIdling(false);
}


void TProjectDocument::Debug(bool makeFirst)
{
#ifdef ENABLE_DEBUGGER
	if (fSymDocument)
	{
		if (!fSymDocument->AllowClose())
			ThrowUserCancelled();

		fSymDocument->Close();
		fSymDocument = NULL;
	}
#endif

	if (makeFirst)
	{
		Make();
		fDebugAfterMake = true;
	}
	else
	{
#ifdef ENABLE_DEBUGGER
		if (fUseExternalDebugger)
#endif
		{
			// fork child process
			pid_t	pid = fork();
			
			if (pid == 0)
			{
				// child
				TString		workingDirectory;
			
				fFile.GetDirectory(workingDirectory);
				chdir(workingDirectory);

				const char* command = fExternalDebuggerCommand;
				execl("/bin/sh", "sh", "-c", command, NULL);
			}
			else if (pid < 0)
				ThrowSystemError(pid);
		}
#ifdef ENABLE_DEBUGGER
		else
		{
			TString path;
			
			if (fFile.IsSpecified())
			{
				fFile.GetDirectory(path);
				path += fDebugPath;
				TFile::NormalizePath(path);
			}
			else
				path = fDebugPath;
				
			TFile	executable(path);
			TSymDocument* document = TSymDocument::CreateDocument(&executable);
			
			if (document)
			{
				SetSymDocument(document);
				document->SetProjectDocument(this);
				gApplication->OpenWindowContext(document);
				
				// start the debuggee
				document->HandleCommand(this, document, kRunCommandID);
			}
			else
				gApplication->Beep();	// need an error here
		}
#endif // ENABLE_DEBUGGER
	}
}


void TProjectDocument::DoIdle()
{
	ASSERT(fMakeChild);

	char buffer[1000];

	bool running = fMakeChild->IsRunning();

	ssize_t outCount = read(fMakeChild->GetStdout(), buffer, sizeof(buffer) - 1);
	if (outCount > 0)
		fLogDocument->AppendStdout(buffer, outCount, true);

	ssize_t errCount = read(fMakeChild->GetStderr(), buffer, sizeof(buffer) - 1);
	if (errCount > 0)
		fLogDocument->AppendStderr(buffer, errCount, true);

	if (!running && outCount == 0 && errCount == 0)
	{
		fLogDocument->FlushBuffers(true);
		
		TTime now = gApplication->GetCurrentTime();

		if (fMakeChild->GetExitStatus() == 0)
		{
			const char* done = _("\n\nBuild Succeeded.  Elapsed time: ");
			fLogDocument->AppendText(done, strlen(done), true);
		}
		else
		{
			const char* done = _("\n\nBuild Failed.  Elapsed time: ");
			fLogDocument->AppendText(done, strlen(done), true);
			
			fDebugAfterMake = false;
		}
			
		TString timeDelta;
		gApplication->PrintTimeDelta(now - fBuildStartTime, timeDelta);
		timeDelta += "\n";
		fLogDocument->AppendText(timeDelta, timeDelta.GetLength(), true);

		delete fMakeChild;
		fMakeChild = NULL;
		fLogDocument->SetAllowClose(true);
		EnableIdling(false);
		
		if (fDebugAfterMake)
			Debug(false);
	}
}


void TProjectDocument::SetMakePath(const TChar* path)
{
	fMakePath = path;
	if (fMakePathText)
		fMakePathText->SetText(path);
}


void TProjectDocument::SetMakeCommand(const TChar* command)
{
	fMakeCommand = command;
	if (fMakeCommandText)
		fMakeCommandText->SetText(command);
}


void TProjectDocument::SetExternalDebuggerCommand(const TChar* command)
{
	fExternalDebuggerCommand = command;
	if (fExternalDebuggerCommandText)
		fExternalDebuggerCommandText->SetText(command);
}


#ifdef ENABLE_DEBUGGER
void TProjectDocument::SetDebugPath(const TChar* path)
{
	fDebugPath = path;
	if (fDebugPathText)
		fDebugPathText->SetText(path);
}


void TProjectDocument::SetDebugArguments(const TChar* arguments)
{
	fDebugArguments = arguments;
	if (fDebugArgumentsText)
		fDebugArgumentsText->SetText(arguments);
}


void TProjectDocument::SetUseExternalDebugger(bool useExternalDebugger)
{
	fUseExternalDebugger = useExternalDebugger;
	if (fUseExternalDebuggerBox)
		fUseExternalDebuggerBox->SetChecked(useExternalDebugger);
}
#endif // ENABLE_DEBUGGER


void TProjectDocument::SetMakeBeforeDebug(bool makeBeforeDebug)
{
	fMakeBeforeDebug = makeBeforeDebug;
	if (fMakeBeforeDebugBox)
		fMakeBeforeDebugBox->SetChecked(makeBeforeDebug);
}


void TProjectDocument::DoSetupMenu(TMenu* menu)
{
	if (IsMaking())
		menu->EnableCommand(kStopMakeCommandID);
	else
	{
		menu->EnableCommand(kMakeCommandID);
		menu->EnableCommand(kDebugCommandID);
	}
	
	TDocument::DoSetupMenu(menu);
}


bool TProjectDocument::DoCommand(TCommandHandler* sender, TCommandHandler* receiver, TCommandID command)
{
	switch (command)
	{
		case kChooseMakePathCommandID:
		{
			TDirectory* directory = TCommonDialogs::ChooseDirectory(GetMainWindow()->GetTopLevelWindow());
			if (directory)
			{
				TString path;
				
				if (fFile.IsSpecified())
					TFile::ComputeRelativePath(directory->GetPath(), fFile.GetPath(), path);
				else
					path = directory->GetPath();
					
				SetMakePath(path);	
				SetModified(true);
				
				delete directory;
			}
			
			return true;
		}
		
#ifdef ENABLE_DEBUGGER
		case kChooseDebugPathCommandID:
		{
			TFile* file = TCommonDialogs::OpenFile(GetMainWindow()->GetTopLevelWindow());
			if (file)
			{
				TString path;
				
				if (fFile.IsSpecified())
					TFile::ComputeRelativePath(file->GetPath(), fFile.GetPath(), path);
				else
					path = file->GetPath();
					
				SetDebugPath(path);
				SetModified(true);
				
				delete file;
			}
			
			return true;
		}
#endif // ENABLE_DEBUGGER

		case kMakeCommandID:
			Make();
			return true;

		case kStopMakeCommandID:
			StopMake();
			return true;

		case kDebugCommandID:
			Debug(fMakeBeforeDebug);
			return true;

		case kDataModifiedCommandID:
		{
			if (sender == fMakePathText)
			{
				fMakePathText->GetText(fMakePath);
				SetModified(true);
				return true;
			}
			else if (sender == fMakeCommandText)
			{
				fMakeCommandText->GetText(fMakeCommand);
				SetModified(true);
				return true;
			}
			else if (sender == fExternalDebuggerCommandText)
			{
				fExternalDebuggerCommandText->GetText(fExternalDebuggerCommand);
				SetModified(true);
				return true;
			}
#ifdef ENABLE_DEBUGGER
			else if (sender == fDebugPathText)
			{
				fDebugPathText->GetText(fDebugPath);
				SetModified(true);
				return true;
			}
			else if (sender == fDebugArgumentsText)
			{
				fDebugArgumentsText->GetText(fDebugArguments);
				SetModified(true);
				return true;
			}
#endif // ENABLE_DEBUGGER
		}
		
		case kValueChangedCommandID:
		{
			if (sender == fMakeBeforeDebugBox)
			{
				fMakeBeforeDebug = fMakeBeforeDebugBox->IsChecked();
				SetModified(true);
				return true;
			}
#ifdef ENABLE_DEBUGGER
			else if (sender == fUseExternalDebuggerBox)
			{
				fUseExternalDebugger = fUseExternalDebuggerBox->IsChecked();
				SetModified(true);
				return true;
			}
#endif // ENABLE_DEBUGGER
		}

		case kFocusAcquiredCommandID:
			sCurrentProject = this;
			return false;	// propagate command
	}

	return TDocument::DoCommand(sender, receiver, command);
}


TProjectBehavior::TProjectBehavior(TProjectDocument* project)
	:	fProjectDocument(project)
{
}


TProjectBehavior::~TProjectBehavior()
{
}


void TProjectBehavior::DoSetupMenu(TMenu* menu)
{
	TProjectDocument* project = (fProjectDocument ? fProjectDocument : TProjectDocument::GetCurrentProject());
	
	if (project)
	{
		if (project->IsMaking())
			menu->EnableCommand(kStopMakeCommandID);
		else
		{
			menu->EnableCommand(kMakeCommandID);
			menu->EnableCommand(kDebugCommandID);
		}
	}
}


bool TProjectBehavior::DoCommand(TCommandHandler* sender, TCommandHandler* receiver, TCommandID command)
{
	TProjectDocument* project = (fProjectDocument ? fProjectDocument : TProjectDocument::GetCurrentProject());
	
	if (project)
	{
		switch (command)
		{
			case kMakeCommandID:
				project->Make();
				return true;

			case kStopMakeCommandID:
				project->StopMake();
				return true;
								
			case kDebugCommandID:
				project->Debug(project->MakeBeforeDebug());
				return true;
		}
	}
	
	return false;
}

