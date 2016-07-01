/*
 * Author:  David Robert Nadeau
 * Site:    http://NadeauSoftware.com/
 * License: Creative Commons Attribution 3.0 Unported License
 *          http://creativecommons.org/licenses/by/3.0/deed.en_US
 */

#if defined(WIN32) || defined(_WIN32)
#include <Windows.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/param.h>
#if defined(BSD)
#include <sys/sysctl.h>
#endif
#endif

#include "system_resource_stats.h"

/**
 * Returns the size of physical memory (RAM) in bytes.
 */
int64_t get_memory_size(void)
{
#if defined(_WIN32) && (defined(__CYGWIN__) || defined(__CYGWIN32__))
    /* Cygwin under Windows. ------------------------------------ */
    /* New 64-bit MEMORYSTATUSEX isn't available.  Use old 32.bit */
    MEMORYSTATUS status;
    status.dwLength = sizeof(status);
    GlobalMemoryStatus( &status );
    return (int64_t)status.dwTotalPhys;

#elif defined(WIN32) || defined(_WIN32)
    /* Windows. ------------------------------------------------- */
    /* Use new 64-bit MEMORYSTATUSEX, not old 32-bit MEMORYSTATUS */
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(status);
    GlobalMemoryStatusEx( &status );
    return (int64_t)status.ullTotalPhys;

#else
    /* UNIX variants. ------------------------------------------- */
    /* Prefer sysctl() over sysconf() except sysctl() HW_REALMEM and HW_PHYSMEM */

#if defined(CTL_HW) && (defined(HW_MEMSIZE) || defined(HW_PHYSMEM64))
    int mib[2];
    mib[0] = CTL_HW;
#if defined(HW_MEMSIZE)
    mib[1] = HW_MEMSIZE; /* OSX. --------------------- */
#elif defined(HW_PHYSMEM64)
    mib[1] = HW_PHYSMEM64; /* NetBSD, OpenBSD. --------- */
#endif
    int64_t size = 0; /* 64-bit */
    size_t len = sizeof(size);
    if ( sysctl( mib, 2, &size, &len, NULL, 0 ) == 0 ) {
        return size;
    }
    return 0; /* Failed? */

#elif defined(_SC_AIX_REALMEM)
    /* AIX. ----------------------------------------------------- */
    return (int64_t)(sysconf( _SC_AIX_REALMEM ) * 1024);

#elif defined(_SC_PHYS_PAGES) && defined(_SC_PAGESIZE)
    /* FreeBSD, Linux, OpenBSD, and Solaris. -------------------- */
    return (int64_t)sysconf( _SC_PHYS_PAGES )
           * (int64_t)sysconf( _SC_PAGESIZE );

#elif defined(_SC_PHYS_PAGES) && defined(_SC_PAGE_SIZE)
    /* Legacy. -------------------------------------------------- */
    return (int64_t)sysconf( _SC_PHYS_PAGES )
           * (int64_t)sysconf( _SC_PAGE_SIZE );

#elif defined(CTL_HW) && (defined(HW_PHYSMEM) || defined(HW_REALMEM))
    /* DragonFly BSD, FreeBSD, NetBSD, OpenBSD, and OSX. -------- */
    int mib[2];
    mib[0] = CTL_HW;
#if defined(HW_REALMEM)
    mib[1] = HW_REALMEM; /* FreeBSD. ----------------- */
#elif defined(HW_PYSMEM)
    mib[1] = HW_PHYSMEM; /* Others. ------------------ */
#endif
    unsigned int size = 0; /* 32-bit */
    size_t len = sizeof( size );
    if ( sysctl( mib, 2, &size, &len, NULL, 0 ) == 0 ) {
        return (int64_t) size;
    }
    return 0; /* Failed? */
#endif /* sysctl and sysconf variants */

#endif
}

size_t get_num_cores(void) {
    size_t num_cores;
#if defined(WIN32) || defined(_WIN32)
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    num_cores = (size_t)sysinfo.dwNumberOfProcessors;
#else
    num_cores = (size_t)sysconf(_SC_NPROCESSORS_ONLN);
#endif

    return num_cores;
}
