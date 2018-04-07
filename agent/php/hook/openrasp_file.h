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

void pre_global_file(INTERNAL_FUNCTION_PARAMETERS);
void pre_global_readfile(INTERNAL_FUNCTION_PARAMETERS);
void pre_global_file_get_contents(INTERNAL_FUNCTION_PARAMETERS);
void pre_global_file_put_contents(INTERNAL_FUNCTION_PARAMETERS);
void pre_global_fopen(INTERNAL_FUNCTION_PARAMETERS);
void pre_global_copy(INTERNAL_FUNCTION_PARAMETERS);

void pre_splfileobject___construct(INTERNAL_FUNCTION_PARAMETERS);

#endif //OPENRASP_FILE_H