// ========================================================================================
//	TDirectoryDiffNode.cpp	 		 Copyright (C) 2003 Mike Lockwood. All rights reserved.
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

#include "TDirectoryDiffNode.h"


TDirectoryDiffNode::TDirectoryDiffNode(const TChar* path, bool isDirectory, int diffIndex, int depth)
	:	TTreeNode(NULL, depth, false),
		fPath(path),
		fIsDirectory(isDirectory),
		fDiffIndex(diffIndex)
{
	fExpandable = isDirectory;
}


TDirectoryDiffNode::~TDirectoryDiffNode()
{
}


void TDirectoryDiffNode::GetText(int column, TString& outText) const
{
	ASSERT(column == 1);
	
	const char* str = fPath;
	const char* prevSlash = NULL;
	const char* lastSlash = NULL;

	while (str[0])
	{
		if (*str++ == '/')
		{
			prevSlash = lastSlash;
			lastSlash = str;
		}	
	}

	if (fIsDirectory && prevSlash)
		outText = prevSlash;
	else if (!fIsDirectory && lastSlash)
		outText = lastSlash;
	else
		outText = fPath;
}


void TDirectoryDiffNode::CreateChildren()
{
	// children are precreated, so we don't need to do anything here
}

