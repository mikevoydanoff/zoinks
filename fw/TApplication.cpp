// ========================================================================================
//	TApplication.cpp		 	Copyright (C) 2001-2009 Mike Lockwood. All rights reserved.
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

#include "TApplication.h"
#include "TCommonDialogs.h"
#include "TChildProcess.h"
#include "TCommandID.h"
#include "TDialogWindow.h"
#include "TDirectory.h"
#include "TDocument.h"
#include "TDocumentWindow.h"
#include "TDrawContext.h"
#include "TException.h"
#include "TFile.h"
#include "TInputContext.h"
#include "TMenu.h"
#include "TSettingsFile.h"
#include "TTextFindBehavior.h"

#include "intl.h"

#include <X11/Xutil.h>

#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <locale.h>
#include <signal.h>
#include <sys/wait.h>

#ifdef ZOINKS_USE_POLL
#include <sys/poll.h>
#endif

const TTime kFileChangedNotificationIdleTime = 2000;

TApplication* gApplication = NULL;

static TException* sSigChildException = NULL;



class TFileChangedNotificationIdler : public TIdler
{
public:
							TFileChangedNotificationIdler();
	virtual					~TFileChangedNotificationIdler();
	virtual void			DoIdle();
};


TFileChangedNotificationIdler::TFileChangedNotificationIdler()
{
	SetIdleFrequency(kFileChangedNotificationIdleTime);
}


TFileChangedNotificationIdler::~TFileChangedNotificationIdler()
{
}


void TFileChangedNotificationIdler::DoIdle()
{
	gApplication->DoCheckFileChangedNotification();
};



TApplication::TApplication(int argc, char* argv[])
	:	TWindowContext(NULL),
		fNewDocumentProc(NULL),
		fArgCount(argc),
	 	fArguments(argv),
	 	fDone(false),
	 	fInSelect(false),
	 	fExitCode(0),
	 	fSettingsFile(NULL),
	 	fModalDialog(NULL),
	 	fFindFileCancelDialog(NULL),
	 	fNextIdle(kMaxIdleTime),
	 	fFileChangedNotificationIdler(NULL),
	 	fDisplay(NULL),
	 	fDefaultScreen(0),
	 	fLeaderWindow(0),
	 	fXIM(NULL),
	 	fHiliteColor(128, 255, 255),
	 	fChildren((TListBase::CompareFunc)TChildProcess::CompareByPID,
					(TListBase::SearchFunc)TChildProcess::SearchByPID)
{
	ASSERT(gApplication == NULL);
	gApplication = this;

	gettimeofday(&fStartTime, NULL);
	
	fFileChangedNotificationIdler = new TFileChangedNotificationIdler;
}


TApplication::~TApplication()
{
	delete fFileChangedNotificationIdler;
	gApplication = NULL;
}


void TApplication::Initialize()
{
	if (setlocale(LC_ALL, "") == NULL)
		fprintf(stderr, "Warning: Could not set locale\n");

	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);

	// should allow argument for this.
	fDisplay = XOpenDisplay(NULL);
	if (!fDisplay)
	{
		printf("Could not open display\n");
		abort();
	}

	fDefaultScreen = DefaultScreen(fDisplay);

	TColor::Initialize(fDisplay, fDefaultScreen);
	TWindow::Initialize(fDisplay);
	TDrawContext::Initialize(fDisplay);
	
	if (!XSupportsLocale())
		fprintf(stderr, "Warning: X does not support locale\n");

	if (XSetLocaleModifiers("") == NULL)
		fprintf(stderr, "Warning: Could not set locale modifiers\n");

	fXIM = XOpenIM(fDisplay, NULL, NULL, NULL);
	if (!fXIM)
		fprintf(stderr, "Warning: Could not open input method\n");

	fLeaderWindow = XCreateSimpleWindow(fDisplay, GetRootWindow(), 10, 10, 10, 10, 0, 0, 0);
	
	XClassHint* classHint = XAllocClassHint();
	classHint->res_name = (char *)"Zoinks";
	classHint->res_class = (char *)"ZOINKS";
	
	XWMHints* wmHints = NULL;
	TPixmap* icon = GetIcon();
	
	if (icon)
	{
		XWMHints* wmHints = XGetWMHints(fDisplay, fLeaderWindow);
		if (!wmHints)
			wmHints = XAllocWMHints();
	
		wmHints->icon_pixmap = icon->GetPixmap();
		wmHints->icon_mask = icon->GetMask();
	
		wmHints->flags |= IconPixmapHint;
		if (wmHints->icon_mask)
			wmHints->flags |= IconMaskHint;

		TTopLevelWindow::SetDefaultIcon(icon);
	}
	
  	XmbSetWMProperties(fDisplay, fLeaderWindow, NULL, NULL, fArguments, fArgCount, NULL, wmHints, classHint);
	XFree(classHint);

	signal(SIGCHLD, SigChildHandler);
}


void TApplication::ProcessArguments()
{
	if (fArgCount > 1)
	{
		for (int i = 1; i < fArgCount; )	// note - ProcessArgument will increment i
		{
			try
			{
				ProcessArgument(i);
			}
			catch (TSystemError* error)
			{
				TCommonDialogs::AlertDialog(strerror(error->GetError()), _("Error"), NULL);
				delete error;
			}
			catch (TProgramError* error)
			{
				TCommonDialogs::AlertDialog(error->GetMessage(), _("Error"), NULL);
				delete error;
			}			
			catch (TUserCancelled* error)
			{
				// resume silently
				delete error;
			}
			catch (...)
			{
				TCommonDialogs::AlertDialog(_("Got an unknown exception"), _("Error"), NULL);
			}
		}

		if (TTopLevelWindow::GetWindowList().GetSize() == 0)
			abort();		// something must have gone horribly wrong above
	}
	else
	{
		// create a new document
		try
		{
			NewDocument();
		}
		catch (TSystemError* error)
		{
			TCommonDialogs::AlertDialog(strerror(error->GetError()), _("Error"), NULL);
			delete error;
			abort();
		}
		catch (TProgramError* error)
		{
			TCommonDialogs::AlertDialog(error->GetMessage(), _("Error"), NULL);
			delete error;
			abort();
		}			
		catch (TUserCancelled* error)
		{
			// fail silently
			delete error;
			exit(0);
		}
		catch (...)
		{
			TCommonDialogs::AlertDialog(_("Got an unknown exception"), "Error", NULL);
			abort();
		}
	}
}


void TApplication::ProcessArgument(int& argIndex)
{
	const char* argument = fArguments[argIndex++];
	TString fileName(argument);
	const char* colon = strchr(argument, ':');
	int line = 0;

	if (colon && isdigit(colon[1]))
	{
		line = atoi(colon + 1);
		fileName.Set(argument, colon - argument);
	}

	TFile* file = new TFile(fileName);
	
	TDocument* document = CreateDocument(file);
	if (document)
	{
		OpenWindowContext(document);
		if (line)
			document->ShowLine(line);
	}
}


void TApplication::Cleanup()
{
}

	
int TApplication::Run()
{
	ProcessArguments();
	
	while (! fDone)
	{	
		PollEvent();
	}

	Cleanup();
	
	return fExitCode;
}


// this function raises all modal dialogs
// it uses recursion to reverse the order of a linked list
static void RaiseModalDialogs(TDialogWindow* dialog, TTopLevelWindow* startWindow)
{
	TDialogWindow* nextDialog = dialog->GetNextDialog();

	if (nextDialog && nextDialog != startWindow)
		RaiseModalDialogs(nextDialog, startWindow);
		
	dialog->Raise();
}


void TApplication::PollEvent(bool allowSleep)
{
	try
	{
		if (sSigChildException)
		{
			TException* exception = sSigChildException;
			sSigChildException = NULL;
			throw exception;
		}
		
		if (XPending(fDisplay))
		{
			XEvent	event;
			XNextEvent(fDisplay, &event);
			TWindow* imWindow = TInputContext::GetFocusedWindow();

			if (event.type == KeyPress || event.type == KeyRelease || !XFilterEvent(&event, (imWindow ? imWindow->GetXWindow() : None)))
			{
				ASSERT(event.xany.display == fDisplay);
				TWindow* window = TWindow::GetWindow(event.xany.window);

				if (window)
				{
					if (fModalDialog)
					{
						TTopLevelWindow* topLevel = window->GetTopLevelWindow();
						
						// check for topLevel != NULL to avoid problems with menus in dialogs
						if (topLevel && topLevel != fModalDialog)
						{
							// always pass these events
							if (event.type == Expose || event.type == FocusOut)
								window->HandleEvent(event);
							else if (event.type == FocusIn)
								RaiseModalDialogs(fModalDialog, topLevel);
						}
						else
							window->HandleEvent(event);
					}
					else
						window->HandleEvent(event);
				}
				else if (event.xany.window == fLeaderWindow)
				{
					HandleLeaderWindowEvent(event);
				}
					
			}
		}

		if (!XPending(fDisplay))
			TWindow::ProcessUpdates();
	
		int sleepTime = fNextIdle - GetCurrentTime();
		if (sleepTime <= 0)
		{
			Idle();
		}
		else if (allowSleep && !XPending(fDisplay) && !CheckForSignals())
		{
			CheckForSignals();
			fInSelect = true;
			
#ifdef ZOINKS_USE_POLL
			pollfd	fd;
			fd.fd = ConnectionNumber(fDisplay);
			fd.events = POLLIN;		
	
			poll(&fd, 1, sleepTime);
#else
			fd_set	fdSet;
			timeval tv;
			
			FD_ZERO(&fdSet);
			int fd = ConnectionNumber(fDisplay);
			FD_SET(fd, &fdSet);
	
			tv.tv_sec = sleepTime / 1000;
			tv.tv_usec = (sleepTime % 1000) * 1000;
			
			select(fd + 1, &fdSet, NULL, NULL, &tv);
#endif
	
			fInSelect = false;
		}
	}
	catch (TSystemError* error)
	{
		TCommonDialogs::AlertDialog(strerror(error->GetError()), _("Error"), NULL);
		delete error;
	}
	catch (TProgramError* error)
	{
		TCommonDialogs::AlertDialog(error->GetMessage(), _("Error"), NULL);
		delete error;
	}			
	catch (TUserCancelled* error)
	{
		// resume silently
		delete error;
	}
	catch (...)
	{
		TCommonDialogs::AlertDialog("Got an unknown exception", _("Error"), NULL);
	}
}


void TApplication::Close()
{
	if (CloseSubContexts())
		Quit();
}


void TApplication::RemoveWindow(TTopLevelWindow* window)
{
	// used for applications with no documents
	fWindowList.Remove(window);
	Quit();
}


void TApplication::Quit(int exitCode)
{
	fExitCode = exitCode;
	fDone = true;
}


void TApplication::DoSetupMenu(TMenu* menu)
{
	if (fDocumentTypes.GetSize() > 0)
	{
		if (fNewDocumentProc)
			menu->EnableCommand(kNewCommandID);
		menu->EnableCommand(kOpenCommandID);
		menu->EnableCommand(kFindFileCommandID);
	}

	if (EnableSaveAll())
		menu->EnableCommand(kSaveAllCommandID);

	menu->EnableCommand(kQuitCommandID);

	TWindowContext::DoSetupMenu(menu);
}


void TApplication::GetUntitledDocumentTitle(TString& title)
{
	static int count = 1;
	char	buffer[100];
	
	sprintf(buffer, _("Untitled %d"), count);
	count++;
	
	title = buffer;
}


TDocument* TApplication::OpenFile(TFile* file, int line)
{
	// first see if it is already open
	TDocument* document = FindDocument(file->GetPath());

	if (document)
	{
		TTopLevelWindow* window = document->GetMainWindow();
		if (window)
			window->Raise();
		if (line)
			document->ShowLine(line);

		return document;
	}

	// create new document
	document = CreateDocument(file);
	if (document)
	{
		OpenWindowContext(document);
		if (line)
			document->ShowLine(line);
	}

	return document;
}


void TApplication::FindFile()
{
	static TString	fileName;
	
	bool result = TCommonDialogs::TextEntryDialog(_("File Name:"), _("Find File"), NULL, fileName);

	if (result)
	{
		const char* colon = strchr(fileName, ':');
		int line = 0;
		if (colon && isdigit(colon[1]))
		{
			line = atoi(colon + 1);
			fileName.Set(fileName, colon - fileName);
		}
	
		TFile* file = FindFile(fileName);

		if (file)
		{
			OpenFile(file, line);
			delete file;
		}
		else if (!fFindFileCancelled)
		{
			TString message("File \"");
			message += fileName;
			message += "\" not found.";
			TCommonDialogs::AlertDialog(message, _("Find File Error"), NULL);
		}
	}
}


bool TApplication::FindFileIdleProc(void* data)
{
	gApplication->PollEvent(false);
		
	if (gApplication->fFindFileCancelDialog && gApplication->fFindFileCancelDialog->IsDismissed())
	{
		gApplication->fFindFileCancelled = true;
		gApplication->fFindFileCancelDialog->Close();
		gApplication->fFindFileCancelDialog = NULL;
		return true;
	}
	else
		return false;
}


TFile* TApplication::FindFile(const char* fileName, TDocument* baseDocument, TDocument* baseDocument2)
{
	TDirectory* directory = NULL;
	TFile* result = NULL;
	
	fFindFileCancelled = false;
	
	TString	message("Searching for file \"");
	message += fileName;
	message += "\"";
	
	fFindFileCancelDialog = TCommonDialogs::CancelDialog(message, "Find File", NULL);
	
	ASSERT(fFindFileCancelDialog);
	fFindFileCancelDialog->SetNextDialog(fModalDialog);	// save current dialog
	fModalDialog = fFindFileCancelDialog;
	
	if (fileName[0] == '/')
	{
		result = new TFile(fileName);
		if (!result->Exists())
		{
			delete result;
			result = NULL;
		}
	}

	if (!result && baseDocument)
	{
		const TFile& file = baseDocument->GetFile();
		directory = file.GetDirectory();
		
		if (directory)
		{
			try
			{
				result = directory->FindFile(fileName, true, FindFileIdleProc, NULL);
			}
			catch (TUserCancelled*)
			{
			}
		
			delete directory;
		}
	}
	
	if (!result && baseDocument2 && !fFindFileCancelled)
	{
		const TFile& file = baseDocument2->GetFile();
		directory = file.GetDirectory();
		
		if (directory)
		{
			try
			{
				result = directory->FindFile(fileName, true, FindFileIdleProc, NULL);
			}
			catch (TUserCancelled*)
			{
			}
		
			delete directory;
		}
	}
	
	if (! result && !fFindFileCancelled)
	{
		directory = new TDirectory(".");
		
		try
		{
			result = directory->FindFile(fileName, true, FindFileIdleProc, NULL);
		}
		catch (TUserCancelled*)
		{
		}

		delete directory;
	}
	
	if (fFindFileCancelDialog)
	{
		ASSERT(fModalDialog == fFindFileCancelDialog);

		fFindFileCancelDialog->SetDismissed();
		fModalDialog = fFindFileCancelDialog->GetNextDialog();
		fFindFileCancelDialog->Close();
		fFindFileCancelDialog = NULL;
	}

	return result;
}


void TApplication::OpenDocuments()
{
	TList<TFile>* fileList = TCommonDialogs::OpenFiles(NULL);	

	if (fileList)
	{
		TListIterator<TFile>	iter(*fileList);

		TFile* file;
		while ((file = iter.Next()) != NULL)
		{
			OpenFile(file);
			delete file;
		}

		delete fileList;
	}
}


TDocument* TApplication::CreateDocument(TFile* file)
{
	if (file)
	{
		TListIterator<TDocumentType> iter(fDocumentTypes);
		TDocumentType* docType;

		while ((docType = iter.Next()) != NULL)
		{
			// if file is NULL, create a blank document
			if (Tstrcmp(docType->fFileExtension, file->GetFileExtension()) == 0 ||
				docType->fFileExtension.IsEmpty() /*wildcard*/)
				return docType->fCreateProc(file);
		}
	}
	else if (fNewDocumentProc)
		return fNewDocumentProc(NULL);

	return NULL;
}


TDocument* TApplication::FindDocument(const TChar* path)
{
	TListIterator<TWindowContext> iter(fSubContexts);
	TWindowContext* context;	

	while ((context = iter.Next()) != NULL)
	{
		TDocument* document = dynamic_cast<TDocument*>(context);

		if (document && strcmp(document->GetFile().GetPath(), path) == 0)
			return document;
	}

	return NULL;
}


TDocument* TApplication::FindOrOpenDocument(const TChar* path)
{
	TDocument* result = FindDocument(path);

	if (! result)
	{
		TFile* file = new TFile(path);

		if (file->Exists())
			result = OpenFile(file);
		
		delete file;
	}

	return result;
}


bool TApplication::EnableSaveAll()
{
	TListIterator<TWindowContext> iter(fSubContexts);
	TWindowContext* context;	

	while ((context = iter.Next()) != NULL)
	{
		TDocument* document = dynamic_cast<TDocument*>(context);
		
		if (document && document->IsModified())
			return true;
	}
	
	return false;
}


void TApplication::SaveAll()
{
	TListIterator<TWindowContext> iter(fSubContexts);
	TWindowContext* context;	

	while ((context = iter.Next()) != NULL)
	{
		TDocument* document = dynamic_cast<TDocument*>(context);
		
		if (document && document->IsModified())
			document->Save();
	}
}


void TApplication::GetTitle(TString& title) const
{
	title = fArguments[0];
}


void TApplication::RegisterFileType(const TChar* extension, CreateDocumentProc createProc)
{
	TDocumentType* docType = new TDocumentType;
	docType->fFileExtension = extension;
	docType->fCreateProc = createProc;

	fDocumentTypes.Insert(docType);
}


bool TApplication::DoCommand(TCommandHandler* sender, TCommandHandler* receiver, TCommandID command)
{
	switch (command)
	{
		case kNewCommandID:
			NewDocument();
			return true;

		case kOpenCommandID:
			OpenDocuments();
			return true;

		case kSaveAllCommandID:
			SaveAll();
			return true;
			
		case kFindFileCommandID:
			FindFile();
			return true;

		case kQuitCommandID:
			Close();
			return true;

		default:
			return TWindowContext::DoCommand(sender, receiver, command);
	}
}


void TApplication::OpenWindowContext(TWindowContext* context, bool show)
{
	try
	{
		TRect	bounds(0, 0, 500, 500);
		TString title;
		
		context->GetTitle(title);
		TDocumentWindow* window = new TDocumentWindow(bounds, title);
		window->Create();
		context->Open(window);
		
		if (show)
			window->Show(true);
	}
	catch (...)
	{
		context->Close();
		throw;
	}
}



const char* TApplication::GetName() const
{
	return "Zoinks";
}


TPixmap* TApplication::GetIcon()
{
	return NULL;
}


void TApplication::Beep()
{
	XBell(fDisplay, 0);
}


TCommandID TApplication::ModalDialog(TDialogWindow* dialog)
{
	ASSERT(dialog);
	dialog->SetNextDialog(fModalDialog);	// save current dialog
	fModalDialog = dialog;

	while (fModalDialog == dialog)
		PollEvent();

	return fDialogResult;
}


void TApplication::DismissDialog(TDialogWindow* dialog, TCommandID result)
{
	ASSERT(dialog);
	ASSERT(dialog == fModalDialog);

	fModalDialog = dialog->GetNextDialog();	// restore current dialog
	fDialogResult = result;

	fNextIdle = 0; // don't sleep
}


void TApplication::CreateSettingsFile(const TChar* name)
{
	ASSERT(!fSettingsFile);
	ASSERT(name);
		
	fSettingsFile = new TSettingsFile("!!! prefs !!!");
	fSettingsPath = getenv("HOME");
	if (fSettingsPath[fSettingsPath.GetLength() - 1] != '/')
		fSettingsPath += "/";
	fSettingsPath += name;

	ReloadSettings();
}


void TApplication::LoadSettings()
{
	TTextFindBehavior::ReadFromSettingsFile(fSettingsFile);
}


void TApplication::SaveSettings()
{
	TTextFindBehavior::WriteToSettingsFile(fSettingsFile);

	TFile	file(fSettingsPath);
	file.Open(false, true, true);
	fSettingsFile->WriteToFile(&file);
	file.Close();
}


void TApplication::InitDefaultSettings()
{
	TTextFindBehavior::WriteToSettingsFile(fSettingsFile);
}


void TApplication::ReloadSettings()
{
	TFile	file(fSettingsPath);
	if (file.Exists())
	{
		file.Open(true, false, false);
		fSettingsFile->ReadFromFile(&file);
		file.Close();
	}
	else
	{
		InitDefaultSettings();
		file.Open(false, true, false);
		fSettingsFile->WriteToFile(&file);
		file.Close();
	}
	
	LoadSettings();
}


void TApplication::NewDocument()
{
	TDocument* document = CreateDocument(NULL);
	if (document)
		OpenWindowContext(document);
}


void TApplication::AddIdler(TIdler* idler)
{
	fIdlers.Insert(idler);
	ComputeNextIdle();
}


void TApplication::RemoveIdler(TIdler* idler)
{
	fIdlers.Remove(idler);
	ComputeNextIdle();
}


void TApplication::NextIdleChanged(TIdler* idler)
{
	ComputeNextIdle();
}


TTime TApplication::GetCurrentTime()
{
	timeval	tv;
	
	gettimeofday(&tv, NULL);

	return (tv.tv_sec - fStartTime.tv_sec) * 1000 + (tv.tv_usec - fStartTime.tv_usec) / 1000;
}


void TApplication::PrintTimeDelta(TTime time, TString& string)
{
	char	buffer[100];
	string.SetEmpty();
	
	int hours = time / (1000 * 60 * 60);
	if (hours > 0)
	{
		sprintf(buffer, (hours == 1 ? "%d hour" : "%d hours"), hours);
		string += buffer;
		time -= (hours * 1000 * 60 * 60);
	}

	int minutes = time / (1000 * 60);
	if (minutes > 0)
	{
		if (string.GetLength() > 0)
			string += ", ";		
	
		sprintf(buffer, (minutes == 1 ? "%d minute" : "%d minutes"), minutes);
		string += buffer;
		time -= (minutes * 1000 * 60);
	}	
	
	if (string.GetLength() > 0)
		string += " and ";
	
	sprintf(buffer, "%2.3f seconds", (float)time / 1000);
	string += buffer;
}


void TApplication::Sleep(TTime time)
{
	double remaining = time;

	while (remaining > 10000.0)
	{
		usleep(10000000);
		remaining -= 10000.0;
	}
	usleep((unsigned long)remaining * 1000);
}


void TApplication::AddNotifyFileChanged(TDocument* document)
{
	ASSERT(document);
	
	if (fNotifyFileChangedList.GetSize() == 0)
		fFileChangedNotificationIdler->EnableIdling(true);

	fNotifyFileChangedList.InsertLast(document);
}


void TApplication::RemoveNotifyFileChanged(TDocument* document)
{
	ASSERT(document);

	fNotifyFileChangedList.Remove(document);
	
	if (fNotifyFileChangedList.GetSize() == 0)
		fFileChangedNotificationIdler->EnableIdling(false);
}


void TApplication::DoCheckFileChangedNotification()
{
	TListIterator<TDocument> docIter(fNotifyFileChangedList);
	TDocument* document;
	while ((document = docIter.Next()) != NULL)
		document->CheckFileModification();
}


void TApplication::Idle()
{
	TLinkedListIterator<TIdler> iter(fIdlers);
	TIdler* idler;
	TTime nextIdle = kMaxIdleTime;
	
	while ((idler = iter.Next()) != NULL)
	{
		TTime now = GetCurrentTime();

		if (idler->IdlingEnabled())
		{
			if (now >= idler->GetNextIdle())
				idler->HandleIdle();

			TTime next = idler->GetNextIdle();
			if (next < nextIdle)
				nextIdle = next;
		}	
	}

	fNextIdle = nextIdle;
}


void TApplication::ComputeNextIdle()
{
	TLinkedListIterator<TIdler> iter(fIdlers);
	TIdler* idler;
	TTime nextIdle = kMaxIdleTime;
	
	while ((idler = iter.Next()) != NULL)
	{
		if (idler->IdlingEnabled())
		{
			TTime next = idler->GetNextIdle();
			if (next < nextIdle)
				nextIdle = next;
		}	
	}

	fNextIdle = nextIdle;
}


TChildProcess* TApplication::Execute(const char* commandLine, const char* workingDirectory, bool nonBlocking, bool useStdio)
{
	TChildProcess* child = NULL;
	int err;
	int		pipeFD[2];
	int		stdinFD, stdoutFD, stderrFD;	// stdio for parent
	int		childStdIn, childStdOut, childStdErr;	// stdio for child

	if (useStdio)
	{
		// pipe for stdin
		err = pipe(pipeFD);
		ASSERT(err == 0);
		childStdIn = pipeFD[0];
		stdinFD = pipeFD[1];
		
		// pipe for stdout
		err = pipe(pipeFD);
		ASSERT(err == 0);
		childStdOut = pipeFD[1];
		stdoutFD = pipeFD[0];
	
		// pipe for stderr
		err = pipe(pipeFD);
		ASSERT(err == 0);
		childStdErr = pipeFD[1];
		stderrFD = pipeFD[0];
	
		if (nonBlocking)
		{
			err = fcntl(stdinFD, F_SETFL, O_NONBLOCK);
			ASSERT(err == 0);
			err = fcntl(stdoutFD, F_SETFL, O_NONBLOCK);
			ASSERT(err == 0);
			err = fcntl(stderrFD, F_SETFL, O_NONBLOCK);
			ASSERT(err == 0);
		}
	}
	else
	{
		stdinFD = stdoutFD = stderrFD = childStdIn = childStdOut = childStdErr = 0;
	}
	
	// now fork child process
	pid_t	pid;
	
	if ((pid = fork()) < 0)
	{
		printf("fork error\n");
		return NULL;
	}
	else if (pid == 0)
	{
		// child

		if (useStdio)
		{
			// close our copies of our parent's FDs
			close(stdinFD);
			close(stdoutFD);
			close(stderrFD);
	
			// redirect stdio
			err = dup2(childStdIn, STDIN_FILENO);
			if (err < 0)
				printf("dup2(childStdIn, STDIN_FILENO) failed with errno = %d\n", errno);
			close(childStdIn);
	
			err = dup2(childStdOut, STDOUT_FILENO);
			if (err < 0)
				printf("dup2(childStdOut, STDOUT_FILENO) failed with errno = %d\n", errno);
			close(childStdOut);
	
			err = dup2(childStdErr, STDERR_FILENO);
			if (err < 0)
				printf("dup2(childStdErr, STDERR_FILENO) failed with errno = %d\n", errno);
			close(childStdErr);
		}

		// change working directory
		if (workingDirectory)
		{
			int err = chdir(workingDirectory);
			if (err)
				printf("chdir returned %d\n", err);
		}
		
		// exec the shell
		err = execl("/bin/sh", "sh", "-c", commandLine, NULL);

		ASSERT(0);	// should never get here
	}
	else
	{
		// parent

		// mgl - possible race condition here.
		// if process exits before we create the TChildProcess, we never can handle the child died event properly.
		child = new TChildProcess(pid, stdinFD, stdoutFD, stderrFD);
		fChildren.Insert(child);

		if (useStdio)
		{
			// close our copies of our child's FDs
			close(childStdIn);
			close(childStdOut);
			close(childStdErr);
		}
	}
	
	return child;
}


void TApplication::SigChildHandler(int x)
{
	try
	{
		// only do this from a signal handler if it is safe
		if (gApplication->fInSelect)
		{
			while (gApplication->CheckForSignals()) {}
		}
	}
	catch (TException* exception)
	{
		ASSERT(sSigChildException == NULL);
		sSigChildException = exception;
	}
	catch (...)
	{
		fprintf(stderr, "Unhandled exception in SigChildHandler\n");
		abort();
	}
}


bool TApplication::CheckForSignals()
{
	pid_t	pid;
	
	int status;
	pid = waitpid(-1, &status, WNOHANG);

	if (pid > 0)
	{
		ChildDied(pid, status);
		return true;
	}
	else
		return false;
}


void TApplication::ChildDied(pid_t pid, int status)
{
	TChildProcess* child = fChildren.Search(&pid);

	if (child)
		child->Terminated(status);
}


void TApplication::HandleLeaderWindowEvent(XEvent& event)
{
}
