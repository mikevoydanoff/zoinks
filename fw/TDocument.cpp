// ========================================================================================
//	TDocument.cpp			   Copyright (C) 2001-2009 Mike Voydanoff. All rights reserved.
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

#include "TDocument.h"
#include "TDocumentWindow.h"
#include "TApplication.h"
#include "TMenu.h"
#include "TCommonDialogs.h"
#include "TCommandID.h"

#include "intl.h"

#include <limits.h>


TDocument::TDocument(TFile* file)
	:	TWindowContext(gApplication),
		fModified(false),
		fLastModification(0)
{
	if (file)
	{
		fFile.Specify(*file);
		fTitle = file->GetFileName();
	}
	else
		gApplication->GetUntitledDocumentTitle(fTitle);

	gApplication->AddSubContext(this);
}


TDocument::~TDocument()
{
	gApplication->RemoveSubContext(this);
}


void TDocument::GetTitle(TString& title) const
{
	title = fTitle;
}


void TDocument::Open(TDocumentWindow* window)
{
	TWindowContext::Open(window);

	if (fFile.IsSpecified())
	{
		if (fFile.Exists())
		{
			ReadFromFile(&fFile);
			UpdateFileModification();
		}

		SetTitle(fFile.GetFileName());
	}
}


void TDocument::Save()
{
	if (! fFile.IsSpecified())
	{
		TFile* file = TCommonDialogs::SaveFile(NULL, GetMainWindow());
			
		if (!file)
			return; // mgl should use exceptions?

		WriteToFile(file);
		SetFile(file);
		delete file;
	}
	else
		WriteToFile(&fFile);
		
	UpdateFileModification();

	SetModified(false);
}


void TDocument::SaveAs(bool makeCopy)
{
	TFile* file = TCommonDialogs::SaveFile(NULL, GetMainWindow());
	
	if (file)
	{
		if (! makeCopy)
			SetFile(file);

		WriteToFile(file);
		UpdateFileModification();

		if (! makeCopy)
			SetModified(false);
		
		delete file;
	}
}


void TDocument::ReadFromFile(TFile* /*file*/)
{
}


void TDocument::WriteToFile(TFile* /*file*/)
{
}


void TDocument::CheckFileModification()
{
	static bool reentrancyCheck = false;

	if (fFile.IsSpecified() && fFile.Exists())
	{
		TFileTime	access, modification, statusChange;
		fFile.GetFileTimes(access, modification, statusChange);

		if (fLastModification != modification && !reentrancyCheck)
		{
			char	prompt[PATH_MAX + 100];
			sprintf(prompt, _("The file \"%s\" has been modified by a different program.  Do you want to reload it?"), fFile.GetFileName());
				
			reentrancyCheck = true;
			
			try
			{
				if (TCommonDialogs::ConfirmDialog(prompt, _("File Modified"), NULL))
				{
					ReadFromFile(&fFile);
					SetModified(false);
					UpdateFileModification();
				}
				else
					fLastModification = modification;
			}
			catch (...)
			{
				// make sure reentrancy check is cleared
				reentrancyCheck = false;
				// set fLastModification so we don't try again
				fLastModification = modification;
				throw;
			}
				
			reentrancyCheck = false;
		}
	}
}


void TDocument::UpdateFileModification()
{
	TFileTime	access, statusChange;
	fFile.GetFileTimes(access, fLastModification, statusChange);
}


void TDocument::AddWindow(TTopLevelWindow* window)
{
	TWindowContext::AddWindow(window);
	
	if (fTitle.GetLength() > 0 && window == GetMainWindow())
		window->SetTitle(fTitle);
}


void TDocument::SetFile(TFile* file)
{
	ASSERT(file);

	fFile = *file;

	if (file->IsSpecified())
	{
		SetTitle(file->GetFileName());
		
		TTopLevelWindow* window = GetMainWindow();
		if (window)
			window->SetTitle(fTitle);
	}
	
	TTopLevelWindow* window = GetMainWindow();
	if (window && window->GetTarget())
		window->GetTarget()->HandleCommand(this, this, kFilePathChangedCommandID);
	else
		HandleCommand(this, this, kFilePathChangedCommandID);
}


void TDocument::SetTitle(const TChar* title)
{
	fTitle = title;
	
	TTopLevelWindow* window = GetMainWindow();
	if (window)
		window->SetTitle(title);
}


bool TDocument::AllowClose()
{
	if (IsModified())
	{
		bool save;

		if (!TCommonDialogs::SaveChanges(fTitle, GetMainWindow(), save))
			return false;

		if (save)
		{	
	 		// save the file
	 		if (fFile.IsSpecified())
				Save();
			else
			{
				TFile* file = TCommonDialogs::SaveFile(NULL, GetMainWindow());
				
				if (file)
				{
					SetFile(file);
					Save();
					delete file;
				}
				else
					return false;
			}
		}
	}

	return true;
}


bool TDocument::IsModified() const
{
	return fModified;
}


void TDocument::ShowLine(int line)
{
}


void TDocument::DoSetupMenu(TMenu* menu)
{
	if (IsModified() || !fFile.IsSpecified())
		menu->EnableCommand(kSaveCommandID);

	menu->EnableCommand(kSaveAsCommandID);
	menu->EnableCommand(kSaveCopyCommandID);

	TWindowContext::DoSetupMenu(menu);
}


bool TDocument::DoCommand(TCommandHandler* sender, TCommandHandler* receiver, TCommandID command)
{
	switch (command)
	{
		case kSaveCommandID:
			Save();
			return true;

		case kSaveAsCommandID:
			SaveAs(false);
			return true;

		case kSaveCopyCommandID:
			SaveAs(true);
			return true;

		default:
			return TWindowContext::DoCommand(sender, receiver, command);
	}
}
