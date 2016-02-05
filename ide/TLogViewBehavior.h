// ========================================================================================
//	TLogViewBehavior.h		   Copyright (C) 2001-2002 Mike Voydanoff. All rights reserved.
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

#ifndef __TLogViewBehavior__
#define __TLogViewBehavior__

#include "fw/TBehavior.h"
#include "fw/TString.h"

class TTextView;
class TFile;

class TLogViewBehavior : public TBehavior
{
public:
								TLogViewBehavior(const char* workingDirectory);
								TLogViewBehavior(TFile* sourceFile);
	virtual						~TLogViewBehavior();

	virtual bool				DoMouseDown(const TPoint& point, TMouseButton button, TModifierState state);

protected:
	virtual void				SetOwner(TCommandHandler* owner, TBehavior* nextBehavior);

	bool						ParseMessage(const char* message, TString& path, int& lineNumber);

private:
	TTextView*					fTextView;
	TString						fWorkingDirectory;
	TFile*						fSourceFile;
};

#endif // __TLogViewBehavior__
