/*
    Pirate Bulletin Board System
    Copyright (C) 1990, Edward Luke, lush@Athena.EE.MsState.EDU
    Eagles Bulletin Board System
    Copyright (C) 1992, Raymond Rocker, rocker@rock.b11.ingr.com
                        Guy Vega, gtvega@seabass.st.usm.edu
                        Dominic Tynes, dbtynes@seabass.st.usm.edu
    Firebird Bulletin Board System
    Copyright (C) 1996, Hsien-Tsung Chang, Smallpig.bbs@bbs.cs.ccu.edu.tw
                        Peng Piaw Foong, ppfoong@csie.ncu.edu.tw
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 1, or (at your option)
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
*/


/* Maximum Screen width in chars */
#define LINELEN (256)

/* Line buffer modes             */
#define MODIFIED (1)   /* if line has been modifed, output to screen   */
#define STANDOUT (2)   /* if this line has a standout region */

struct screenline {
    unsigned char oldlen ;       /* previous line length             	 */
    unsigned char len ;          /* current length of line            	*/
    unsigned char mode ;         /* status of line, as far as update  	*/
    unsigned char smod ;         /* start of modified data            	*/
    unsigned char emod ;         /* end of modified data              	*/
    unsigned char sso ;          /* start stand out 					*/
    unsigned char eso ;          /* end stand out 						*/
    unsigned char data[LINELEN] ;/*	data in one line					*/
} ;
