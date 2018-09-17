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
#include <winbase.h>
#include <process.h>
#include <LMCONS.H>

#include "main/php.h"
#include "ext/standard/md5.h"

#define OPENRASP_FILEMAP_NAME 	"OpenRASP.SharedMemoryArea"
#define OPENRASP_MUTEX_NAME 	"OpenRASP.SharedMemoryMutex"
#define OPENRASP_FILEMAP_BASE 	"OpenRASP.MemoryBase"
#define OPENRASP_EVENT_SOURCE 	"OpenRASP"

static HANDLE memfile = NULL, memory_mutex = NULL;
static void *mapping_base;

#define MAX_MAP_RETRIES 5

#define ZEND_BIN_ID "BIN_" ZEND_TOSTR(SIZEOF_CHAR) ZEND_TOSTR(SIZEOF_INT) ZEND_TOSTR(SIZEOF_LONG) ZEND_TOSTR(SIZEOF_SIZE_T) ZEND_TOSTR(SIZEOF_ZEND_LONG) ZEND_TOSTR(ZEND_MM_ALIGNMENT)
static char *openrasp_gen_system_id(void)
{
	PHP_MD5_CTX context;
	unsigned char digest[16], c;
	int i;
	static char md5str[32];
	static zend_bool done = 0;

	if (done) {
		return md5str;
	}

	PHP_MD5Init(&context);
	PHP_MD5Update(&context, PHP_VERSION, sizeof(PHP_VERSION)-1);
	PHP_MD5Update(&context, ZEND_EXTENSION_BUILD_ID, sizeof(ZEND_EXTENSION_BUILD_ID)-1);
	PHP_MD5Update(&context, ZEND_BIN_ID, sizeof(ZEND_BIN_ID)-1);
	if (strstr(PHP_VERSION, "-dev") != 0) {
		PHP_MD5Update(&context, __DATE__, sizeof(__DATE__)-1);
		PHP_MD5Update(&context, __TIME__, sizeof(__TIME__)-1);
	}
	PHP_MD5Final(digest, &context);
	for (i = 0; i < 16; i++) {
		c = digest[i] >> 4;
		c = (c <= 9) ? c + '0' : c - 10 + 'a';
		md5str[i * 2] = c;
		c = digest[i] &  0x0f;
		c = (c <= 9) ? c + '0' : c - 10 + 'a';
		md5str[(i * 2) + 1] = c;
	}

	done = 1;

	return md5str;
}

static char *create_name_with_username(char *name)
{
	static char newname[MAXPATHLEN + UNLEN + 4 + 1 + 32];
	char uname[UNLEN + 1];
	DWORD unsize = UNLEN;

	GetUserName(uname, &unsize);
	snprintf(newname, sizeof(newname) - 1, "%s@%s@%.32s", name, uname, openrasp_gen_system_id());
	return newname;
}

static char *get_mmap_base_file(void)
{
	static char windir[MAXPATHLEN+UNLEN + 3 + sizeof("\\\\@") + 1 + 32];
	char uname[UNLEN + 1];
	DWORD unsize = UNLEN;
	int l;

	GetTempPath(MAXPATHLEN, windir);
	GetUserName(uname, &unsize);
	l = strlen(windir);
	snprintf(windir + l, sizeof(windir) - l - 1, "\\%s@%s@%.32s", OPENRASP_FILEMAP_BASE, uname, openrasp_gen_system_id());
	return windir;
}

void openrasp_shared_alloc_create_lock(void)
{
	memory_mutex = CreateMutex(NULL, FALSE, create_name_with_username(OPENRASP_MUTEX_NAME));
	if (!memory_mutex) {
		openrasp_error(E_WARNING, SHM_ERROR, _("Cannot create mutex"));
		return;
	}
	ReleaseMutex(memory_mutex);
}

void openrasp_shared_alloc_lock_win32(void)
{
	DWORD waitRes = WaitForSingleObject(memory_mutex, INFINITE);

	if (waitRes == WAIT_FAILED) {
		openrasp_error(E_WARNING, SHM_ERROR, _("Cannot lock mutex"));
	}
}

void openrasp_shared_alloc_unlock_win32(void)
{
	ReleaseMutex(memory_mutex);
}

static int openrasp_shared_alloc_reattach(openrasp_shared_segment_globals **shared_segments_p, char **error_in)
{
	size_t requested_size = sizeof(openrasp_shared_segment_globals);	
	int err;
	void *wanted_mapping_base;
	char *mmap_base_file = get_mmap_base_file();
	FILE *fp = fopen(mmap_base_file, "r");
	MEMORY_BASIC_INFORMATION info;

	err = GetLastError();
	if (!fp) {
		openrasp_error(E_WARNING, SHM_ERROR, _("Unable to open base address file"));
		*error_in="fopen";
		return ALLOC_FAILURE;
	}
	if (!fscanf(fp, "%p", &wanted_mapping_base)) {
		err = GetLastError();
		openrasp_error(E_WARNING, SHM_ERROR, _("Unable to read base address"));
		*error_in="read mapping base";
		fclose(fp);
		return ALLOC_FAILURE;
	}
	fclose(fp);

	/* Check if the requested address space is free */
	if (VirtualQuery(wanted_mapping_base, &info, sizeof(info)) == 0 ||
	    info.State != MEM_FREE ||
	    info.RegionSize < requested_size) {
	    err = ERROR_INVALID_ADDRESS;
		openrasp_error(E_WARNING, SHM_ERROR, _("Unable to reattach to base address, requested address space isn't free"));
		return ALLOC_FAILURE;
   	}

	mapping_base = MapViewOfFileEx(memfile, FILE_MAP_ALL_ACCESS, 0, 0, 0, wanted_mapping_base);
	err = GetLastError();

	if (mapping_base == NULL) {
		openrasp_error(E_WARNING, SHM_ERROR, _("Unable to reattach to base address: %d"), err);
		if (err == ERROR_INVALID_ADDRESS) {			
			return ALLOC_FAILURE;
		}
		return ALLOC_FAIL_MAPPING;
	}
	*shared_segments_p = (openrasp_shared_segment_globals *) mapping_base;

	return SUCCESSFULLY_REATTACHED;
}

// static int create_segments(size_t requested_size, zend_shared_segment ***shared_segments_p, int *shared_segments_count, char **error_in)
static int create_segments(openrasp_shared_segment_globals **shared_segments_p, char **error_in)
{
	size_t requested_size = sizeof(openrasp_shared_segment_globals);	
	int err, ret;
	int map_retries = 0;
	void *default_mapping_base_set[] = { (void *) 0x60000000, (void *) 0x61000000, (void *) 0x62000000, (void *) 0x63000000, (void *) 0x64000000, 0 };
#if defined(_WIN64)
	void *vista_mapping_base_set[] = { (void *) 0x0000150000000000, (void *) 0x0000250000000000, (void *) 0x0000350000000000, (void *) 0x0000750000000000, 0 };
#else
	void *vista_mapping_base_set[] = { (void *) 0x25000000, (void *) 0x26000000, (void *) 0x35000000, (void *) 0x36000000, (void *) 0x55000000, 0 };
#endif
	void **wanted_mapping_base = default_mapping_base_set;
	TSRMLS_FETCH();

	openrasp_shared_alloc_lock_win32();
	/* Mapping retries: When Apache2 restarts, the parent process startup routine
	   can be called before the child process is killed. In this case, the map will fail
	   and we have to sleep some time (until the child releases the mapping object) and retry.*/
	do {
		memfile = OpenFileMapping(FILE_MAP_WRITE, 0, create_name_with_username(OPENRASP_FILEMAP_NAME));
		err = GetLastError();
		if (memfile == NULL) {
			break;
		}

		ret =  openrasp_shared_alloc_reattach(shared_segments_p, error_in);
		err = GetLastError();
		if (ret == ALLOC_FAIL_MAPPING) {
			/* Mapping failed, wait for mapping object to get freed and retry */
			CloseHandle(memfile);
			memfile = NULL;
			if (++map_retries >= MAX_MAP_RETRIES) {
				break;
			}
			openrasp_shared_alloc_unlock_win32();
			Sleep(1000 * (map_retries + 1));
			openrasp_shared_alloc_lock_win32();
		} else {
			openrasp_shared_alloc_unlock_win32();
			return ret;
		}
	} while (1);

	if (map_retries == MAX_MAP_RETRIES) {
		openrasp_shared_alloc_unlock_win32();
		openrasp_error(E_WARNING, SHM_ERROR, _("Unable to open file mapping"));
		*error_in = "OpenFileMapping";
		return ALLOC_FAILURE;
	}

	/* creating segment here */
	*shared_segments_p = (openrasp_shared_segment_globals *) calloc(1, requested_size);
	if (!*shared_segments_p) {
		openrasp_shared_alloc_unlock_win32();
		openrasp_error(E_WARNING, SHM_ERROR, _("calloc() failed"));
		*error_in = "calloc";
		return ALLOC_FAILURE;
	}

	memfile	= CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, requested_size,
								create_name_with_username(OPENRASP_FILEMAP_NAME));
	err = GetLastError();
	if (memfile == NULL) {
		openrasp_shared_alloc_unlock_win32();
		openrasp_error(E_WARNING, SHM_ERROR, _("Unable to create file mapping"));
		*error_in = "CreateFileMapping";
		return ALLOC_FAILURE;
	}

	/* Starting from windows Vista, heap randomization occurs which might cause our mapping base to
	   be taken (fail to map). So under Vista, we try to map into a hard coded predefined addresses
	   in high memory. */
	do {
		OSVERSIONINFOEX osvi;
		SYSTEM_INFO si;

		ZeroMemory(&si, sizeof(SYSTEM_INFO));
		ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));

		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

		if (! GetVersionEx ((OSVERSIONINFO *) &osvi)) {
			osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
			if (!GetVersionEx((OSVERSIONINFO *)&osvi)) {
				break;
			}
		}

		GetSystemInfo(&si);

		/* Are we running Vista ? */
		if (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT && osvi.dwMajorVersion >= 6) {
			wanted_mapping_base = vista_mapping_base_set;
		}
	} while (0);


	do {
		*shared_segments_p = mapping_base = MapViewOfFileEx(memfile, FILE_MAP_ALL_ACCESS, 0, 0, 0, *wanted_mapping_base);
		if (*wanted_mapping_base == NULL) { /* Auto address (NULL) is the last option on the array */
			break;
		}
		wanted_mapping_base++;
	} while (!mapping_base);

	err = GetLastError();
	if (mapping_base == NULL) {
		openrasp_shared_alloc_unlock_win32();
		openrasp_error(E_WARNING, SHM_ERROR, _("Unable to create view for file mapping"));
		*error_in = "MapViewOfFile";
		return ALLOC_FAILURE;
	} else {
		char *mmap_base_file = get_mmap_base_file();
		FILE *fp = fopen(mmap_base_file, "w");
		err = GetLastError();
		if (!fp) {
			openrasp_shared_alloc_unlock_win32();
			openrasp_error(E_WARNING, SHM_ERROR, _("Unable to write base address"));
			return ALLOC_FAILURE;
		}
		fprintf(fp, "%p\n", mapping_base);
		fclose(fp);
	}

	openrasp_shared_alloc_unlock_win32();

	return ALLOC_SUCCESS;
}

static int detach_segment(openrasp_shared_segment_globals *shared_segment)
{
	openrasp_shared_alloc_lock_win32();
	if (mapping_base) {
		UnmapViewOfFile(mapping_base);
	}
	CloseHandle(memfile);
	openrasp_shared_alloc_unlock_win32();
	CloseHandle(memory_mutex);
	return 0;
}

openrasp_shared_memory_handlers openrasp_alloc_win32_handlers = {
	create_segments,
	detach_segment
};
