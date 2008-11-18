// ========================================================================================
//	TDirectory.h		 		Copyright (C) 2001-2002 Mike Lockwood. All rights reserved.
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

#ifndef __TDirectory__
#define __TDirectory__

#include "TString.h"

#ifdef HAVE_INTTYPES_H
// needed for dirent.h on MacOS X
#include <inttypes.h>
#endif

#include <dirent.h>

class TFile;


class TDirectory
{
public:
					TDirectory();
					TDirectory(const char* path);
					TDirectory(const TDirectory& directory, const char* name);
					TDirectory(const TDirectory& directory);
	virtual 		~TDirectory();

	const char*		GetPath() const;
	const char*		GetDirectoryName() const;

	void			Specify(const char* path);
	void			Specify(const TDirectory& directory);

	void			Open();
	void			Close();

	int				GetFileCount();
	void			Reset();
	
	void			GetFile(int index, TString& name, bool& isDirectory);
	TFile*			GetFile(int index);			// returns NULL if directory
	TDirectory*		GetDirectory(int index);	// returns NULL if not directory
	TDirectory*		GetParent();

	typedef bool	(* IdleProc)(void* data);
	TFile*			FindFile(const char* fileName, bool recurse, IdleProc idleProc = NULL, void* data = NULL) const;

	bool			Exists();
	inline bool		IsSpecified() const { return fPath.GetLength() > 0; }

	inline bool		IsOpen() const { return fDir != NULL; }

protected:
	dirent* 		DoReadDir();

protected:
	TString			fPath;
	DIR*			fDir;
	int				fFileCount;
	int				fCurrentIndex;
};

#endif // __TDirectory__
