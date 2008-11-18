// ========================================================================================
//	TChildProcess.h		 		Copyright (C) 2001-2002 Mike Lockwood. All rights reserved.
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

#ifndef __TChildProcess__
#define __TChildProcess__

#include <fcntl.h>


class TChildProcess
{
public:
								TChildProcess(pid_t pid, int stdinFD, int stdoutFD, int stderrFD);
	virtual 					~TChildProcess();

	void						Terminate();	// sends a SIGTERM signal
	
	void						Terminated(int status);

	inline bool					IsRunning() const { return fRunning; }
	inline int					GetExitStatus() const { return fStatus; }

	inline int					GetStdin() const { return fStdin; }
	inline int					GetStdout() const { return fStdout; }
	inline int					GetStderr() const { return fStderr; }
	
	static int 					CompareByPID(const TChildProcess* item1, const TChildProcess* item2);
	static int 					SearchByPID(const TChildProcess* item, const pid_t* id);

protected:
	pid_t						fPID;
	bool						fRunning;
	int							fStatus;
	int							fStdin;
	int							fStdout;
	int							fStderr;
};

#endif // __TChildProcess__
