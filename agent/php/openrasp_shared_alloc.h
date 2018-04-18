/*
 * Copyright 2017-2018 Baidu Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef OPENRASP_SHARED_ALLOC_H
#define OPENRASP_SHARED_ALLOC_H

#include "zend.h"
#include "zend_extensions.h"
#include "openrasp.h"

#if defined(__APPLE__) && defined(__MACH__) /* darwin */
# ifdef HAVE_SHM_MMAP_ANON
#  define USE_MMAP      1
# endif
#elif defined(__linux__)
# ifdef HAVE_SHM_MMAP_ANON
#  define USE_MMAP      1
# endif
#else
# ifdef HAVE_SHM_MMAP_ANON
#  define USE_MMAP      1
# endif
#endif

#define ALLOC_FAILURE           0
#define ALLOC_SUCCESS           1
#define FAILED_REATTACHED       2
#define SUCCESSFULLY_REATTACHED 4
#define ALLOC_FAIL_MAPPING      8

#define SQL_CONNECTION_ARRAY_SIZE 32

/*** file locking ***/
#ifndef ZEND_WIN32
extern int lock_file;

# if (defined(__APPLE__) && defined(__MACH__)/* Darwin */)
#  define FLOCK_STRUCTURE(name, type, whence, start, len) \
		struct flock name = {start, len, -1, type, whence}
# elif defined(__linux__)
#  define FLOCK_STRUCTURE(name, type, whence, start, len) \
		struct flock name = {type, whence, start, len, 0}
# elif defined(HAVE_FLOCK_BSD)
#  define FLOCK_STRUCTURE(name, type, whence, start, len) \
		struct flock name = {start, len, -1, type, whence}
# elif defined(HAVE_FLOCK_LINUX)
#  define FLOCK_STRUCTURE(name, type, whence, start, len) \
		struct flock name = {type, whence, start, len}
# else
#  error "Don't know how to define struct flock"
# endif
#endif

typedef struct _openrasp_shared_segment_globals {
    size_t size;
	char date[32];
    ulong sql_connection_hash[SQL_CONNECTION_ARRAY_SIZE];
} openrasp_shared_segment_globals;

extern openrasp_shared_segment_globals *shared_segment_globals;

typedef int (*create_segments_t)(openrasp_shared_segment_globals **shared_segments, char **error_in);
typedef int (*detach_segment_t)(openrasp_shared_segment_globals *shared_segment);

typedef struct {
	create_segments_t create_segments;
	detach_segment_t detach_segment;	
} openrasp_shared_memory_handlers;

typedef struct _handler_entry {
	const char                  *name;
	openrasp_shared_memory_handlers *handler;
} openrasp_shared_memory_handler_entry;

int openrasp_shared_alloc_startup();
void openrasp_shared_alloc_shutdown();
void set_sapi_shared_alloc_available(zend_bool available);

/* exclusive locking */
void openrasp_shared_alloc_lock(TSRMLS_D);
void openrasp_shared_alloc_unlock(TSRMLS_D); /* returns the allocated size during lock..unlock */
void openrasp_shared_alloc_safe_unlock(TSRMLS_D);

int openrasp_shared_hash_exist(ulong hash, char *date_tag);

#ifdef USE_MMAP
extern openrasp_shared_memory_handlers openrasp_alloc_mmap_handlers;
#endif

#ifdef ZEND_WIN32
extern openrasp_shared_memory_handlers openrasp_alloc_win32_handlers;
void openrasp_shared_alloc_create_lock(void);
void openrasp_shared_alloc_lock_win32(void);
void openrasp_shared_alloc_unlock_win32(void);
#endif

#endif /* OPENRASP_SHARED_ALLOC_H */
