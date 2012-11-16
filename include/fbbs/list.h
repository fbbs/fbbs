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
 * @{
 */

/**
 * Define a singly-linked list structure where @a name is the name of the
 * structure to be defined and @a type is the type of the elements to be
 * linked into the list.
 */
#define SLIST_HEAD(name, type)  struct name { struct type *first; }

/** Evaluate to an initializer for the list head. */
#define SLIST_HEAD_INITIALIZER  { .first = NULL }

/** Return the first element in the list or NULL if the list is empty. */
#define SLIST_FIRST(head)  ((head)->first)

/** Initialize a list. */
#define SLIST_INIT_HEAD(head)  do { (head)->first = NULL; } while (0)

/** Declare a structure that connects the elements in the list. */
#define SLIST_FIELD(type)  struct { struct type *next; }

/** Return the element next to @a entry in the list. */
#define SLIST_NEXT(entry, field)  ((entry)->field.next)

/** Insert the new element @a entry at the head of the list. */
#define SLIST_INSERT_HEAD(head, entry, field)  \
	do { \
		SLIST_NEXT(entry, field) = SLIST_FIRST(head); \
		SLIST_FIRST(head) = entry; \
	} while (0)

/** Insert the new element @a entry to the list after element @a base. */
#define SLIST_INSERT_AFTER(base, entry, field) \
	do { \
		SLIST_NEXT(entry, field) = SLIST_NEXT(base, field); \
		SLIST_NEXT(base, field) = entry; \
	} while (0)

/**
 * Traverse the list referenced by @a head in the forward direction, assigning
 * each element in turn to @a var.
 */
#define SLIST_FOREACH(type, var, head, field)  \
	for (type *var = SLIST_FIRST(head); var; var = SLIST_NEXT(var, field))

/** @} */

/**
 * @defgroup stailq Singly-linked Tail Queue
 * @{
 */

/**
 * Define a singly-linked tail queue structure where @a name is the name of
 * the structure to be defined and @a type is the type of the elements to be
 * linked into the queue.
 */
#define STAILQ_HEAD(name, type)  \
	struct name { struct type *first; struct type **last; }

/** Evaluate to an initializer for the queue head. */
#define STAILQ_HEAD_INITIALIZER(head)  \
	{ .first = NULL, .last = &(head).first }

/** Return the first element in the queue or NULL if the queue is empty. */
#define STAILQ_FIRST(head)  ((head)->first)

/** Initialize a queue. */
#define STAILQ_INIT_HEAD(head)  \
	do { \
		STAILQ_FIRST(head) = NULL; \
		(head)->last = &STAILQ_FIRST((head)); \
	} while (0)

/** Declare a structure that connects the elements in the queue. */
#define STAILQ_FIELD(type)  struct { struct type *next; }

/** Return the element next to @a entry in the queue. */
#define STAILQ_NEXT(entry, field)  ((entry)->field.next)

/** Insert the new element @a entry at the head of the queue. */
#define STAILQ_INSERT_HEAD(head, entry, field)  \
	do { \
		if ((STAILQ_NEXT(entry, field) = STAILQ_FIRST(head)) == NULL) \
			(head)->last = &STAILQ_NEXT(entry, field); \
		STAILQ_FIRST(head) = entry; \
	} while (0)

/** Insert the new element @a entry to the queue after element @a base. */
#define STAILQ_INSERT_AFTER(head, base, entry, field)  \
	do { \
		if ((STAILQ_NEXT(entry, field) = STAILQ_NEXT(base, field)) == NULL) \
			(head)->last = &STAILQ_NEXT(entry, field); \
		STAILQ_NEXT(base, field) = entry; \
	} while (0)

/** Insert the new element @a entry at the tail of the queue. */
#define STAILQ_INSERT_TAIL(head, entry, field)  \
	do { \
		STAILQ_NEXT(entry, field) = NULL; \
		*(head)->last = entry; \
		(head)->last = &STAILQ_NEXT(entry, field); \
	} while (0)

/**
 * Traverse the queue referenced by @a head in the forward direction, assigning
 * each element in turn to @a var.
 */
#define STAILQ_FOREACH(type, var, head, field)  \
	for (type *var = STAILQ_FIRST((head)); var; var = STAILQ_NEXT(var, field))

/** @} */

#endif // FB_LIST_H
