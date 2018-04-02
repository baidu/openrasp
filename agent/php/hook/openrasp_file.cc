#include "openrasp_file.h"

static const char* mode_to_type(char *mode)
{
    if (strchr(mode, '+') > mode)
    {
        return "writeFile";
    }
    else if (strchr(mode, 'r') != nullptr)
    {
        return "readFile";
    }
    else
    {
        return "writeFile";
    }
}

static void check_file_operation(const char* type, char *filename, int filename_len, zend_bool use_include_path TSRMLS_DC)
{
    char resolved_path_buff[MAXPATHLEN];
    char *real_path = nullptr;
    real_path = php_resolve_path(filename, filename_len, use_include_path ? PG(include_path) : NULL TSRMLS_CC);
    if (real_path)
    {
        zval *params;
        MAKE_STD_ZVAL(params);
        array_init(params);
        zval *path = NULL;
        MAKE_STD_ZVAL(path);
        ZVAL_STRING(path, filename, 1);
        zval *realpath = NULL;
        MAKE_STD_ZVAL(realpath);
        ZVAL_STRING(realpath, real_path, 1);
        add_assoc_zval(params, "path", path);
        add_assoc_zval(params, "realpath", realpath);
        check(type, params TSRMLS_CC);
    }
}

void pre_global_file(INTERNAL_FUNCTION_PARAMETERS)
{
    char *filename;
	int filename_len;
	long flags = 0;
    zend_bool use_include_path;
	zval *zcontext = NULL;

	if (openrasp_check_type_ignored(ZEND_STRL("readFile") TSRMLS_CC)
    || zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|lr!", &filename, &filename_len, &flags, &zcontext) == FAILURE) {
		return;
	}
    use_include_path = flags & PHP_FILE_USE_INCLUDE_PATH;
    check_file_operation("readFile", filename, filename_len, use_include_path TSRMLS_CC);
}

void pre_global_readfile(INTERNAL_FUNCTION_PARAMETERS)
{
    char *filename;
	int filename_len;
	zend_bool use_include_path = 0;
	zval *zcontext = NULL;

	if (openrasp_check_type_ignored(ZEND_STRL("readFile") TSRMLS_CC)
    || zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|br!", &filename, &filename_len, &use_include_path, &zcontext) == FAILURE) {
		return;
	}
    check_file_operation("readFile", filename, filename_len, use_include_path TSRMLS_CC);
}

void pre_global_file_get_contents(INTERNAL_FUNCTION_PARAMETERS)
{
    char *filename;
	int filename_len;
	zend_bool use_include_path = 0;
	long offset = -1;
	long maxlen = PHP_STREAM_COPY_ALL;
	zval *zcontext = NULL;

	if (openrasp_check_type_ignored(ZEND_STRL("readFile") TSRMLS_CC)
    || zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|br!ll", &filename, &filename_len, &use_include_path, &zcontext, &offset, &maxlen) == FAILURE) {
		return;
	}
    check_file_operation("readFile", filename, filename_len, use_include_path TSRMLS_CC);
}
void pre_global_file_put_contents(INTERNAL_FUNCTION_PARAMETERS)
{
    zval **path, **data, **flags;
    char resolved_path_buff[MAXPATHLEN];
    int argc = MIN(3, ZEND_NUM_ARGS());
    if (argc > 1 && zend_get_parameters_ex(argc, &path, &data, &flags) == SUCCESS &&
        Z_TYPE_PP(path) == IS_STRING)
    {
        if (!openrasp_check_type_ignored(ZEND_STRL("webshell_file_put_contents") TSRMLS_CC)
            && openrasp_zval_in_request(*path TSRMLS_CC)
            && openrasp_zval_in_request(*data TSRMLS_CC))
        {
            zval *attack_params = NULL;
            MAKE_STD_ZVAL(attack_params);
            ZVAL_STRING(attack_params, "", 1);
            zval *plugin_message = NULL;
            MAKE_STD_ZVAL(plugin_message);
            ZVAL_STRING(plugin_message, _("File dropper backdoor"), 1);
            openrasp_buildin_php_risk_handle(1, "webshell_file_put_contents", 100, attack_params, plugin_message TSRMLS_CC);
        }
        if (!openrasp_check_type_ignored(ZEND_STRL("writeFile") TSRMLS_CC))
        {
            char *real_path = nullptr;
            char *include_path = nullptr;
            if (argc == 3 && Z_TYPE_PP(flags) == IS_LONG && (Z_LVAL_PP(flags) & PHP_FILE_USE_INCLUDE_PATH))
            {
                include_path = PG(include_path);
            }
            else
            {
                include_path = NULL;
            }
            real_path = php_resolve_path(Z_STRVAL_PP(path), Z_STRLEN_PP(path), include_path TSRMLS_CC);
            zval *params;
            MAKE_STD_ZVAL(params);
            array_init(params);
            add_assoc_zval(params, "path", *path);
            Z_ADDREF_P(*path);
            if (real_path)
            {
                add_assoc_string(params, "realpath", real_path, 1);
            }
            else
            {
                zval *realpath;
                MAKE_STD_ZVAL(realpath);
                ZVAL_STRING(realpath, Z_STRVAL_PP(path), 1);
                add_assoc_zval(params, "realpath", realpath);
            }
            check("writeFile", params TSRMLS_CC);
        }
    }

}
void pre_global_fopen(INTERNAL_FUNCTION_PARAMETERS)
{
    char *filename, *mode;
	int filename_len, mode_len;
	zend_bool use_include_path = 0;
	zval *zcontext = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss|br", &filename, &filename_len, &mode, &mode_len, &use_include_path, &zcontext) == FAILURE) {
		return;
	}
    const char *type = mode_to_type(mode);
    if (!openrasp_check_type_ignored(type, strlen(type) TSRMLS_CC))
    {
        check_file_operation(type, filename, filename_len, use_include_path TSRMLS_CC);
    }
}

void pre_splfileobject___construct(INTERNAL_FUNCTION_PARAMETERS)
{
    char *filename, *mode;
	int filename_len, mode_len;
	zend_bool use_include_path = 0;
	zval *zcontext = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|sbr", &filename, &filename_len, &mode, &mode_len, &use_include_path, &zcontext) == FAILURE) {
		return;
	}
    const char *type = mode_to_type(mode);
    if (!openrasp_check_type_ignored(type, strlen(type) TSRMLS_CC))
    {
        check_file_operation(type, filename, filename_len, use_include_path TSRMLS_CC);
    }
}

void pre_global_copy(INTERNAL_FUNCTION_PARAMETERS)
{
    char *source, *target;
	int source_len, target_len;
	zval *zcontext = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss|r", &source, &source_len, &target, &target_len, &zcontext) == FAILURE) {
		return;
	}

    if (source && target && strlen(source) == source_len && strlen(target) == target_len) {
        char *real_path = php_resolve_path(target, target_len, NULL TSRMLS_CC);
        zval *params;
        MAKE_STD_ZVAL(params);
        array_init(params);
        add_assoc_string(params, "path", target, 1);
        add_assoc_string(params, "realpath", real_path ? real_path : target, 1);
        check("writeFile", params TSRMLS_CC);
	}
}