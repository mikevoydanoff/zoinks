// ========================================================================================
//	TDirectoryDiffNode.h		 	 Copyright (C) 2003 Mike Lockwood. All rights reserved.
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

#ifndef __TDirectoryDiffNode__
#define __TDirectoryDiffNode__

#include "fw/TTreeNode.h"
#include "fw/TString.h"


class TDirectoryDiffNode : public TTreeNode
{
public:
							TDirectoryDiffNode(const TChar* path, bool isDirectory, int diffIndex, int depth);
	virtual					~TDirectoryDiffNode();

	virtual void			GetText(int column, TString& outText) const;
	virtual void			CreateChildren();	// called when expanded	
	
	inline const TString&	GetPath() const { return fPath; }
	inline bool				IsDirectory() const { return fIsDirectory; }
	inline int				GetDiffIndex() const { return fDiffIndex; }

protected:				
	TString					fPath;
	bool					fIsDirectory;
	int						fDiffIndex;
};

#endif // __TDirectoryDiffNode__
