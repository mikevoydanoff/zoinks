// ========================================================================================
//	IDEMain.cpp				   Copyright (C) 2001-2002 Mike Voydanoff. All rights reserved.
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
#include "TProjectDocument.h"
#include "TTextDocument.h"
#include "fw/TTextView.h"
#include "fw/TWindowPositioners.h"
#include "fw/TScroller.h"
#include "fw/TFont.h"
#include "fw/TFile.h"
#include "fw/TCommandID.h"

#ifdef HAVE_LIBIMLIB
#include <Imlib.h>

ImlibData* gImlibData = NULL;
#endif // HAVE_LIBIMLIB


int main(int argc, char* argv[])
{
	TIDEApplication		application(argc, argv);
	application.Initialize();
//	application.RegisterFileType("proj", TProjectDocument::CreateDocument);
	application.RegisterFileType("", TTextDocument::CreateDocument);
	application.SetNewDocumentProc(TTextDocument::CreateDocument);
	
	application.CreateSettingsFile(".zoinks");

#ifdef HAVE_LIBIMLIB
	gImlibData = Imlib_init(application.GetDisplay());
	ASSERT(gImlibData);
#endif // HAVE_LIBIMLIB

	int result = application.Run();
	
	application.SaveSettings();
	
	return result;
}
