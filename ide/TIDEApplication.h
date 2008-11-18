// ========================================================================================
//	TIDEApplication.h		 	Copyright (C) 2001-2008 Mike Lockwood. All rights reserved.
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

#ifndef __TIDEApplication__
#define __TIDEApplication__

#include "fw/TApplication.h"

#ifdef ENABLE_DEBUGGER
#include "debugger/TTargetListener.h"
#include "debugger/TProcessListener.h"
#include "debugger/TThreadListener.h"

class TVariable;
class TStackFrame;
#endif // ENABLE_DEBUGGER

class TIDEApplication : public TApplication
#ifdef ENABLE_DEBUGGER
							, 
							 public TTargetListener,
							 public TProcessListener,
							 public TThreadListener
#endif // ENABLE_DEBUGGER				
{
public:
							TIDEApplication(int argc, char **argv);
	virtual 				~TIDEApplication(); 
	
	virtual void			Initialize();
	virtual void			ProcessArgument(int& argIndex);

	virtual TPixmap*		GetIcon();

	virtual void			DoSetupMenu(TMenu* menu);
	virtual bool			DoCommand(TCommandHandler* sender, TCommandHandler* receiver, TCommandID command);

	void					ShowAboutBox();
	void					NewProject();
	void					CloseAllTextWindows();
	void					ShowManPage();
	void					ShowManPage(const char* query);
	void					CompareFiles();

	virtual TDocument*		CreateDocument(TFile* file);

	virtual void			LoadSettings();
	virtual void			SaveSettings();
	virtual void			InitDefaultSettings();
	
	int						GetSpacesPerTab(const char* filename);

	void					DoDiff(const char* path1, const char* path2);

#ifdef ENABLE_DEBUGGER
	void					ClearAllBreakpoints();
	
	inline static TTarget*	GetLocalTarget() { return sLocalTarget; }
	
	void					MakeVariableWindow(TVariable* variable);
#endif // ENABLE_DEBUGGER

protected:
	virtual TFile*			FindFile(const char* fileName, TDocument* baseDocument = NULL, TDocument* baseDocument2 = NULL);

	virtual void			HandleLeaderWindowEvent(XEvent& event);

#ifdef ENABLE_DEBUGGER
	virtual void			ChildDied(pid_t pid, int status);

	virtual	void			ProcessCreated(TProcess* process);
	virtual	void			ThreadCreated(TThread* thread);
	virtual	void			ThreadSuspended(TThread* threads);

protected:	
	static TTarget*			sLocalTarget;	// for single machine debugging
#endif // ENABLE_DEBUGGER

protected:
	Atom					fOpenFileAtom;
	Atom					fLineNumberAtom;
};

inline TIDEApplication* GetIDEApplication() { return dynamic_cast<TIDEApplication*>(gApplication); }

#endif // __TIDEApplication__
