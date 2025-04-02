/* $Progeny: mkdtemp.c 3839 2003-11-17 04:25:01Z dsp $ */
/*	$NetBSD: gettemp.c,v 1.5 1999/09/20 04:39:30 lukem Exp $	*/

/*
 * Copyright (c) 1987, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
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

#include <config.h>

#ifndef HAVE_MKDTEMP

#include "mkdtemp.h"

#include <sys/types.h>

#include "safesysstat.h"
#include "safeunistd.h"

#include <cerrno>

static int
do_mkdtemp(char *path)
{
	char *start, *trv;
	struct stat sbuf;
	pid_t pid;

	/* To guarantee multiple calls generate unique names even if
	   the file is not created. 676 different possibilities with 7
	   or more X's, 26 with 6 or less. */
	static char xtra[2] = { 'a', 'a' };
	int xcnt = 0;

	pid = getpid();

	/* Move to end of path and count trailing X's. */
	for (trv = path; *trv; ++trv)
		if (*trv == 'X')
			xcnt++;
		else
			xcnt = 0;

	/* Use at least one from xtra.  Use 2 if more than 6 X's. */
	if (*(trv-1) == 'X')
		*--trv = xtra[0];
	if (xcnt > 6 && *(trv-1) == 'X')
		*--trv = xtra[1];

	/* Set remaining X's to pid digits with 0's to the left. */
	while (*--trv == 'X') {
		*trv = (pid % 10) + '0';
		pid /= 10;
	}

	/* update xtra for next call. */
	if (xtra[0] != 'z')
		xtra[0]++;
	else {
		xtra[0] = 'a';
		if (xtra[1] != 'z')
			xtra[1]++;
		else
			xtra[1] = 'a';
	}

	/*
	 * check the target directory; if you have six X's and it
	 * doesn't exist this runs for a *very* long time.
	 */
	for (start = trv + 1;; --trv) {
		if (trv <= path)
			break;
		if (*trv == '/') {
			*trv = '\0';
			if (stat(path, &sbuf))
				return (0);
			if (!S_ISDIR(sbuf.st_mode)) {
				errno = ENOTDIR;
				return (0);
			}
			*trv = '/';
			break;
		}
	}

	for (;;) {
		if (mkdir(path, 0700) >= 0)
			return (1);
		if (errno != EEXIST)
			return (0);

		/* tricky little algorithm for backward compatibility */
		for (trv = start;;) {
			if (!*trv)
				return (0);
			if (*trv == 'z')
				*trv++ = 'a';
			else {
				if (*trv >= '0' && *trv <= '9')
					*trv = 'a';
				else
					++*trv;
				break;
			}
		}
	}
	/*NOTREACHED*/
}

char *
fallback_mkdtemp(char *path)
{
	return (do_mkdtemp(path) ? path : (char *)0);
}

#endif
