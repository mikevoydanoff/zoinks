// ========================================================================================
//	TChildProcess.cpp		 	Copyright (C) 2001-2002 Mike Lockwood. All rights reserved.
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

#include "TChildProcess.h"
#include "TApplication.h"

#include <unistd.h>
#include <signal.h>
#include <stdio.h>


TChildProcess::TChildProcess(pid_t pid, int stdinFD, int stdoutFD, int stderrFD)
	:	fPID(pid),
		fRunning(true),
		fStatus(0),
		fStdin(stdinFD),
		fStdout(stdoutFD),
		fStderr(stderrFD)
{
}


TChildProcess::~TChildProcess()
{
	if (fStdin)
		close(fStdin);
	if (fStdout)
		close(fStdout);
	if (fStderr)
		close(fStderr);

	gApplication->fChildren.Remove(this);
}


void TChildProcess::Terminate()
{
	kill(fPID, SIGTERM);
}


void TChildProcess::Terminated(int status)
{
	fRunning = false;
	fStatus = status;
}


int TChildProcess::CompareByPID(const TChildProcess* item1, const TChildProcess* item2)
{
	if (item1->fPID < item2->fPID)
		return -1;
	else if (item1->fPID > item2->fPID)
		return 1;
	else
		return 0;
}


int TChildProcess::SearchByPID(const TChildProcess* item, const pid_t* pid)
{
	pid_t	id = *pid;
	
	if (item->fPID < id)
		return -1;
	else if (item->fPID > id)
		return 1;
	else
		return 0;
}


