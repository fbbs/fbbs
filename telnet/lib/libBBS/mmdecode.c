/* 
 * mmdecode.c		-- a tool for decoding QP/BASE64 string
 *					deliver from Maple 3
 * 
 * of SEEDNetBBS generation 1 (libBBS implement)
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
 * CVS: $Id: mmdecode.c 366 2007-05-12 16:35:51Z danielfree $
 */
 
/*-------------------------------------------------------*/
/* lib/str_decode.c	( NTHU CS MapleBBS Ver 3.00 )	     */
/*-------------------------------------------------------*/
/* target : included C for QP/BASE64 decoding		     */
/* create : 95/03/29				 	                 */
/* update : 97/03/29				 	                 */
/*-------------------------------------------------------*/


/* ----------------------------------------------------- */
/* QP code : "0123456789ABCDEF"				             */
/* ----------------------------------------------------- */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int
qp_code(x)
  register int x;
{
  if (x >= '0' && x <= '9')
    return x - '0';
  if (x >= 'a' && x <= 'f')
    return x - 'a' + 10;
  if (x >= 'A' && x <= 'F')
    return x - 'A' + 10;
  return -1;
}


/* ------------------------------------------------------------------ */
/* BASE64 :							      */
/* "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/" */
/* ------------------------------------------------------------------ */


static int
base64_code(x)
  register int x;
{
  if (x >= 'A' && x <= 'Z')
    return x - 'A';
  if (x >= 'a' && x <= 'z')
    return x - 'a' + 26;
  if (x >= '0' && x <= '9')
    return x - '0' + 52;
  if (x == '+')
    return 62;
  if (x == '/')
    return 63;
  return -1;
}


/* ----------------------------------------------------- */
/* judge & decode QP / BASE64				 */
/* ----------------------------------------------------- */


void _mmdecode(str)
  unsigned char *str;
{
  int code, c1, c2, c3, c4;
  unsigned char *src, *dst;
  unsigned char buf[256];

#define	IS_QP		0x01
#define	IS_BASE64	0x02
#define	IS_TRAN		0x04

  src = str;
  dst = buf;
  code = 0;

  while (c1 = *src++)
  {
    if (c1 == '?' && *src == '=')
    {
      if (code)
      {
	code &= IS_TRAN;
	if (*++src == ' ')
	  src++;
      }
      continue;
    }

    if (c1 == '\n')		/* chuan: multi line encoding */
      goto line;

    if (code & (IS_QP | IS_BASE64))
    {
      if (c1 == '_')
      {
	*dst++ = ' ';
	continue;
      }

      if ((c1 == '=') && (code & IS_QP))
      {
	if (!(c1 = *src))
	  break;

	if (!(c2 = *++src))
	  break;

	src++;
	*dst++ = (qp_code(c1) << 4) | qp_code(c2);
	code |= IS_TRAN;
	continue;
      }

      if (code & IS_BASE64)
      {
	c2 = *src++;

	if (c1 == '=' || c2 == '=')
	{
	  code ^= IS_BASE64;
	  continue;
	}

	if (!(c3 = *src++))
	  break;

	if (!(c4 = *src++))
	  break;

	c2 = base64_code(c2);
	*dst++ = (base64_code(c1) << 2) | ((c2 & 0x30) >> 4);

	if (c3 == '=')
	{
	  code ^= IS_BASE64;
	}
	else
	{
	  c3 = base64_code(c3);
	  *dst++ = ((c2 & 0xF) << 4) | ((c3 & 0x3c) >> 2);

	  if (c4 == '=')
	    code ^= IS_BASE64;
	  else
	    *dst++ = ((c3 & 0x03) << 6) | base64_code(c4);
	}

	code |= IS_TRAN;
	continue;
      }
    }

    /* "=?%s?Q?" for QP, "=?%s?B?" for BASE64 */

    if ((c1 == '=') && (*src == '?'))
    {
      c2 = 0;

      for (;;)
      {
	c1 = *++src;
	if (!c1)
	  goto home;

	if (c1 == '?')
	{
	  if (++c2 >= 2)
	    break;

	  continue;
	}

	if (c2 == 1)
	{
	  if (c1 == 'Q')
	    code = IS_QP;
	  else if (c1 == 'B')
	    code = IS_BASE64;
	}
      }

      src++;
      continue;
    }

    *dst++ = c1;
  }

home:

  if (code & IS_TRAN)
  {
line:
    *dst = '\0';
    strcpy(str, buf);
  }
}
