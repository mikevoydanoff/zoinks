// ========================================================================================
//	GNU_diff_analyze.h				 Copyright (C) 2003 Mike Lockwood. All rights reserved.
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

#ifndef __GNU_diff_analyze__
#define __GNU_diff_analyze__


typedef struct file_data {
    int buffered_lines;

    /* Vector, indexed by line number, containing an equivalence code for
       each line.  It is this vector that is actually compared with that
       of another file to generate differences. */
    int		   *equivs;

     /* 1 more than the maximum equivalence value used for this or its
       sibling file. */
    int equiv_max;

   /* Vector, like the previous one except that
       the elements for discarded lines have been squeezed out.  */
    int		   *undiscarded;

    /* Vector mapping virtual line numbers (not counting discarded lines)
       to real ones (counting those lines).  Both are origin-0.  */
    int		   *realindexes;

    /* Total number of nondiscarded lines. */
    int		    nondiscarded_lines;

    /* Vector, indexed by real origin-0 line number,
       containing 1 for a line that is an insertion or a deletion.
       The results of comparison are stored here.  */
    char	   *changed_flag;

};


typedef void (* add_change_callback)(int line0, int line1, int deleted, int inserted, void* userData);


void diff_2_files (struct file_data filevec[], add_change_callback callback, void* userData);


#endif // __GNU_diff_analyze__
