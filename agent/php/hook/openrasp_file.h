#ifndef OPENRASP_FILE_H
#define OPENRASP_FILE_H

#include "openrasp_hook.h"

#ifdef ZEND_WIN32
# ifndef MAXPATHLEN
#  define MAXPATHLEN     _MAX_PATH
# endif
#else
# ifndef MAXPATHLEN
#  define MAXPATHLEN     4096
# endif
#endif

void hook_file(INTERNAL_FUNCTION_PARAMETERS);
void hook_readfile(INTERNAL_FUNCTION_PARAMETERS);
void hook_file_get_contents(INTERNAL_FUNCTION_PARAMETERS);
void hook_file_put_contents(INTERNAL_FUNCTION_PARAMETERS);
void hook_fopen(INTERNAL_FUNCTION_PARAMETERS);
void hook_copy(INTERNAL_FUNCTION_PARAMETERS);

void hook_splfileobject___construct_ex(INTERNAL_FUNCTION_PARAMETERS);

#endif //OPENRASP_FILE_H