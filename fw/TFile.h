// ========================================================================================
//	TFile.h					 	Copyright (C) 2001-2002 Mike Lockwood. All rights reserved.
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

#ifndef __TFile__
#define __TFile__

#include "TString.h"
#include <stdio.h>

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#else
#include <time.h>
#endif

class TDirectory;

typedef time_t TFileTime;

class TFile
{
public:
							TFile();
							TFile(const char* path);
							TFile(const TDirectory& directory, const char* name);
							TFile(const TFile& file);
	virtual 				~TFile();

	const char*				GetPath() const;
	const char*				GetFileName() const;

	bool					IsDirectory() const;
	TDirectory*				GetDirectory() const;
	void					GetDirectory(TString& directoryPath) const;
	
	const char*				GetFileExtension() const;
	static const char*		GetFileExtension(const char* fileName);
	
	void					SetFileExtension(const char* extension);

	void					Specify(const char* path);
	void					Specify(const TFile& file);

	void					Open(bool readOnly, bool allowCreate, bool truncate);	// opens existing file
	void					CreateAndOpen();										// creates the file if necessary, then opens read/write
	void					Close();
		
	void					SetPosition(uint32 offset);	
	void					Read(void* buffer, uint32 length);
	void					Write(const void* buffer, uint32 length);
	
	void					GetFileTimes(TFileTime& lastAccess, TFileTime& lastModify, TFileTime& lastStatusChange);
	
	int						ReadLine(char* buffer, int length);

	bool					Exists();
	inline bool				IsSpecified() const { return fPath.GetLength() > 0; }
	uint32					GetFileSize();

	void*					Map(bool readOnly);
	void					Unmap();

	inline bool				IsOpen() const { return fFileDescriptor != 0; }
	inline int				GetFileDescriptor() const { return fFileDescriptor; }
	inline bool				IsMapped() const { return fMapData != NULL; }
	inline void*			GetMapData() const { return fMapData; }

	static void				NormalizePath(TString& path);
	static void				ComputeRelativePath(const char* path, const char* base, TString& relativePath);
	
	static bool				FilesAreEqual(const TFile& file1, const TFile& file2, bool ignoreBinaryFiles);

protected:
	TString					fPath;
	int						fFileDescriptor;
	int						fOpenFlags;				// flags used to open the file
	int						fPosition;
	void*					fMapData;
	uint32					fMapSize;
	bool					fReadOnly;
};

#endif // __TFile__
