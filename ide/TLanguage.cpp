// ========================================================================================
//	TLanguage.cpp		 	   Copyright (C) 2001-2003 Mike Voydanoff. All rights reserved.
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

#include "TLanguage.h"
#include "fw/TString.h"


ELanguage GetFileLanguage(const TChar* extension)
{
	ELanguage language = kLanguageNone;

	if (Tstrcasecmp(extension, "c") == 0)
	{
		language = kLanguageC;
	}
	else if (Tstrcasecmp(extension, "cc") == 0 ||
			 Tstrcasecmp(extension, "cp") == 0 ||
			 Tstrcasecmp(extension, "cpp") == 0 ||
			 Tstrcasecmp(extension, "h") == 0 ||
			 Tstrcasecmp(extension, "hpp") == 0)
	{
		language = kLanguageCPlusPlus;
	}
	else if (Tstrcasecmp(extension, "java") == 0)
	{
		language = kLanguageJava;
	}
	else if (Tstrcasecmp(extension, "html") == 0 || 
			 Tstrcasecmp(extension, "htm") == 0 ||
			 Tstrcasecmp(extension, "shtml") == 0)
	{
		language = kLanguageHTML;
	}
	else if (Tstrcasecmp(extension, "rb") == 0 ||
	         Tstrcasecmp(extension, "rbx") == 0)
	{
		language = kLanguageRuby;
	}			         
	else if (Tstrcasecmp(extension, "py") == 0)
	{
		language = kLanguagePython;
	}			         
	else if (Tstrcasecmp(extension, "tex") == 0)
	{
		language = kLanguageTeX;
	}

	return language;
}
