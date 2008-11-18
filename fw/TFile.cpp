// ========================================================================================
//	TFile.cpp				 	Copyright (C) 2001-2002 Mike Lockwood. All rights reserved.
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

#include "TFile.h"
#include "TDirectory.h"
#include "TException.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>
#include <string.h>


TFile::TFile()
	:	fFileDescriptor(0),
		fOpenFlags(0),
		fPosition(0),
		fMapData(NULL),
		fMapSize(0),
		fReadOnly(false)
{
}


TFile::TFile(const char* path)
	:	fPath(path),
		fFileDescriptor(0),
		fOpenFlags(0),
		fPosition(0),
		fMapData(NULL),
		fMapSize(0),
		fReadOnly(false)
{
	NormalizePath(fPath);
}


TFile::TFile(const TDirectory& directory, const char* name)
	:	fPath(directory.GetPath()),
		fFileDescriptor(0),
		fOpenFlags(0),
		fPosition(0),
		fMapData(NULL),
		fMapSize(0),
		fReadOnly(false)
{
	fPath += name;
	NormalizePath(fPath);
}


TFile::TFile(const TFile& file)
	:	fPath(file.fPath),
		fFileDescriptor(0),
		fOpenFlags(0),
		fPosition(0),
		fMapData(NULL),	
		fReadOnly(false)
{
}


TFile::~TFile()
{
	Unmap();
	Close();
}


const char* TFile::GetPath() const
{
	return fPath;
}


const char* TFile::GetFileName() const
{
	const char* fileName = fPath;
	const char* lastSlash = NULL;

	while (fileName[0])
	{
		if (*fileName++ == '/')
			lastSlash = fileName;
	}

	if (lastSlash)
		return lastSlash;
	else
		return fPath;
}


bool TFile::IsDirectory() const
{
	struct stat 	statBuf;

	int result = stat(fPath, &statBuf);
	return (result == 0 && S_ISDIR(statBuf.st_mode));
}


TDirectory* TFile::GetDirectory() const
{
	if (!IsSpecified())
		return NULL;

	TString	path;
	GetDirectory(path);
	return new TDirectory(path);
}


void TFile::GetDirectory(TString& directoryPath) const
{
	ASSERT(IsSpecified());
		
	const char* fileName = fPath;
	const char* str = fileName;
	const char* lastSlash = NULL;

	while (str[0])
	{
		if (*str++ == '/')
			lastSlash = str;
	}

	ASSERT(lastSlash);
	directoryPath.Set(fileName, lastSlash - fileName);
}


const char* TFile::GetFileExtension() const
{
	return GetFileExtension(GetFileName());
}


const char* TFile::GetFileExtension(const char* fileName)
{
	const char* lastDot = NULL;

	while (fileName[0])
	{
		if (*fileName++ == '.')
			lastDot = fileName;
	}

	if (lastDot)
		return lastDot;
	else
		return kEmptyString;
}


void TFile::SetFileExtension(const char* extension)
{
	const char* oldExtension = GetFileExtension();

	if (oldExtension && oldExtension[0])
		fPath.Truncate(oldExtension - fPath);
	else
		fPath += ".";
		
	fPath += extension;
}


void TFile::Specify(const char* path)
{
	ASSERT(! IsOpen());

	fPath = path;
}


void TFile::Specify(const TFile& file)
{
	Specify(file.fPath);
}


void TFile::Open(bool readOnly, bool allowCreate, bool truncate)
{
	if (fFileDescriptor)
		return;	// already opened

	int flags = (readOnly ? O_RDONLY : O_RDWR);
	fOpenFlags = flags;
	if (truncate && !readOnly)
		flags |= O_TRUNC;

	int fd = open(fPath, flags);
	
	if (fd < 0)
	{
		if (allowCreate && errno == ENOENT)
		{
			flags |= O_CREAT;
			fd = open(fPath, flags, 0644);
			
			if (fd < 0)
				ThrowSystemError();
		}
		else
			ThrowSystemError();
	}
	
	fFileDescriptor = fd;
	fReadOnly = readOnly;
}


void TFile::CreateAndOpen()
{
	if (fFileDescriptor)
		return;	// already opened

	int flags = O_RDWR | O_CREAT | O_TRUNC;
	fOpenFlags = O_RDWR;
	int fd = open(fPath, flags, 0644);
	if (fd < 0)
		ThrowSystemError();
	
	fFileDescriptor = fd;
	fReadOnly = false;
}


void TFile::Close()
{
	if (fFileDescriptor)
	{
		close(fFileDescriptor);
		fFileDescriptor = 0;
		
		// don't fail on close
//		ThrowSystemError();
	}
}


void TFile::SetPosition(uint32 offset)
{
	int result = lseek(fFileDescriptor, offset, SEEK_SET);
	if (result < 0)
		ThrowSystemError();
	fPosition = result;
}


void TFile::Read(void* buffer, uint32 length)
{
	int result = read(fFileDescriptor, buffer, length);
	if (result < 0)
		ThrowSystemError();
}


void TFile::Write(const void* buffer, uint32 length)
{
	int result = write(fFileDescriptor, buffer, length); 
	if (result < 0)
	{
		if (errno == EBADF)
		{
			// perhaps when we originally opened the file we could only get read access.
			// try to open again read/write
			close(fFileDescriptor);
			result = open(fPath, fOpenFlags);
			if (result < 0)
				ThrowSystemError();
			fFileDescriptor = result;
			
			SetPosition(fPosition);

			result = write(fFileDescriptor, buffer, length); 
			if (result < 0)
				ThrowSystemError();			
		}
		else
			ThrowSystemError();
	}
}


void TFile::GetFileTimes(TFileTime& lastAccess, TFileTime& lastModify, TFileTime& lastStatusChange)
{
	ASSERT(IsSpecified());

	struct stat 	statBuf;

	int result = stat(fPath, &statBuf);
	if (result < 0)
		ThrowSystemError();
		
	lastAccess = statBuf.st_atime;
	lastModify = statBuf.st_mtime;
	lastStatusChange = statBuf.st_ctime;
}


int TFile::ReadLine(char* buffer, int length)
{
	ASSERT(length > 1);
	int result = read(fFileDescriptor, buffer, length - 1);
	
	if (result > 0)
	{
		ASSERT(result < length);
		buffer[result] = 0;
		
		for (int i = 0; i < result; i++)
		{
			if (buffer[i] == '\n' || buffer[i] == '\r')
			{
				buffer[i] = 0;
				int err = lseek(fFileDescriptor, i - result + 1, SEEK_CUR);
				if (err < 0)
					ThrowSystemError();
				break;
			}
		}
	}
	else if (result < 0)
		ThrowSystemError();
		
	return result;
}


bool TFile::Exists()
{
	if (IsOpen())
		return true;

	struct stat 	statBuf;
	return (stat(fPath, &statBuf) == 0);
}


uint32 TFile::GetFileSize()
{
	struct stat 	statBuf;
	if (stat(fPath, &statBuf) == 0)
		return statBuf.st_size;
	else
		return 0;
}
 

void* TFile::Map(bool readOnly)
{
	Open(readOnly, false, false);
	
	fMapSize = GetFileSize();
	fMapData = mmap(NULL, fMapSize, (readOnly ? PROT_READ : PROT_READ | PROT_WRITE),  MAP_SHARED, fFileDescriptor, 0);
	return fMapData;
}


void TFile::Unmap()
{
	if (fMapData)
		munmap((char *)fMapData, fMapSize);
		
	Close();
}


void TFile::NormalizePath(TString& path)
{
	if (path[0] == '.' && (path.GetLength() == 1 || path[1] == '/'))
	{
		char	cwd[PATH_MAX + 2];
		getcwd(cwd, PATH_MAX);
		path.Replace(0, 1, cwd);	// replace '.' with cwd
	}
	else if (path[0] != '/')
	{
		char	cwd[PATH_MAX + 2];
		getcwd(cwd, PATH_MAX);
		strcat(cwd, "/");
		path.Replace(0, 0, cwd);	// prepend cwd + '/' before the file name
	}
	// first remove all "/./" within the path
	const char* subStr = strstr(path, "/./");

	while (subStr)
	{
		int offset = subStr - (const char *)path;
		ASSERT(offset < path.GetLength());

		path.Replace(offset, 3, "/");
		
		subStr = strstr(path, "/./");
	}

	// then remove all "../" within the path
	
	subStr = strstr(path, "/../");

	while (subStr)
	{
		int offset = subStr - (const char *)path;
		ASSERT(offset < path.GetLength());

		if (offset == 0)
		{
			// reduce leading "/../" to "/"
			path.Replace(0, 3, "");
		}
		else
		{
			int previousSlash = -1;
			for (int i = offset - 2; i >= 0; i--)
			{
				if (path[i] == '/')
				{
					previousSlash = i;
					break;
				}
			}

			if (previousSlash >= 0)
				path.Replace(previousSlash, offset + 3 - previousSlash, "");
		}
		
		subStr = strstr(path, "/../");
	}
}


void TFile::ComputeRelativePath(const char* path, const char* base, TString& relativePath)
{
	ASSERT(base[0] == '/');		// base must be absolute
	ASSERT(path[0] == '/');		// path must be absolute
	
	const char* lastBaseSlash = NULL;
	const char* temp = base;
	while (*temp)
	{
		if (*temp == '/')
			lastBaseSlash = temp;
		temp++;
	}
	
	const char* lastSlash = NULL;
	while (*path && *path == *base && base <= lastBaseSlash)
	{
		if (*path == '/')
			lastSlash = path + 1;
		path++;
		base++;
	}

	if (lastSlash)
		path = lastSlash;

	relativePath.SetEmpty();

	while (base <= lastBaseSlash)
	{
		if (*base == '/')
			relativePath += "../";
		base++;
	}

	relativePath += path;

	if (relativePath.IsEmpty())
		relativePath = "./";
}


bool TFile::FilesAreEqual(const TFile& file1, const TFile& file2, bool ignoreBinaryFiles)
{
	struct stat 	statBuf1, statBuf2;

	int result = stat(file1.fPath, &statBuf1);
	if (result < 0)
		ThrowSystemError();
	result = stat(file2.fPath, &statBuf2);
	if (result < 0)
		ThrowSystemError();

	if (!ignoreBinaryFiles && statBuf1.st_size != statBuf2.st_size)
		return false;
	if (statBuf1.st_size == 0)
		return true;

	// open files and compare contents
	char	buffer1[1000], buffer2[1000];
	off_t	length = statBuf1.st_size;

	int fd1 = open(file1.fPath, O_RDONLY);
	int fd2 = open(file2.fPath, O_RDONLY);

	bool beginning = true;

	while (length > 0)
	{	
		int result1 = read(fd1, buffer1, sizeof(buffer1));
		if (result1 < 0)
			ThrowSystemError();
		int result2 = read(fd2, buffer2, sizeof(buffer2));
		if (result2 < 0)
			ThrowSystemError();
		
		if (ignoreBinaryFiles)
		{
			if (beginning)
			{
				// for now, just detect ELF files
				if ((buffer1[0] == 0x7F && buffer1[1] == 'E' && buffer1[2] == 'L' && buffer1[3] == 'F') ||
					(buffer2[0] == 0x7F && buffer2[1] == 'E' && buffer2[2] == 'L' && buffer2[3] == 'F'))
					goto return_equal;		// treat ELF files as equal
				
				beginning = false;
			}
		}

		if (result1 != result2 || memcmp(buffer1, buffer2, result1) != 0)
		{
			close(fd1);
			close(fd2);
			return false;
		}

		length -= result1;
	}

return_equal:
	close(fd1);
	close(fd2);
	return true;
}


