// ========================================================================================
//	TDirectoryDiffDocument.cpp		 Copyright (C) 2003 Mike Lockwood. All rights reserved.
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

#include "TDirectoryDiffDocument.h"
#include "TDirectoryDiffListView.h"
#include "TProjectCommands.h"

#include "fw/TApplication.h"
#include "fw/TCommandID.h"
#include "fw/TCommonDialogs.h"
#include "fw/TDocumentWindow.h"
#include "fw/TFile.h"
#include "fw/TMenuBar.h"
#include "fw/TScroller.h"
#include "fw/TWindowsMenu.h"
#include "fw/TWindowPositioners.h"

#include "fw/intl.h"

#include <X11/keysym.h>
#include <stdlib.h>
#include <limits.h>


static TMenuItemRec sFileMenu[] = 
{
	{ N_("New"), kNewCommandID, Mod1Mask, 'n' },
	{ N_("New Project"), kNewProjectCommandID, Mod1Mask|ShiftMask, 'n' },
	{ "-" },
	{ N_("Open..."), kOpenCommandID, Mod1Mask, 'o' },
	{ N_("Find File..."), kFindFileCommandID, Mod1Mask, 'd' },
	{ "-" },
	{ N_("Compare Files..."), kCompareFilesCommandID },
	{ "-" },
	{ N_("Close"), kCloseCommandID, Mod1Mask, 'w' },
	{ N_("Quit"), kQuitCommandID, Mod1Mask, 'q' },
	{ "" }
};

static TMenuItemRec sWindowsMenu[] =
{
	{ N_("About Zoinks..."), kAboutCommandID },
	{ "-" },
	{ "" }
};


TDirectoryDiffDocument::TDirectoryDiffDocument(const TDirectory& directory1, const TDirectory& directory2)
	:	TWindowContext(gApplication),
		fDirectory1(directory1),
		fDirectory2(directory2)
{
	gApplication->AddSubContext(this);
}


TDirectoryDiffDocument::~TDirectoryDiffDocument()
{
	fDiffList.DeleteAll();
}


TDirectoryDiffDocument* TDirectoryDiffDocument::CreateDocument(const TDirectory& directory1, const TDirectory& directory2)
{
	return new TDirectoryDiffDocument(directory1, directory2);
}


void TDirectoryDiffDocument::Open(TDocumentWindow* window)
{
	TMenuBar* menuBar = MakeMenuBar(window);
	menuBar->SetWindowPositioner(WidthParent);

	TRect bounds;
	window->GetLocalBounds(bounds);
	bounds.top += menuBar->GetHeight();
	
	TScroller* scroller = new TScroller(window, bounds, true, true);
	scroller->SetWindowPositioner(SizeRelativeParent);
	
	fDirectoryDiffListView = new TDirectoryDiffListView(scroller, bounds, fDirectory1, fDirectory2, fDiffList);
	scroller->SetContainedView(fDirectoryDiffListView);
	window->SetTarget(fDirectoryDiffListView);
	window->SetWindowContext(this);

	ComputeDiffs();
	fDirectoryDiffListView->GenerateDiffTree();
}


void TDirectoryDiffDocument::GetTitle(TString& title) const
{
	char buffer[PATH_MAX + 100];
	
	sprintf(buffer, _("Compare Directories: %s, %s"), fDirectory1.GetDirectoryName(), fDirectory2.GetDirectoryName());
	title = buffer;
}


void TDirectoryDiffDocument::DoCompareDirectories(TDirectory& directory1, TDirectory& directory2)
{
	directory1.Open();
	directory2.Open();
	
	// iterate through directory 1
	int	count = directory1.GetFileCount();
	for (int i = 0; i < count; i++)
	{
		TString	name;
		bool	isDirectory;
		
		directory1.GetFile(i, name, isDirectory);
		if (isDirectory)
		{
			// ignore CVS, subversion and directories
			if (Tstrcmp(name, "CVS") != 0 && Tstrcmp(name, ".svn") != 0 && Tstrcmp(name, ".git") != 0)
			{
				TDirectory	subDir2(directory2, name);
				
				if (subDir2.Exists())
				{
					TDirectory	subDir1(directory1, name);
					DoCompareDirectories(subDir1, subDir2);
				}
			}
		}
		else
		{
			TFile	file2(directory2, name);
			
			if (file2.Exists())
			{
				TFile	file1(directory1, name);
				
				// ignore class files and compiled python files
				const char* extension = file1.GetFileExtension();
				if (Tstrcmp(extension, "class") != 0 && Tstrcmp(extension, "pyc") != 0)
				{
					if (!TFile::FilesAreEqual(file1, file2, true))
					{
						TString* path = new TString;;
						
						TFile::ComputeRelativePath(file1.GetPath(), fDirectory1.GetPath(), *path);
						fDiffList.InsertLast(path);
					}
				}
			}
		}
	}

	directory1.Close();
	directory2.Close();
}


void TDirectoryDiffDocument::ComputeDiffs()
{
	DoCompareDirectories(fDirectory1, fDirectory2);
}


TMenuBar* TDirectoryDiffDocument::MakeMenuBar(TWindow* window)
{
	TMenuBar* menuBar = new TMenuBar(window, TRect(0, 0, window->GetWidth(), 20));

	menuBar->AddMenu(_("File"), sFileMenu);
	TWindowsMenu* windowsMenu = new TWindowsMenu(_("Windows"), sWindowsMenu);
	menuBar->AddMenu(windowsMenu);

	return menuBar;
}


void TDirectoryDiffDocument::DoSetupMenu(TMenu* menu)
{
	TWindowContext::DoSetupMenu(menu);
}


bool TDirectoryDiffDocument::DoCommand(TCommandHandler* sender, TCommandHandler* receiver, TCommandID command)
{
	return TWindowContext::DoCommand(sender, receiver, command);
}
