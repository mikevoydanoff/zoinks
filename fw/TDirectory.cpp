// ========================================================================================
//	TDirectory.cpp			 	Copyright (C) 2001-2002 Mike Lockwood. All rights reserved.
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

#include "TDirectory.h"
#include "TFile.h"
#include "TException.h"

#include <sys/stat.h>
#include <stdio.h>
#include <limits.h>


TDirectory::TDirectory()
	:	fDir(NULL),
		fFileCount(0)
{
}


TDirectory::TDirectory(const char* path)
	:	fPath(path),
		fDir(NULL),
		fFileCount(0)
{
	TFile::NormalizePath(fPath);
	
	// make sure path ends in a '/'
	if (fPath[fPath.GetLength() - 1] != '/')
		fPath += '/';
}


TDirectory::TDirectory(const TDirectory& directory, const char* name)
	:	fPath(directory.GetPath()),
		fDir(NULL),
		fFileCount(0)
{
	fPath += name;
	TFile::NormalizePath(fPath);

	// make sure path ends in a '/'
	if (fPath[fPath.GetLength() - 1] != '/')
		fPath += '/';
}



TDirectory::TDirectory(const TDirectory& directory)
	:	fPath(directory.fPath),
		fDir(NULL),
		fFileCount(0)
{
}


TDirectory::~TDirectory()
{
	Close();
}


const char* TDirectory::GetPath() const
{
	return fPath;
}


const char* TDirectory::GetDirectoryName() const
{
	const char* directoryName = fPath;
	const char* lastSlash = NULL;

	while (directoryName[0])
	{
		if (*directoryName++ == '/' && *directoryName)		// don't count last slash
			lastSlash = directoryName;
	}

	if (lastSlash)
		return lastSlash;
	else
		return fPath;
}


void TDirectory::Specify(const char* path)
{
	ASSERT(! IsOpen());

	fPath = path;
}


void TDirectory::Specify(const TDirectory& directory)
{
	Specify(directory.fPath);
}


void TDirectory::Open()
{
	if (fDir)
		return;	// already opened

	char	path[PATH_MAX + 1];
	fPath.CopyTo(path, PATH_MAX);

	fDir = opendir(path);
	if (!fDir)
		ThrowSystemError();
}


void TDirectory::Close()
{
	if (fDir)
	{
		closedir(fDir);
		fDir = NULL;
	}
}


int TDirectory::GetFileCount()
{
	if (fFileCount == 0)
	{
		dirent* 	entry;

		Reset();

		while ((entry = DoReadDir()) != NULL)
		{
			fCurrentIndex++;
			fFileCount++;
		}
	}

	return fFileCount;
}


void TDirectory::Reset()
{
	rewinddir(fDir);
	fCurrentIndex = 0;
}


void TDirectory::GetFile(int index, TString& name, bool& isDirectory)
{
	ASSERT(index >= 0 && index < GetFileCount());
	
	if (index < fCurrentIndex)
		Reset();

	dirent* 	entry = NULL;
	struct stat statbuf;
	
	for (int i = fCurrentIndex; i <= index; i++)
	{
		entry = DoReadDir();
		fCurrentIndex++;
	}

	ASSERT(entry);

	name = entry->d_name;

	TString path(fPath);
	path += name;
	stat(path, &statbuf);
	isDirectory = S_ISDIR(statbuf.st_mode);
}


TFile* TDirectory::GetFile(int index)
{
	TString	name;
	bool	isDirectory;

	GetFile(index, name, isDirectory);

	if (!isDirectory)
	{
		TFile* file = new TFile(*this, name);
		return file;
	}

	return NULL;
}


TDirectory* TDirectory::GetDirectory(int index)
{
	TString	name;
	bool	isDirectory;

	GetFile(index, name, isDirectory);

	if (isDirectory)
	{
		TDirectory* directory = new TDirectory(*this, name);
		return directory;
	}

	return NULL;
}


TDirectory* TDirectory::GetParent()
{
	return new TDirectory(*this, "../");
}


static TFile* DoFindFile(const char* fileName, TString& path, bool recurse, TDirectory::IdleProc idleProc, void* data)
{
	TFile* result = NULL;
	int pathLength = path.GetLength();

	if (fileName[0] == '/')
	{
		// file is absolute path
		path = fileName;
		recurse = false;
	}
	else	
		path += fileName;

	struct stat 	statBuf;
	int statResult = stat(path, &statBuf);

	if (statResult == 0 && !S_ISDIR(statBuf.st_mode))
		return new TFile(path);

	if (recurse)
	{
		path.Truncate(pathLength);
		DIR* dir = NULL;
		dir = opendir(path);
		ASSERT(dir);

		try
		{
			while (!result)
			{
				dirent* entry = readdir(dir);
	
				if (!entry)
					break;
				else if ((strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0))
					continue;
	
				path.Replace(pathLength, path.GetLength() - pathLength, entry->d_name);
				int statResult = stat(path, &statBuf);
				
				// stat may fail due to a bad symlink
				if (statResult == 0)
				{
					if (S_ISDIR(statBuf.st_mode))
					{
						path += "/";
						result = DoFindFile(fileName, path, true, idleProc, data);
					}
				}
				
				if (idleProc)
				{
					bool cancel = idleProc(data);
					if (cancel)
						ThrowUserCancelled();
				}
			}
		}
		catch (...)
		{
			closedir(dir);	
			throw;
		}

		closedir(dir);
	}

	return result;
}


TFile* TDirectory::FindFile(const char* fileName, bool recurse, IdleProc idleProc, void* data) const
{
	TString path(fPath);
	return DoFindFile(fileName, path, recurse, idleProc, data);
}


bool TDirectory::Exists()
{
	if (IsOpen())
		return true;

	char	path[PATH_MAX + 1];
	fPath.CopyTo(path, PATH_MAX);

	struct stat 	statBuf;
	return (stat(path, &statBuf) == 0);
}


dirent* TDirectory::DoReadDir()
{
	// suppress "." and ".."
	dirent* entry;

	do 
	{
		entry = readdir(fDir);
	}
	while (entry && (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0));

	return entry;
}

