// ========================================================================================
//	TImage.cpp				   Copyright (C) 2001-2002 Mike Voydanoff. All rights reserved.
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

#include "TImage.h"
#include "TApplication.h"

#include <stdlib.h>

#ifdef HAVE_SYS_IPC_H
#include <sys/ipc.h>
#endif

#ifdef HAVE_SYS_SHM_H
#include <sys/shm.h>
#endif

#include <X11/Xutil.h>

#if defined(HAVE_SYS_IPC_H) && defined(HAVE_SYS_SHM_H) && defined(HAVE_XSHM)
#define USE_SHM 1
#endif

TImage::TImage(TCoord width, TCoord height)
	:	fImage(NULL),
		fBuffer(NULL),
		fWidth(width),
		fHeight(height),
		fRowBytes(0)
{
	Display* display = gApplication->GetDisplay();
	int screen = gApplication->GetDefaultScreen();
	fDepth = DefaultDepth(display, screen);

	XVisualInfo	visualInfo;
	if (! XMatchVisualInfo(display, screen, 16, TrueColor, &visualInfo))
	{
		ASSERT(0);
	}

#ifdef USE_SHM	
	memset(&fShmInfo, 0, sizeof(fShmInfo));
	
	if (XShmQueryExtension(display))
	{
		fImage = XShmCreateImage(display, visualInfo.visual, fDepth,
								 ZPixmap, NULL, &fShmInfo, width, height);

		if (fImage)
		{
			fRowBytes = fImage->bytes_per_line;
			fShmInfo.shmid = shmget(IPC_PRIVATE, fRowBytes * height, IPC_CREAT | 0777);
			
			if (fShmInfo.shmid == -1)
			{
				XDestroyImage(fImage);
				fImage = NULL;
			}
			else
			{
				fShmInfo.readOnly = false;
				fImage->data = fBuffer = fShmInfo.shmaddr = (char *)shmat(fShmInfo.shmid, 0, 0);
			
				if (fShmInfo.shmaddr == (char *)-1)
				{
					XDestroyImage(fImage);
					fImage = NULL;
				}
				else
				{
					XShmAttach(display, &fShmInfo);
					XSync(display, false);
				}		
			}			 
		}
	}
#endif // USE_SHM
	
	if (fImage)
	{
		fUsingShm = true;
	}
	else
	{
		fRowBytes = (((width * fDepth) / 8) * 4 + 3) / 4;	// round up to nearest 4 bytes
		fBuffer = (char*)malloc(height * fRowBytes);
		fImage = XCreateImage(display, visualInfo.visual, fDepth, ZPixmap, 0,
	    						fBuffer, width, height, 32, fRowBytes);
	    fUsingShm = false;
	}
}


TImage::~TImage()
{
	if (fImage)
		XDestroyImage(fImage);

#ifdef USE_SHM 
	if (fUsingShm)
	{
		XShmDetach(gApplication->GetDisplay(), &fShmInfo);
		shmdt(fShmInfo.shmaddr);
	}
#endif // USE_SHM

	if (fBuffer)
		free(fBuffer);
}


