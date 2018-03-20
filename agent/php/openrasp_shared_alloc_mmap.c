#include "openrasp_shared_alloc.h"

#ifdef USE_MMAP

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

#if defined(MAP_ANON) && !defined(MAP_ANONYMOUS)
# define MAP_ANONYMOUS MAP_ANON
#endif

static int create_segments(openrasp_shared_segment_globals **shared_segments_p, char **error_in)
{
	size_t requested_size = sizeof(openrasp_shared_segment_globals);	
	*shared_segments_p = mmap(0, requested_size, PROT_READ | PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
	if (*shared_segments_p == MAP_FAILED) {
		*error_in = "mmap";
		return ALLOC_FAILURE;
	}

	return ALLOC_SUCCESS;
}

static int detach_segment(openrasp_shared_segment_globals *shared_segment)
{
	munmap(shared_segment, sizeof(openrasp_shared_segment_globals));
	return 0;
}

openrasp_shared_memory_handlers openrasp_alloc_mmap_handlers = {
	create_segments,
	detach_segment
};

#endif /* USE_MMAP */
