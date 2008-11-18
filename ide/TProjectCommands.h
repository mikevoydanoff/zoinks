// ========================================================================================
//	TProjectCommands.h		 	Copyright (C) 2001-2002 Mike Lockwood. All rights reserved.
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

#ifndef __TProjectCommands__
#define __TProjectCommands__

#include "fw/TCommandID.h"

const TCommandID kNewProjectCommandID			= 6000;
const TCommandID kMakeCommandID					= 6001;
const TCommandID kStopMakeCommandID				= 6002;
const TCommandID kDebugCommandID				= 6003;

const TCommandID kChooseMakePathCommandID		= 6004;
#ifdef ENABLE_DEBUGGER
const TCommandID kChooseDebugPathCommandID		= 6005;
#endif

const TCommandID kShowManPageCommandID			= 6010;
const TCommandID kCloseAllTextWindowsCommandID	= 6011;
const TCommandID kToggleHTMLMenuCommandID		= 6012;
const TCommandID kToggleTeXMenuCommandID		= 6013;
const TCommandID kCompareFilesCommandID			= 6014;

// Compare Files commands
const TCommandID kChoosePath1CommandID			= 6020;
const TCommandID kChoosePath2CommandID			= 6021;
const TCommandID kDoCompareFilesCommandID		= 6022;
const TCommandID kDoCompareDirectoriesCommandID	= 6023;
const TCommandID kCopyToLeftCommandID			= 6024;
const TCommandID kCopyToRightCommandID			= 6025;
const TCommandID kCopyAllToLeftCommandID		= 6026;
const TCommandID kCopyAllToRightCommandID		= 6027;
const TCommandID kRecalcDiffsCommandID			= 6028;

#endif	// __TProjectCommands__
