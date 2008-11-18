// ========================================================================================
//	TSettingsFile.h			 	Copyright (C) 2001-2007 Mike Lockwood. All rights reserved.
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

#ifndef __TSettingsFile__
#define __TSettingsFile__

#include "TList.h"
#include "TString.h"

class TFile;
class TColor;

class TSettingsFile
{
public:
						TSettingsFile(const char* fileSignature);
	virtual 			~TSettingsFile();

	// string settings
	const char*			GetStringSetting(const char* setting);
	void				SetStringSetting(const char* setting, const char* value);
	
	// boolean settings
	bool				GetBoolSetting(const char* setting, bool defaultValue);
	void				SetBoolSetting(const char* setting, bool value);
	
	// int settings
	int					GetIntSetting(const char* setting, int defaultValue);
	void				SetIntSetting(const char* setting, int value);
	
	// color settings
	bool				GetColorSetting(const char* setting, TColor& value);
	void				SetColorSetting(const char* setting, const TColor& value);

	void				ReadFromFile(TFile* file);
	void				WriteToFile(TFile* file);

protected:
	struct SettingItem
	{
		TString			setting;
		TString			value;
	};

	static int 			CompareByName(const SettingItem* item1, const SettingItem* item2);
	static int 			SearchByName(const SettingItem* item1, const char* name);

protected:
	TString				fSignature;
	TList<SettingItem>	fSettings;
};

#endif // __TSettingsFile__
