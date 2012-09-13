#ifndef FB_LIST_H
#define FB_LIST_H

/*
 * Code taken from FreeBSD sys/queue.h with modifications.
 * Documentation taken from FreeBSD queue(3) man page with modifications.
 *
 * Copyright (c) 1991, 1993
 * The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/**
 * @defgroup slist Singly-linked Lists
 */

/** @{ */

/**
 * Defines a singly-linked list structure where 'name' is the name of the
 * structure to be defined and 'type' is the type of the elements to be
 * linked into the list.
 */
#define SLIST_HEAD(name, type)  struct name { struct type *first; }

/** Evaluates to an initializer for the list head. */
#define SLIST_HEAD_INITIALIZER  { .first = NULL }

/** Returns the first element in the list or NULL if the list is empty. */
#define SLIST_FIRST(head)  ((head)->first)

/** Initializes a list. */
#define SLIST_INIT_HEAD(head)  do { (head)->first = NULL; } while (0)

/** Declares a structure that connects the elements in the list. */
#define SLIST_FIELD(type)  struct { struct type *next; }

/** Returns the next element in the list. */
#define SLIST_NEXT(entry, field)  ((entry)->field.next)

/** Inserts the new element 'entry' at the head of the list. */
#define SLIST_INSERT_HEAD(head, entry, field)  \
	do { \
		SLIST_NEXT(entry, field) = SLIST_FIRST(head); \
		SLIST_FIRST(head) = entry; \
	} while (0)

/** Inserts the new element 'entry' to the list after element 'base' */
#define SLIST_INSERT_AFTER(base, entry, field) \
	do { \
		SLIST_NEXT(entry, field) = SLIST_NEXT(base, field); \
		SLIST_NEXT(base, field) = entry; \
	} while (0)

/**
 * Traverses the list referenced by 'head' in the forward direction, assigning
 * each element in turn to var.
 */
#define SLIST_FOREACH(type, var, head, field)  \
	for (type *var = SLIST_FIRST(head); var; var = SLIST_NEXT(var, field))

/** @} */

#endif // FB_LIST_H
