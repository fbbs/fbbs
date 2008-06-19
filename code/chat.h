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
/*
$Id: chat.h 2 2005-07-14 15:06:08Z root $
*/

#define MY_BBS_NAME   "日月光华"
#define MY_BBS_DOMAIN "bbs.fudan.edu.cn"

/* chat.h - port numbers for the chat rooms -- changing them is wise. */
#define CHATPORT1 7201
#define CHATPORT2 7202
#define CHATNAME1 "国际会议厅"
#define CHATNAME2 "燕圆夜话"

#define MAXDEFINEALIAS  60         /* MAX User Define Alias  */
#define MAXROOM         32         /* MAX number of Chat-Room */
#define MAXLASTCMD      6          /* MAX preserved chat input */
#define IDLEN           12         /* ID Length (must match in BBS.H) */
#define CHAT_IDLEN      9          /* Chat ID Length in Chat-Room */
#define CHAT_NAMELEN    20         /* MAX 20 characters of CHAT-ROOM NAME */
#define CHAT_TITLELEN   40         /* MAX 40 characters of CHAT-ROOM TITLE */

#define EXIT_LOGOUT     0
#define EXIT_LOSTCONN   -1
#define EXIT_CLIERROR   -2
#define EXIT_TIMEDOUT   -3
#define EXIT_KICK       -4

#define CHAT_LOGIN_OK       "OK"
#define CHAT_LOGIN_EXISTS   "EX"
#define CHAT_LOGIN_INVALID  "IN"
#define CHAT_LOGIN_BOGUS    "BG"

/* 
   This defines the set of characters disallowed in chat id's. These
   characters get translated to underscores (_) when someone tries to use
   them. At the very least you should disallow spaces and '*'.
*/ 

#define BADCIDCHARS " *:/%"

