// ========================================================================================
//	TSettingsFile.cpp		 	Copyright (C) 2001-2007 Mike Lockwood. All rights reserved.
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

#include "TSettingsFile.h"
#include "TColor.h"
#include "TFile.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>


TSettingsFile::TSettingsFile(const char* fileSignature)
	:	fSignature(fileSignature),
		fSettings((TListBase::CompareFunc)CompareByName, (TListBase::SearchFunc)SearchByName)
{
}


TSettingsFile::~TSettingsFile()
{
	TListIterator<SettingItem>	iter(fSettings);
	SettingItem* item;
	
	while ((item = iter.Next()) != NULL)
		delete item;
	fSettings.RemoveAll();
}


const char* TSettingsFile::GetStringSetting(const char* setting)
{
	SettingItem* item = fSettings.Search(setting);
	if (item)
		return item->value;
	else
		return NULL;
}


void TSettingsFile::SetStringSetting(const char* setting, const char* value)
{
	SettingItem* item = fSettings.Search(setting);

	if (item)
	{
		if (value)
			item->value = value;
		else
			fSettings.Remove(item);
	}
	else if (value)
	{
		item = new SettingItem;
		item->setting = setting;
		item->value = value;
		fSettings.Insert(item);
	}
}


bool TSettingsFile::GetBoolSetting(const char* setting, bool defaultValue)
{
	const char* value = GetStringSetting(setting);
	
	if (value)
		return (strcasecmp(value, "true") == 0);
	else
		return defaultValue;
}


void TSettingsFile::SetBoolSetting(const char* setting, bool value)
{
	SetStringSetting(setting, (value ? "true" : "false"));
}


int	TSettingsFile::GetIntSetting(const char* setting, int defaultValue)
{
	const char* value = GetStringSetting(setting);
	
	if (value == NULL)
		return defaultValue;
	return atoi(value);
}


void TSettingsFile::SetIntSetting(const char* setting, int value)
{
	char	buffer[20];
	sprintf(buffer, "%d", value);
	SetStringSetting(setting, buffer);	
}


bool TSettingsFile::GetColorSetting(const char* setting, TColor& value)
{
	const char* valueString = GetStringSetting(setting);
	int red, green, blue;
	
	if (valueString && sscanf(valueString, "(%d,%d,%d)", &red, &green, &blue) == 3)
	{
		value.Set(red, green, blue);
		return true;
	}
	else
		return false;
}


void TSettingsFile::SetColorSetting(const char* setting, const TColor& value)
{
	char	buffer[20];
	
	sprintf(buffer, "(%d,%d,%d)", value.Red(), value.Green(), value.Blue());
	SetStringSetting(setting, buffer);
}


void TSettingsFile::ReadFromFile(TFile* file)
{
	char	line[4000];

	bool firstLine = true;
	while (file->ReadLine(line, sizeof(line)) > 0)
	{
		int length = strlen(line);
		if (length > 0 && line[length - 1] == '\n')
			line[length - 1] = 0;

		if (firstLine)
		{
// temporarily disabled to deal with project file backward compatibility
//			int signatureCompare = strcmp(line, fSignature);
//			ASSERT(signatureCompare == 0);
			firstLine = false;
		}
		else
		{
			char* equals = strchr(line, '=');

			if (equals)
			{
				*equals++ = 0;
				SetStringSetting(line, equals);
			}
		}
	}
}


void TSettingsFile::WriteToFile(TFile* file)
{
	TListIterator<SettingItem>	iter(fSettings);
	SettingItem* item;
	char	buffer[4000];

	snprintf(buffer, sizeof(buffer), "%s\n", (const char *)fSignature);
	file->Write(buffer, strlen(buffer));

	while ((item = iter.Next()) != NULL)
	{
		snprintf(buffer, sizeof(buffer), "%s=%s\n", (const char *)item->setting, (const char *)item->value);
		file->Write(buffer, strlen(buffer));
	}
}


int TSettingsFile::CompareByName(const SettingItem* item1, const SettingItem* item2)
{
	return strcmp(item1->setting, item2->setting);
}


int TSettingsFile::SearchByName(const SettingItem* item1, const char* name)
{
	return strcmp(item1->setting, name);
}
