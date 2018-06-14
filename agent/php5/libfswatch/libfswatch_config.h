/* Define to 1 if you have the <sys/inotify.h> header file. */
#if defined(__linux__)
#define HAVE_SYS_INOTIFY_H 1
#endif

/* Define to 1 if you have the <sys/stat.h> header file. */
#if !defined(_WIN32) && (defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__)) || defined(__CYGWIN__))
#define HAVE_SYS_STAT_H 1
#endif

/* Define to 1 if you have the <sys/event.h> header file. */
#if defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__))
#include <sys/param.h>
#if defined(BSD)
#define HAVE_SYS_EVENT_H 1
#endif
#endif

/* Define to 1 if you have the <port.h> header file. */
#if defined(__sun) && defined(__SVR4)
#define HAVE_PORT_H 1
#endif

/* Define if the file events are supported by OS X FSEvents API. */
#if defined(__APPLE__) && defined(__MACH__)
#define HAVE_FSEVENTS_FILE_EVENTS 1
#endif

/* Windows API present. */
#if defined(_WIN32)
#define HAVE_WINDOWS 1
#endif

/* Define if <atomic> is available. */
#define HAVE_CXX_ATOMIC 1

/* Define if <mutex> is available. */
#define HAVE_CXX_MUTEX 1

/* Define if the thread_local storage specified is available. */
#define HAVE_CXX_THREAD_LOCAL 1

/* Define to 1 if `st_mtime' is a member of `struct stat'. */
#define HAVE_STRUCT_STAT_ST_MTIME 1

/* Define to 1 if `st_mtimespec' is a member of `struct stat'. */
#define HAVE_STRUCT_STAT_ST_MTIMESPEC 1

/* Define to 1 if you have the <unordered_map> header file. */
#define HAVE_UNORDERED_MAP 1

/* Define to 1 if you have the <unordered_set> header file. */
#define HAVE_UNORDERED_SET 1