/* 
 * html.c       -- some function for html string parse, or something else
 * 
 * of SEEDNetBBS generation 1 (libtool implement)
 *
 * Copyright (c) 1998, 1999, Edward Ping-Da Chuang <edwardc@edwardc.dhs.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * CVS: $Id: html.c 366 2007-05-12 16:35:51Z danielfree $
 */

#ifdef BBS
  #include "bbs.h"
#else
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
#endif

char html_c[] = 
  "$Id: html.c 366 2007-05-12 16:35:51Z danielfree $";

char *no_tag(char *source)
{
	static char result[256];
	int i, j, lock;
	
	lock = 0;	/* 0 is unlock, 1 is locked */	
	j = 0;
	
	memset(result,0,sizeof(result));
	for ( i = 0 ; i < 256 ; i++ ) {
		if ( source[i] == 0 )
			break;
		else if ( source[i] == '<' && lock == 0 )
			lock = 1;
		else if ( source[i] == '>' && lock == 1 )
			lock = 0;
		else if ( lock == 0 )
			result[j++] = source[i];
	}
	
	return((char *)result);
}
