/* freemem.cc: determine how much free physical memory there is.
 *
 * Copyright (C) 2007,2008,2009,2010 Olly Betts
 * Copyright (C) 2008 Lemur Consulting Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <config.h>

#include "freemem.h"

#include <sys/types.h>
#include <climits>
#include "safeunistd.h"
#ifdef HAVE_SYS_SYSCTL_H
# include <sys/sysctl.h>
#endif
#ifdef HAVE_VM_VM_PARAM_H
# include <vm/vm_param.h>
#endif
#ifdef HAVE_SYS_VMMETER_H
# include <sys/vmmeter.h>
#endif
#ifdef HAVE_SYS_SYSMP_H
# include <sys/sysmp.h>
#endif
#ifdef HAVE_SYS_SYSINFO_H
# include <sys/sysinfo.h>
#endif
#ifdef HAVE_SYS_PSTAT_H
# include <sys/pstat.h>
#endif

#ifdef __WIN32__
# include "safewindows.h"
#endif

/* Tested on:
 * Linux, FreeBSD, IRIX, HP-UX, Microsoft Windows.
 */

long
get_free_physical_memory()
{
#ifndef __WIN32__
    long pagesize = 1;
    long pages = -1;
#if defined(_SC_PAGESIZE) && defined(_SC_PHYS_PAGES)
    /* Linux:
     * _SC_AVPHYS_PAGES is "available memory", but that excludes memory being
     * used by the OS VM cache, which will often be almost all memory which
     * isn't otherwise used, so used _SC_PHYS_PAGES which is just memory -
     * that's good enough for Omega's use where we really just want to avoid
     * runaway filter processes for dragging down the system.
     */
    pagesize = sysconf(_SC_PAGESIZE);
    pages = sysconf(_SC_PHYS_PAGES);
#elif defined HAVE_SYSMP
    /* IRIX: (rminfo64 and MPSA_RMINFO64?) */
    struct rminfo meminfo;
    if (sysmp(MP_SAGET, MPSA_RMINFO, &meminfo, sizeof(meminfo)) == 0) {
	pagesize = sysconf(_SC_PAGESIZE);
	pages = meminfo.availrmem;
    }
#elif defined HAVE_PSTAT_GETDYNAMIC
    /* HP-UX: */
    struct pst_dynamic info;
    if (pstat_getdynamic(&info, sizeof(info), 1, 0) == 1) {
	pagesize = getpagesize();
	pages = info.psd_free;
    }
#elif defined CTL_VM && (defined VM_TOTAL || defined VM_METER)
    /* FreeBSD: */
    struct vmtotal vm_info;
    int mib[2] = {
	CTL_VM,
#ifdef VM_TOTAL
	VM_TOTAL
#else
	VM_METER
#endif
    };
    size_t len = sizeof(vm_info);
    if (sysctl(mib, 2, &vm_info, &len, NULL, 0) == 0) {
	pagesize = getpagesize();
	pages = vm_info.t_free;
    }
#endif
    if (pagesize > 0 && pages > 0) {
	long mem = LONG_MAX;
	if (pages < LONG_MAX / pagesize) {
	    mem = pages * pagesize;
	}
	return mem;
    }
    return -1;
#else
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof(statex);
    GlobalMemoryStatusEx(&statex);
    return statex.ullAvailPhys;
#endif
}

/* Tested on:
 * Linux, Microsoft Windows.
 */

long
get_total_physical_memory()
{
#ifndef __WIN32__
    long pagesize = 1;
    long pages = -1;
#if defined(_SC_PAGESIZE) && defined(_SC_AVPHYS_PAGES)
    /* Linux: */
    pagesize = sysconf(_SC_PAGESIZE);
    pages = sysconf(_SC_PHYS_PAGES);
#elif defined HAVE_SYSMP
    /* IRIX: (rminfo64 and MPSA_RMINFO64?) */
    struct rminfo meminfo;
    if (sysmp(MP_SAGET, MPSA_RMINFO, &meminfo, sizeof(meminfo)) == 0) {
	pagesize = sysconf(_SC_PAGESIZE);
	pages = meminfo.physmem;
    }
#elif defined HAVE_PSTAT_GETDYNAMIC
    /* HP-UX: */
    struct pst_dynamic info;
    if (pstat_getdynamic(&info, sizeof(info), 1, 0) == 1) {
	pagesize = getpagesize();
	pages = info.psd_rm;
    }
#elif defined CTL_VM && (defined VM_TOTAL || defined VM_METER)
    /* FreeBSD: */
    struct vmtotal vm_info;
    int mib[2] = {
	CTL_VM,
#ifdef VM_TOTAL
	VM_TOTAL
#else
	VM_METER
#endif
    };
    size_t len = sizeof(vm_info);
    if (sysctl(mib, 2, &vm_info, &len, NULL, 0) == 0) {
	pagesize = getpagesize();
	pages = vm_info.t_rm;
    }
#endif
    if (pagesize > 0 && pages > 0) {
	long mem = LONG_MAX;
	if (pages < LONG_MAX / pagesize) {
	    mem = pages * pagesize;
	}
	return mem;
    }
    return -1;
#else
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof(statex);
    GlobalMemoryStatusEx(&statex);
    return statex.ullTotalPhys;
#endif
}
