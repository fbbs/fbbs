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

#define WRAPMARGIN (255)

#define M_MARK  0x01
#define M_ATTR0 0x01
#define M_ATTR1 0x02
#define M_ATTR2 0x04
#define M_ATTR3 0x08
#define M_ATTR4 0x10
#define M_ATTR5 0x20
#define M_ATTR6 0x40
#define M_ATTR7 0x80


struct textline {
    struct textline *prev ;
    struct textline *next ;
    int  len ;
    unsigned char attr;
    char data[WRAPMARGIN + 1] ;
} ;

