// ========================================================================================
//	TApplication.h			   Copyright (C) 2001-2009 Mike Voydanoff. All rights reserved.
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

#ifndef __TApplication__
#define __TApplication__

#include "TWindowContext.h"
#include "TIdler.h"
#include "TString.h"
#include "TColor.h"
#include "TList.h"
#include "TSettingsFile.h"

#include <sys/time.h>
#include <X11/Xlib.h>


class TApplication;
class TDocument;
class TFile;
class TFileChangedNotificationIdler;
class TDialogWindow;
class TDocumentType;
class TChildProcess;
class TPixmap;


typedef TDocument* (* CreateDocumentProc)(TFile* file);

extern TApplication* gApplication;


class TDocumentType
{
public:
	TString				fFileExtension;
	CreateDocumentProc	fCreateProc;
};


class TApplication : public TWindowContext
{
public:
							TApplication(int argc, char* argv[]);
	virtual					~TApplication();

	virtual void			Initialize();
	virtual void			ProcessArguments();
	virtual void			ProcessArgument(int& argIndex);

	virtual void			Cleanup();

	int		 				Run();
	virtual void 		 	Close();
	void					Quit(int exitCode = 0);
	virtual void			RemoveWindow(TTopLevelWindow* window);

	virtual void			DoSetupMenu(TMenu* menu);

	void					GetUntitledDocumentTitle(TString& title);
	TDocument*				OpenFile(TFile* file, int line = 0);
	void					OpenDocuments();
	virtual TDocument*		CreateDocument(TFile* file);
	TDocument*				FindDocument(const TChar* path);
	TDocument*				FindOrOpenDocument(const TChar* path);
	bool					EnableSaveAll();
	void					SaveAll();

	virtual void			GetTitle(TString& title) const;

	void					FindFile();
	virtual TFile*			FindFile(const char* fileName, TDocument* baseDocument = NULL, TDocument* baseDocument2 = NULL);

	void					RegisterFileType(const TChar* extension, CreateDocumentProc createProc);
	inline void				SetNewDocumentProc(CreateDocumentProc proc) { fNewDocumentProc = proc; }

	virtual bool			DoCommand(TCommandHandler* sender, TCommandHandler* receiver, TCommandID command);

	virtual const char*		GetName() const;
	virtual TPixmap*		GetIcon();
	
	void					Beep();
	
	inline int				GetArgumentCount() const { return fArgCount; }
	inline char**			GetArguments() const { return fArguments; }

	TCommandID				ModalDialog(TDialogWindow* dialog);
	void					DismissDialog(TDialogWindow* dialog, TCommandID result);
	
	void					CreateSettingsFile(const TChar* name);
	inline TSettingsFile*	GetSettingsFile() const { return fSettingsFile; }
	
	inline const TString&	GetSettingsFilePath() const { return fSettingsPath; }

	virtual void			LoadSettings();
	virtual void			SaveSettings();
	virtual void			InitDefaultSettings();
	void					ReloadSettings();

	// idling support
	void					AddIdler(TIdler* idler);
	void					RemoveIdler(TIdler* idler);
	void					NextIdleChanged(TIdler* idler);
	TTime					GetCurrentTime();
	void					PrintTimeDelta(TTime time, TString& string);
	void					Sleep(TTime time);
	
	// file changed notification support
	void					AddNotifyFileChanged(TDocument* document);
	void					RemoveNotifyFileChanged(TDocument* document);

	inline Display*			GetDisplay() const { return fDisplay; }
	inline int				GetDefaultScreen() const { return fDefaultScreen; }
	inline Window			GetRootWindow() const { return  RootWindow(fDisplay, fDefaultScreen); }
	inline Window			GetLeaderWindow() const { return fLeaderWindow; }
	
	inline TCoord			GetScreenWidth() const { return DisplayWidth(fDisplay, fDefaultScreen); }
	inline TCoord			GetScreenHeight() const { return DisplayHeight(fDisplay, fDefaultScreen); }

	inline XIM				GetInputMethod() const { return fXIM; }

	inline TColor&			GetHiliteColor() { return fHiliteColor; }

	virtual void			NewDocument();
	virtual void			OpenWindowContext(TWindowContext* context, bool show = true);

	TChildProcess*			Execute(const char* commandLine, const char* workingDirectory, bool nonBlocking, bool useStdio);

	bool 					CheckForSignals();
	
	inline bool				HasPendingEvents() const { return XPending(fDisplay); }
	inline bool				InModalDialog() const { return (fModalDialog != NULL); }

protected:
	static bool				FindFileIdleProc(void* data);

	void					PollEvent(bool allowSleep = true);
	void					Idle();
	void					ComputeNextIdle();

	static void				SigChildHandler(int);

	virtual void			ChildDied(pid_t pid, int status);

	virtual void			HandleLeaderWindowEvent(XEvent& event);
	
	friend class TFileChangedNotificationIdler;
	void					DoCheckFileChangedNotification();

protected:
	TList<TDocumentType>	fDocumentTypes;
	CreateDocumentProc		fNewDocumentProc;
	int						fArgCount;
	char**					fArguments;
	bool					fDone;
	bool					fInSelect;
	int						fExitCode;
	
	TSettingsFile*			fSettingsFile;
	TString					fSettingsPath;
	
	// dialog support
	TDialogWindow*			fModalDialog;
	TCommandID				fDialogResult;
	
	TDialogWindow*			fFindFileCancelDialog;
	bool					fFindFileCancelled;
	
	// idling stuff
	TLinkedList<TIdler>		fIdlers;
	timeval					fStartTime;
	TTime					fNextIdle;
	
	// file changed notification
	TFileChangedNotificationIdler*	fFileChangedNotificationIdler;
	TList<TDocument>				fNotifyFileChangedList;

	// X11 Stuff
	Display*				fDisplay;
	int						fDefaultScreen;
	Window					fLeaderWindow;
	
	// XIM Stuff
	XIM						fXIM;

	TColor					fHiliteColor;

	// child processes
	friend class TChildProcess;
	TList<TChildProcess>	fChildren;
};


#endif // __TApplication__
