/*
 *  bdremoteng - helper daemon for Sony(R) BD Remote Control
 *  Based on bdremoted, written by Anton Starikov <antst@mail.ru>.
 *
 *  Copyright (C) 2009  Michael Wojciechowski <wojci@wojci.dk>
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

/** \ingroup Gen */
/*@{*/

/*! \file globaldefs.h
  \brief Global defines used by this application.

  This file contains all global defines used by this
  application. This means some common definitions, macros and
  debugging defines.

*/

#ifndef BD_GLOBALDEFS_H
#define BD_GLOBALDEFS_H

/** Define used to return errors. */
#ifndef BDREMOTE_FAIL
#  define BDREMOTE_FAIL (-1)
#endif

/** Define used to return success. */
#ifndef BDREMOTE_OK
#  define BDREMOTE_OK (0)
#endif

/** Define this to enable debug. */
#ifndef BDREMOTE_DEBUG
#  define BDREMOTE_DEBUG 1
#endif

/* Masks used for logging. */

/** Mask: LIRC thread. */
#define MODULEMASK_LIRC_THR  0x1
/** Mask: LIRC server. */
#define MODULEMASK_LIRC_SOCK 0x2
/** Mask: LIRC callback. */
#define MODULEMASK_LIRC_CB   0x4
/** Mask: bluetooth interface. */
#define MODULEMASK_BT_IF     0x8
/** Mask: bluetooth implementation. */
#define MODULEMASK_BT_IMPL   0x10
/** Mask: queue. */
#define MODULEMASK_QUEUE     0x20
/** Mask: unused. */
#define MODULEMASK_SPARE     0x40 /* Unused. */
/** Mask: main application. */
#define MODULEMASK_MAIN      0x80

/** Mask used for debugging. */
extern unsigned int globalLogMask;

/** Macro used to check, if a mask is set. */
#define BDREMOTE_MASKCHECK(_MASK) ((globalLogMask & _MASK) == _MASK)

/** Macro used to cgecj if a module mask is set. */
#define BDREMOTE_ISMASKSET BDREMOTE_MASKCHECK(moduleMask)

#include <stdio.h>
extern FILE* printStream;

#if BDREMOTE_DEBUG
#include <time.h>

/** Extract the hour from a time_t struct. */
int getHour(time_t* _ltime);

/** Extract the minute from a time_t struct. */
int getMinute(time_t* _ltime);

/** Extract the second from a time_t struct. */
int getSecond(time_t* _ltime);

/** Given a path, return the file name only*/
const char* bdrGetFilename(const char* _filenameWithSlashes);

/** Macro used to print a time stamp. */
#  define BDREMOTE_DBG_HDR(_ENABLED) { if (_ENABLED) { time_t ltime = time(NULL); fprintf(printStream, "%s:%d -> %d:%d:%d:\n", bdrGetFilename(__FILE__), __LINE__, getHour(&ltime), getMinute(&ltime), getSecond(&ltime)); fflush(printStream);} }

/** Macro used to print a number of messages using fprintf. */
#   define BDREMOTE_LOG(_ENABLED, _x) { if ((_ENABLED) && (BDREMOTE_ISMASKSET)) { BDREMOTE_DBG_HDR(_ENABLED); _x; fflush(printStream); } }

/** Macro used to print debug output. */
#  define BDREMOTE_DBG(_ENABLED, _x) { if ((_ENABLED) && (BDREMOTE_ISMASKSET)) { BDREMOTE_DBG_HDR(_ENABLED); fprintf(printStream, "%s\n",_x); fflush(printStream); }}


/** Macro used to print error output. */
#  define BDREMOTE_ERR(x) { fprintf(printStream, "%s:%d -> ERROR: %s\n", __FILE__, __LINE__, x); fflush(printStream); }

#else

/** Macro used to print a time stamp. */
#  define BDREMOTE_DBG_HDR(_ENABLED)

/** Macro used to print a number of messages using fprintf. */
#  define BDREMOTE_LOG(_ENABLED, _x) { if ((_ENABLED && (BDREMOTE_ISMASKSET)) {_x; fflush(printStream); } }

/** Macro used to print debug output. */
#  define BDREMOTE_DBG(_ENABLED, _x) if ((_ENABLED) && (BDREMOTE_ISMASKSET)) { fprintf(printStream, "%s\n", _x); fflush(printStream);}

/** Macro used to print error output. */
#  define BDREMOTE_ERR(x) fprintf(printStream, "Error: %s\n", x); fflush(printStream);

#endif

/** Macro used to copy a string to the config. */
#define SETVAL(CK, S) {                         \
    if (CK != NULL)                             \
      {                                         \
        free(CK);                               \
        CK = NULL;                              \
      }                                         \
    CK =(char *)malloc(strlen(S)+1);            \
    strcpy(CK, S);                              \
  }

/** Macro used to destroy string members of the config. */
#define FREEVAL(CK) {                           \
    if (CK != NULL)                             \
      {                                         \
        free(CK);                               \
        CK = NULL;                              \
      }                                         \
  }

#endif /* BD_GLOBALDEFS_H */

/*@}*/

