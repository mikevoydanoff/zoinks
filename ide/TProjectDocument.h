// ========================================================================================
//	TProjectDocument.h		 	Copyright (C) 2001-2002 Mike Lockwood. All rights reserved.
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

#ifndef __TProjectDocument__
#define __TProjectDocument__

#include "TLogDocumentOwner.h"
#include "fw/TBehavior.h"
#include "fw/TDocument.h"
#include "fw/TIdler.h"
#include "fw/TSettingsFile.h"
#include "fw/TString.h"

class TMenuBar;
class TWindow;
class TTextField;
class TCheckBox;
class TChildProcess;

#ifdef ENABLE_DEBUGGER
class TSymDocument;
#endif


class TProjectDocument : public TDocument, public TIdler, public TLogDocumentOwner
{
public:
	static TDocument*				CreateDocument(TFile* file);
	
	static inline TProjectDocument*	GetCurrentProject() { return sCurrentProject; } 
	static void						SetCurrentProject(TProjectDocument* project);

	void							Make();
	void							StopMake();
	void							Debug(bool makeFirst);

	inline bool						IsMaking() const { return (fMakeChild != NULL); }

#ifdef ENABLE_DEBUGGER
	inline TSymDocument*			GetSymDocument() const { return fSymDocument; }
	inline void						SetSymDocument(TSymDocument* document) { fSymDocument = document; }

	const TString&					GetDebugArguments() const { return fDebugArguments; }
	inline bool						UseExternalDebugger() const { return fUseExternalDebugger; }
#endif // ENABLE_DEBUGGER

	inline bool						MakeBeforeDebug() const { return fMakeBeforeDebug; }

protected:
									TProjectDocument(TFile* file);
	virtual							~TProjectDocument();

	static bool						IsProjectFile(TFile* file);

	virtual void					Open(TDocumentWindow* window);

	virtual void					ReadFromFile(TFile* file);
	virtual void					WriteToFile(TFile* file);

	virtual void					DoIdle();

	void							SetMakePath(const TChar* path);
	void							SetMakeCommand(const TChar* command);
	void							SetExternalDebuggerCommand(const TChar* command);

#ifdef ENABLE_DEBUGGER
	void							SetDebugPath(const TChar* path);
	void							SetDebugArguments(const TChar* arguments);
	void							SetUseExternalDebugger(bool useExternalDebugger);
#endif // ENABLE_DEBUGGER

	void							SetMakeBeforeDebug(bool makeBeforeDebug);

	virtual void					DoSetupMenu(TMenu* menu);
	virtual bool					DoCommand(TCommandHandler* sender, TCommandHandler* receiver, TCommandID command);

	TMenuBar*						MakeMenuBar(TWindow* window);

protected:
	// project settings
	TSettingsFile					fSettingsFile;
	TString							fMakePath;
	TString							fMakeCommand;
	TString							fExternalDebuggerCommand;
	
#ifdef ENABLE_DEBUGGER
	TString							fDebugPath;
	TString							fDebugArguments;
	bool							fUseExternalDebugger;
#endif // ENABLE_DEBUGGER

	bool							fMakeBeforeDebug;

	// for project settings window
	TTextField*						fMakePathText;
	TTextField*						fMakeCommandText;
	TTextField*						fExternalDebuggerCommandText;
	
#ifdef ENABLE_DEBUGGER
	TTextField*						fDebugPathText;
	TTextField*						fDebugArgumentsText;
	TCheckBox*						fUseExternalDebuggerBox;
#endif // ENABLE_DEBUGGER

	TCheckBox*						fMakeBeforeDebugBox;

	// for making
	TChildProcess*					fMakeChild;		// make in process
	TTime							fBuildStartTime;
	bool							fDebugAfterMake;

#ifdef ENABLE_DEBUGGER
	TSymDocument*					fSymDocument;
#endif

	static TProjectDocument*		sCurrentProject;
};


class TProjectBehavior : public TBehavior
{
public:
									TProjectBehavior(TProjectDocument* project);
	virtual							~TProjectBehavior();

	virtual void					DoSetupMenu(TMenu* menu);
	virtual bool					DoCommand(TCommandHandler* sender, TCommandHandler* receiver, TCommandID command);

private:
	TProjectDocument*				fProjectDocument;
};


#endif // __TProjectDocument__
