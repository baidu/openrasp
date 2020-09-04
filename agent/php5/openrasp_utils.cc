/*
 * Copyright 2017-2020 Baidu Inc.
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

#include "openrasp_utils.h"
#include "openrasp_ini.h"
#include <cmath>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include "openrasp_log.h"
#include "utils/debug_trace.h"
#include <unordered_set>
#include "utils/regex.h"

extern "C"
{
#include "php_ini.h"
#include "php_main.h"
#include "php_streams.h"
#include "ext/json/php_json.h"
#include "ext/standard/url.h"
#include "ext/standard/file.h"
#include "ext/date/php_date.h"
#include "ext/standard/php_string.h"
#include "ext/standard/php_smart_str.h"
#include "Zend/zend_builtin_functions.h"
#include "zend_extensions.h"
}

using openrasp::DebugTrace;

static std::vector<DebugTrace> build_debug_trace(long limit TSRMLS_DC)
{
    zval trace_arr;
#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION <= 3)
    zend_fetch_debug_backtrace(&trace_arr, 0, 0 TSRMLS_CC);
#else
    zend_fetch_debug_backtrace(&trace_arr, 0, 0, 0 TSRMLS_CC);
#endif
    std::vector<DebugTrace> array;
    if (Z_TYPE(trace_arr) == IS_ARRAY)
    {
        int i = 0;
        HashTable *hash_arr = Z_ARRVAL(trace_arr);
        for (zend_hash_internal_pointer_reset(hash_arr);
             zend_hash_has_more_elements(hash_arr) == SUCCESS;
             zend_hash_move_forward(hash_arr))
        {
            if (++i > limit)
            {
                break;
            }
            zval **ele_value;
            if (zend_hash_get_current_data(hash_arr, (void **)&ele_value) != SUCCESS ||
                Z_TYPE_PP(ele_value) != IS_ARRAY)
            {
                continue;
            }
            DebugTrace trace_item;
            zval **trace_ele;
            if (zend_hash_find(Z_ARRVAL_PP(ele_value), ZEND_STRS("file"), (void **)&trace_ele) == SUCCESS &&
                Z_TYPE_PP(trace_ele) == IS_STRING)
            {
                trace_item.set_file(Z_STRVAL_PP(trace_ele));
            }
            if (zend_hash_find(Z_ARRVAL_PP(ele_value), ZEND_STRS("function"), (void **)&trace_ele) == SUCCESS &&
                Z_TYPE_PP(trace_ele) == IS_STRING)
            {
                trace_item.set_function(Z_STRVAL_PP(trace_ele));
            }
            //line number
            if (zend_hash_find(Z_ARRVAL_PP(ele_value), ZEND_STRS("line"), (void **)&trace_ele) == SUCCESS &&
                Z_TYPE_PP(trace_ele) == IS_LONG)
            {
                trace_item.set_line(Z_LVAL_PP(trace_ele));
            }
            array.push_back(trace_item);
        }
    }
    zval_dtor(&trace_arr);
    return array;
}

std::vector<std::string> format_source_code_arr()
{
    TSRMLS_FETCH();
    std::vector<DebugTrace> trace = build_debug_trace(OPENRASP_CONFIG(plugin.maxstack) TSRMLS_CC);
    std::vector<std::string> array;
    for (DebugTrace &item : trace)
    {
        array.push_back(item.get_source_code());
    }
    return array;
}

std::vector<std::string> format_debug_backtrace_arr()
{
    TSRMLS_FETCH();
    return format_debug_backtrace_arr(OPENRASP_CONFIG(plugin.maxstack) TSRMLS_CC);
}

std::vector<std::string> format_debug_backtrace_arr(long limit TSRMLS_DC)
{
    std::vector<DebugTrace> trace = build_debug_trace(limit TSRMLS_CC);
    std::vector<std::string> array;
    for (DebugTrace &item : trace)
    {
        array.push_back(item.to_log_string());
    }
    return array;
}

int recursive_mkdir(const char *path, int len, int mode TSRMLS_DC)
{
    struct stat sb;
    if (VCWD_STAT(path, &sb) == 0 && (sb.st_mode & S_IFDIR) != 0)
    {
        return 1;
    }
    char *dirname = estrndup(path, len);
    int dirlen = php_dirname(dirname, len);
    int rst = recursive_mkdir(dirname, dirlen, mode TSRMLS_CC);
    efree(dirname);
    if (rst)
    {
#ifndef PHP_WIN32
        mode_t oldmask = umask(0);
        rst = VCWD_MKDIR(path, mode);
        umask(oldmask);
#else
        rst = VCWD_MKDIR(path, mode);
#endif
        if (rst == 0 || EEXIST == errno)
        {
            return 1;
        }
        openrasp_error(LEVEL_WARNING, RUNTIME_ERROR, _("Could not create directory '%s': %s"), path, strerror(errno));
    }
    return 0;
}

const char *determine_scheme_pos(const char *filename)
{
    if (nullptr == filename)
    {
        return nullptr;
    }
    const char *p;
    for (p = filename; isalnum((int)*p) || *p == '+' || *p == '-' || *p == '.'; p++)
    {
        //skip number "+" "-" "."
    }
    if ((*p == ':') && (p - filename > 1) && (p[1] == '/') && (p[2] == '/'))
    {
        return p;
    }
    return nullptr;
}

std::string fetch_possible_protocol(const char *filename)
{
    std::string protocol;
    const char *scheme_pos = determine_scheme_pos(filename);
    if (nullptr != scheme_pos)
    {
        protocol = std::string(filename, scheme_pos - filename);
    }
    return protocol;
}

std::string fetch_outmost_string_from_ht(HashTable *ht, const char *arKey)
{
    std::string result;
    zval **origin_zv;
    if (zend_hash_find(ht, arKey, strlen(arKey) + 1, (void **)&origin_zv) == SUCCESS &&
        Z_TYPE_PP(origin_zv) == IS_STRING)
    {
        result = std::string(Z_STRVAL_PP(origin_zv), Z_STRLEN_PP(origin_zv));
    }
    return result;
}

std::string json_encode_from_zval(zval *value TSRMLS_DC)
{
    smart_str buf_json = {0};
    php_json_encode(&buf_json, value, 0 TSRMLS_CC);
    if (buf_json.a > buf_json.len)
    {
        buf_json.c[buf_json.len] = '\0';
        buf_json.len++;
    }
    std::string result(buf_json.c);
    smart_str_free(&buf_json);
    return result;
}

char *fetch_request_body(size_t max_len TSRMLS_DC)
{
    php_stream *stream = php_stream_open_wrapper("php://input", "rb", 0, nullptr);
    if (stream)
    {
        char *buf = nullptr;
        int len = php_stream_copy_to_mem(stream, &buf, max_len, 0);
        php_stream_close(stream);
        if (len > 0 && buf != nullptr)
        {
            return buf;
        }
    }
    return estrdup("");
}

bool need_alloc_shm_current_sapi()
{
    static const char *supported_sapis[] = {
        "cgi-fcgi",
        "fpm-fcgi",
        "apache2handler",
        nullptr};
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

std::string convert_to_header_key(char *key, size_t length)
{
    std::string result;
    if (nullptr == key)
    {
        return result;
    }
    if (strcmp("HTTP_CONTENT_TYPE", key) == 0 || strcmp("CONTENT_TYPE", key) == 0)
    {
        result = "content-type";
    }
    else if (strcmp("HTTP_CONTENT_LENGTH", key) == 0 || strcmp("CONTENT_LENGTH", key) == 0)
    {
        result = "content-length";
    }
    else if (strncmp(key, "HTTP_", 5) == 0)
    {
        std::string http_header(key + 5, length - 5);
        for (auto &ch : http_header)
        {
            if (ch == '_')
            {
                ch = '-';
            }
            else
            {
                ch = std::tolower(ch);
            }
        }
        result = http_header;
    }
    return result;
}

bool openrasp_parse_url(const std::string &origin_url, openrasp::Url &openrasp_url)
{
    php_url *url = php_url_parse_ex(origin_url.c_str(), origin_url.length());
    if (url)
    {
        if (url->scheme)
        {
            openrasp_url.set_scheme(url->scheme);
        }
        if (url->host)
        {
            openrasp_url.set_host(url->host);
        }
        if (url->port)
        {
            openrasp_url.set_port(std::to_string(url->port));
        }
        if (url->path)
        {
            openrasp_url.set_path(url->path);
        }
        if (url->query)
        {
            openrasp_url.set_query(url->query);
        }
        php_url_free(url);
        return true;
    }
    return false;
}

bool make_openrasp_root_dir(const char *path TSRMLS_DC)
{
    if (!path)
    {
        openrasp_error(LEVEL_WARNING, CONFIG_ERROR, _("openrasp.root_dir must not be an empty path"));
        return false;
    }
    if (!IS_ABSOLUTE_PATH(path, strlen(path)))
    {
        openrasp_error(LEVEL_WARNING, CONFIG_ERROR, _("openrasp.root_dir must not be a relative path"));
        return false;
    }
    char expand_root_path[MAXPATHLEN];
    expand_filepath(path, expand_root_path TSRMLS_CC);
    if (!expand_root_path || strnlen(expand_root_path, 2) == 1)
    {
        openrasp_error(LEVEL_WARNING, CONFIG_ERROR, _("openrasp.root_dir must not be a root path"));
        return false;
    }
    std::string root_dir(expand_root_path);
    std::string default_slash(1, DEFAULT_SLASH);
    std::vector<std::string> sub_dir_list{
        "assets",
        "conf",
        "plugins",
        "locale",
        "logs" + default_slash + RaspLoggerEntry::ALARM_LOG_DIR_NAME,
        "logs" + default_slash + RaspLoggerEntry::POLICY_LOG_DIR_NAME,
        "logs" + default_slash + RaspLoggerEntry::PLUGIN_LOG_DIR_NAME,
        "logs" + default_slash + RaspLoggerEntry::RASP_LOG_DIR_NAME,
        "logs" + default_slash + "crash"};
    for (auto dir : sub_dir_list)
    {
        std::string sub_path(root_dir + DEFAULT_SLASH + dir);
        if (!recursive_mkdir(sub_path.c_str(), sub_path.length(), 0777 TSRMLS_CC))
        {
            openrasp_error(LEVEL_WARNING, RUNTIME_ERROR, _("openrasp.root_dir must be a writable path"));
            return false;
        }
    }
    return true;
}

void openrasp_set_locale(const char *locale, const char *locale_path)
{
#ifdef HAVE_GETTEXT
    static const char *GETTEXT_PACKAGE = "openrasp";
    if (nullptr != setlocale(LC_ALL, locale ? locale : "C"))
    {
        if (!bindtextdomain(GETTEXT_PACKAGE, locale_path))
        {
            openrasp_error(LEVEL_WARNING, CONFIG_ERROR, _("bindtextdomain() failed: %s"), strerror(errno));
        }
        if (!textdomain(GETTEXT_PACKAGE))
        {
            openrasp_error(LEVEL_WARNING, CONFIG_ERROR, _("textdomain() failed: %s"), strerror(errno));
        }
    }
    else
    {
        openrasp_error(LEVEL_WARNING, CONFIG_ERROR, _("Unable to set OpenRASP locale to %s"), locale);
    }
#endif
}

bool current_sapi_supported()
{
    const static std::unordered_set<std::string> supported_sapis =
        {
#ifdef HAVE_CLI_SUPPORT
            "cli",
#endif
            "cli-server",
            "cgi-fcgi",
            "fpm-fcgi",
            "apache2handler"};
    auto iter = supported_sapis.find(std::string(sapi_module.name));
    return iter != supported_sapis.end();
}

std::map<std::string, std::string> get_env_map()
{
    std::map<std::string, std::string> result;
    char **env;
    for (env = environ; env != nullptr && *env != nullptr; env++)
    {
        std::string item(*env);
        std::size_t found = item.find("=");
        if (found != std::string::npos)
        {
            std::string key = item.substr(0, found);
            std::string value = item.substr(found + 1);
            result.insert({key, value});
        }
    }
    return result;
}

std::string get_phpversion()
{
    std::string version;
    TSRMLS_FETCH();
    zval z_version;
    if (zend_get_constant("PHP_VERSION", sizeof("PHP_VERSION") - 1, &z_version TSRMLS_CC))
    {
        if (Z_TYPE(z_version) == IS_STRING)
        {
            version = std::string(Z_STRVAL(z_version), Z_STRLEN(z_version));
        }
        zval_dtor(&z_version);
    }
    return version;
}

bool openrasp_call_user_function(HashTable *function_table, zval **object_pp, const std::string &function_name,
                                 zval *retval_ptr, zend_uint param_count, zval *params[] TSRMLS_DC)
{
    zval z_function_name;
    INIT_ZVAL(z_function_name);
    ZVAL_STRING(&z_function_name, function_name.c_str(), 0);
    if (call_user_function(EG(function_table), object_pp, &z_function_name, retval_ptr,
                           param_count, params TSRMLS_CC) == SUCCESS)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool get_long_constant(const std::string &key, long &value)
{
    TSRMLS_FETCH();
    bool found = false;
    zval z_result;
    if (zend_get_constant(key.c_str(), key.length(), &z_result TSRMLS_CC))
    {
        if (Z_TYPE(z_result) == IS_LONG)
        {
            value = Z_LVAL(z_result);
            found = true;
        }
        zval_dtor(&z_result);
    }
    return found;
}

zval *fetch_http_globals(int vars_id TSRMLS_DC)
{
    static std::map<int, std::string> pairs = {{TRACK_VARS_POST, "_POST"},
                                               {TRACK_VARS_GET, "_GET"},
                                               {TRACK_VARS_SERVER, "_SERVER"},
                                               {TRACK_VARS_COOKIE, "_COOKIE"},
                                               {TRACK_VARS_FILES, "_FILES"}};
    auto it = pairs.find(vars_id);
    if (it != pairs.end())
    {
        if ((PG(http_globals)[vars_id] && Z_TYPE_P(PG(http_globals)[vars_id]) == IS_ARRAY) ||
            zend_is_auto_global(const_cast<char *>(it->second.c_str()), it->second.length() TSRMLS_CC))
        {
            return PG(http_globals)[it->first];
        }
    }
    return nullptr;
}

bool maybe_ssrf_vulnerability(zval *file)
{
    if (nullptr != file &&
        Z_TYPE_P(file) == IS_STRING &&
        Z_STRLEN_P(file) > 0)
    {
        std::string protocol = fetch_possible_protocol(Z_STRVAL_P(file));
        return maybe_ssrf_vulnerability(protocol);
    }
    return false;
}

bool maybe_ssrf_vulnerability(std::string protocol)
{
    static std::unordered_set<std::string> protocols = {"http", "https", "ftp"};
    if (protocol.empty())
    {
        return false;
    }
    for (auto &ch : protocol)
    {
        ch = std::tolower(ch);
    }
    auto found = protocols.find(protocol);
    return found != protocols.end();
}