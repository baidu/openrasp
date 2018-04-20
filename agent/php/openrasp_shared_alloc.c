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

#include "openrasp_shared_alloc.h"
#include <errno.h>
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#include <fcntl.h>
#ifndef ZEND_WIN32
# include <sys/types.h>
# include <dirent.h>
# include <signal.h>
# include <sys/stat.h>
# include <stdio.h>
#endif
#include "php_main.h"

#ifdef HAVE_MPROTECT
# include "sys/mman.h"
#endif

#ifndef MIN
# define MIN(x, y) ((x) > (y)? (y) : (x))
#endif

#define TMP_DIR "/tmp"
#define SEM_FILENAME_PREFIX ".OpenRASPSem."
#define S_H(s) g_shared_alloc_handler->s

static const openrasp_shared_memory_handlers *g_shared_alloc_handler = NULL;
static const char *g_shared_model;
openrasp_shared_segment_globals *shared_segment_globals;
static zend_bool need_alloc_shm = 0;

#ifndef PHP_WIN32
#ifdef ZTS
static MUTEX_T zts_lock;
#endif
int lock_file;
static char lockfile_name[sizeof(TMP_DIR) + sizeof(SEM_FILENAME_PREFIX) + 8];
#endif

static const openrasp_shared_memory_handler_entry handler_table[] = {
#ifdef USE_MMAP
	{ "mmap", &openrasp_alloc_mmap_handlers },
#endif
#ifdef PHP_WIN32
	{ "win32", &openrasp_alloc_win32_handlers },
#endif
	{ NULL, NULL}
};

static inline int check_sapi_need_alloc_shm()
{
	static const char *supported_sapis[] = {
		"fpm-fcgi",
		"apache2handler",
		NULL};
	const char **sapi_name;
	if (sapi_module.name)
	{
		for (sapi_name = supported_sapis; *sapi_name; sapi_name++)
		{
			if (strcmp(sapi_module.name, *sapi_name) == 0)
			{
				return 1;
			}
		}
	}
	return 0;
}

#ifndef ZEND_WIN32
void openrasp_shared_alloc_create_lock(void)
{
	if(!need_alloc_shm)
	{
		return;
	}
	int val;

#ifdef ZTS
    zts_lock = tsrm_mutex_alloc();
#endif

	sprintf(lockfile_name, "%s/%sXXXXXX", TMP_DIR, SEM_FILENAME_PREFIX);
	lock_file = mkstemp(lockfile_name);
	fchmod(lock_file, 0666);

	if (lock_file == -1) {
		openrasp_error(E_WARNING, SHM_ERROR, _("Unable to create lock file: %s (%d)"), strerror(errno), errno);
	}
	val = fcntl(lock_file, F_GETFD, 0);
	val |= FD_CLOEXEC;
	fcntl(lock_file, F_SETFD, val);

	unlink(lockfile_name);
}
#endif

static int openrasp_shared_alloc_try(const openrasp_shared_memory_handler_entry *he, openrasp_shared_segment_globals **shared_segments_p, char **error_in)
{
	int res;
	g_shared_alloc_handler = he->handler;
	g_shared_model = he->name;

	res = S_H(create_segments)(shared_segments_p, error_in);

	if (res) {
		/* this model works! */
		return res;
	}
	if (*shared_segments_p) {
		if ((*shared_segments_p) && (*shared_segments_p) != (void *)-1) {
			S_H(detach_segment)(*shared_segments_p);
		}				
		*shared_segments_p = NULL;
	}
	g_shared_alloc_handler = NULL;
	return ALLOC_FAILURE;
}

int openrasp_shared_alloc_startup()
{
	if(!(need_alloc_shm = check_sapi_need_alloc_shm()))
	{
		return ALLOC_FAILURE;
	}
	char *error_in = NULL;
	const openrasp_shared_memory_handler_entry *he;
	int res = ALLOC_FAILURE;

	TSRMLS_FETCH();

	openrasp_shared_alloc_create_lock();

	/* try memory handlers in order */
	for (he = handler_table; he->name; he++) {
		res = openrasp_shared_alloc_try(he, &shared_segment_globals, &error_in);
		if (res) {
			/* this model works! */
			break;
		}
	}

	if (res == FAILED_REATTACHED) {
		shared_segment_globals = NULL;
		return res;
	}

	if (!g_shared_alloc_handler) {
		openrasp_error(E_WARNING, SHM_ERROR, _("Unable to allocate shared memory segment of %ld bytes: %s: %s (%d)"),
		sizeof(openrasp_shared_segment_globals), error_in ? error_in : "unknown", strerror(errno), errno);
		return ALLOC_FAILURE;
	}

	if (res == SUCCESSFULLY_REATTACHED) {
		return res;
	}

	return res;
}

void openrasp_shared_alloc_shutdown()
{
	if(!need_alloc_shm)
	{
		return;
	}
	if (shared_segment_globals != NULL)
	{
		S_H(detach_segment)(shared_segment_globals);
		shared_segment_globals = NULL;
	}
	g_shared_alloc_handler = NULL;
#ifndef ZEND_WIN32
	close(lock_file);
#endif
}

void openrasp_shared_alloc_safe_unlock(TSRMLS_D)
{
	if (OPENRASP_G(locked)) {
		openrasp_shared_alloc_unlock(TSRMLS_C);
	}
}

#ifndef ZEND_WIN32
/* name l_type l_whence l_start l_len */
static FLOCK_STRUCTURE(mem_write_lock, F_WRLCK, SEEK_SET, 0, 1);
static FLOCK_STRUCTURE(mem_write_unlock, F_UNLCK, SEEK_SET, 0, 1);
#endif

void openrasp_shared_alloc_lock(TSRMLS_D)
{
	if(!need_alloc_shm)
	{
		return;
	}
#ifndef ZEND_WIN32

#ifdef ZTS
	tsrm_mutex_lock(zts_lock);
#endif

#if 0
	/* this will happen once per process, and will un-globalize mem_write_lock */
	if (mem_write_lock.l_pid == -1) {
		mem_write_lock.l_pid = getpid();
	}
#endif

	while (1) {
		if (fcntl(lock_file, F_SETLKW, &mem_write_lock) == -1) {
			if (errno == EINTR) {
				continue;
			}
			openrasp_error(E_WARNING, SHM_ERROR, _("Cannot create lock - %s (%d)"), strerror(errno), errno);
		}
		break;
	}
#else
	openrasp_shared_alloc_lock_win32();
#endif

	OPENRASP_G(locked) = 1;
}

void openrasp_shared_alloc_unlock(TSRMLS_D)
{
	if(!need_alloc_shm)
	{
		return;
	}
	OPENRASP_G(locked) = 0;

#ifndef ZEND_WIN32
	if (fcntl(lock_file, F_SETLK, &mem_write_unlock) == -1) {
		openrasp_error(E_WARNING, SHM_ERROR, _("Cannot remove lock - %s (%d)"), strerror(errno), errno);
	}
#ifdef ZTS
	tsrm_mutex_unlock(zts_lock);
#endif
#else
	openrasp_shared_alloc_unlock_win32();
#endif
}

int openrasp_shared_hash_exist(ulong hash, char *date_tag)
{
	if (!shared_segment_globals)
	{
		return 0;
	}
	else
	{
		if (strcmp(shared_segment_globals->date, date_tag) != 0)
		{
			shared_segment_globals->size = 0;
			strcpy(shared_segment_globals->date, date_tag);
		}
		int i;
		for (i = 0; i <= MIN(shared_segment_globals->size, SQL_CONNECTION_ARRAY_SIZE); ++i)	
		{
			if (shared_segment_globals->sql_connection_hash[i] == hash)
			{
				return 1;
			}
		}
		if (shared_segment_globals->size < SQL_CONNECTION_ARRAY_SIZE - 1)
		{
			shared_segment_globals->sql_connection_hash[shared_segment_globals->size] = hash;
			shared_segment_globals->size += 1;
		}
		return 0;
	}
}
