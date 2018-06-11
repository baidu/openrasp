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

#include "openrasp_v8.h"
using namespace openrasp;
static void url_getter(v8::Local<v8::Name> name, const v8::PropertyCallbackInfo<v8::Value> &info)
{
    info.GetReturnValue().SetEmptyString();
    TSRMLS_FETCH();

    if (Z_TYPE(PG(http_globals)[TRACK_VARS_SERVER]) != IS_ARRAY && !zend_is_auto_global_str(ZEND_STRL("_SERVER")))
    {
        return;
    }
    const HashTable *_SERVER = Z_ARRVAL(PG(http_globals)[TRACK_VARS_SERVER]);

    zval *REQUEST_SCHEME = zend_hash_str_find(_SERVER, ZEND_STRL("REQUEST_SCHEME"));
    zval *SERVER_NAME = zend_hash_str_find(_SERVER, ZEND_STRL("SERVER_NAME"));
    zval *HTTP_HOST = zend_hash_str_find(_SERVER, ZEND_STRL("HTTP_HOST"));
    zval *SERVER_ADDR = zend_hash_str_find(_SERVER, ZEND_STRL("SERVER_ADDR"));
    zval *SERVER_PORT = zend_hash_str_find(_SERVER, ZEND_STRL("SERVER_PORT"));
    zval *REQUEST_URI = zend_hash_str_find(_SERVER, ZEND_STRL("REQUEST_URI"));

    std::string url;
    if (REQUEST_SCHEME)
    {
        url.append(Z_STRVAL_P(REQUEST_SCHEME), Z_STRLEN_P(REQUEST_SCHEME));
    }
    url.append("://");
    if (HTTP_HOST)
    {
        url.append(Z_STRVAL_P(HTTP_HOST), Z_STRLEN_P(HTTP_HOST));
    }
    else
    {
        if (SERVER_NAME)
        {
            url.append(Z_STRVAL_P(SERVER_NAME), Z_STRLEN_P(SERVER_NAME));
        }
        else if (SERVER_ADDR)
        {
            url.append(Z_STRVAL_P(SERVER_ADDR), Z_STRLEN_P(SERVER_ADDR));
        }
        url.append(":");
        if (SERVER_PORT)
        {
            url.append(Z_STRVAL_P(SERVER_PORT), Z_STRLEN_P(SERVER_PORT));
        }
    }
    if (REQUEST_URI)
    {
        url.append(Z_STRVAL_P(REQUEST_URI), Z_STRLEN_P(REQUEST_URI));
    }

    v8::Isolate *isolate = info.GetIsolate();
    v8::Local<v8::String> v8_url;
    if (V8STRING_EX(url.c_str(), v8::NewStringType::kNormal, url.length()).ToLocal(&v8_url))
    {
        info.GetReturnValue().Set(v8_url);
    }
}
static void method_getter(v8::Local<v8::Name> name, const v8::PropertyCallbackInfo<v8::Value> &info)
{
    info.GetReturnValue().SetEmptyString();
    TSRMLS_FETCH();

    if (Z_TYPE(PG(http_globals)[TRACK_VARS_SERVER]) != IS_ARRAY && !zend_is_auto_global_str(ZEND_STRL("_SERVER")))
    {
        return;
    }
    const HashTable *_SERVER = Z_ARRVAL(PG(http_globals)[TRACK_VARS_SERVER]);

    zval *REQUEST_METHOD = zend_hash_str_find(_SERVER, ZEND_STRL("REQUEST_METHOD"));

    if (!REQUEST_METHOD)
    {
        return;
    }

    std::string method(Z_STRVAL_P(REQUEST_METHOD), Z_STRLEN_P(REQUEST_METHOD));
    for (auto &ch : method)
    {
        ch = std::tolower(ch);
    }

    v8::Isolate *isolate = info.GetIsolate();
    v8::Local<v8::String> v8_method;
    if (V8STRING_EX(method.c_str(), v8::NewStringType::kNormal, method.length()).ToLocal(&v8_method))
    {
        info.GetReturnValue().Set(v8_method);
    }
}
static void querystring_getter(v8::Local<v8::Name> name, const v8::PropertyCallbackInfo<v8::Value> &info)
{
    info.GetReturnValue().SetEmptyString();
    TSRMLS_FETCH();

    if (Z_TYPE(PG(http_globals)[TRACK_VARS_SERVER]) != IS_ARRAY && !zend_is_auto_global_str(ZEND_STRL("_SERVER")))
    {
        return;
    }
    const HashTable *_SERVER = Z_ARRVAL(PG(http_globals)[TRACK_VARS_SERVER]);

    zval *QUERY_STRING = zend_hash_str_find(_SERVER, ZEND_STRL("QUERY_STRING"));

    info.GetReturnValue().Set(zval_to_v8val(QUERY_STRING, info.GetIsolate() TSRMLS_CC));
}
static void appBasePath_getter(v8::Local<v8::Name> name, const v8::PropertyCallbackInfo<v8::Value> &info)
{
    info.GetReturnValue().SetEmptyString();
    TSRMLS_FETCH();

    if (Z_TYPE(PG(http_globals)[TRACK_VARS_SERVER]) != IS_ARRAY && !zend_is_auto_global_str(ZEND_STRL("_SERVER")))
    {
        return;
    }
    const HashTable *_SERVER = Z_ARRVAL(PG(http_globals)[TRACK_VARS_SERVER]);

    zval *DOCUMENT_ROOT = zend_hash_str_find(_SERVER, ZEND_STRL("DOCUMENT_ROOT"));

    info.GetReturnValue().Set(zval_to_v8val(DOCUMENT_ROOT, info.GetIsolate() TSRMLS_CC));
}
static void protocol_getter(v8::Local<v8::Name> name, const v8::PropertyCallbackInfo<v8::Value> &info)
{
    info.GetReturnValue().SetEmptyString();
    TSRMLS_FETCH();

    if (Z_TYPE(PG(http_globals)[TRACK_VARS_SERVER]) != IS_ARRAY && !zend_is_auto_global_str(ZEND_STRL("_SERVER")))
    {
        return;
    }
    const HashTable *_SERVER = Z_ARRVAL(PG(http_globals)[TRACK_VARS_SERVER]);

    zval *REQUEST_SCHEME = zend_hash_str_find(_SERVER, ZEND_STRL("REQUEST_SCHEME"));

    info.GetReturnValue().Set(zval_to_v8val(REQUEST_SCHEME, info.GetIsolate() TSRMLS_CC));
}
static void remoteAddr_getter(v8::Local<v8::Name> name, const v8::PropertyCallbackInfo<v8::Value> &info)
{
    info.GetReturnValue().SetEmptyString();
    TSRMLS_FETCH();

    if (Z_TYPE(PG(http_globals)[TRACK_VARS_SERVER]) != IS_ARRAY && !zend_is_auto_global_str(ZEND_STRL("_SERVER")))
    {
        return;
    }
    const HashTable *_SERVER = Z_ARRVAL(PG(http_globals)[TRACK_VARS_SERVER]);

    zval *REMOTE_ADDR = zend_hash_str_find(_SERVER, ZEND_STRL("REMOTE_ADDR"));

    info.GetReturnValue().Set(zval_to_v8val(REMOTE_ADDR, info.GetIsolate() TSRMLS_CC));
}
static void path_getter(v8::Local<v8::Name> name, const v8::PropertyCallbackInfo<v8::Value> &info)
{
    info.GetReturnValue().SetEmptyString();
    TSRMLS_FETCH();

    if (Z_TYPE(PG(http_globals)[TRACK_VARS_SERVER]) != IS_ARRAY && !zend_is_auto_global_str(ZEND_STRL("_SERVER")))
    {
        return;
    }
    const HashTable *_SERVER = Z_ARRVAL(PG(http_globals)[TRACK_VARS_SERVER]);

    zval *REQUEST_URI = zend_hash_str_find(_SERVER, ZEND_STRL("REQUEST_URI"));

    if (!REQUEST_URI)
    {
        return;
    }

    std::string path(Z_STRVAL_P(REQUEST_URI), Z_STRLEN_P(REQUEST_URI));
    size_t len = path.find_first_of('?');

    v8::Isolate *isolate = info.GetIsolate();
    v8::Local<v8::String> v8_path;
    if (V8STRING_EX(path.c_str(), v8::NewStringType::kNormal, len != std::string::npos ? len : path.length()).ToLocal(&v8_path))
    {
        info.GetReturnValue().Set(v8_path);
    }
}
static void parameter_getter(v8::Local<v8::Name> name, const v8::PropertyCallbackInfo<v8::Value> &info)
{
    TSRMLS_FETCH();
    if ((Z_TYPE(PG(http_globals)[TRACK_VARS_GET]) != IS_ARRAY && !zend_is_auto_global_str(ZEND_STRL("_GET"))) ||
        (Z_TYPE(PG(http_globals)[TRACK_VARS_POST]) != IS_ARRAY && !zend_is_auto_global_str(ZEND_STRL("_POST"))))
    {
        return;
    }
    const HashTable *_GET = Z_ARRVAL(PG(http_globals)[TRACK_VARS_GET]);
    const HashTable *_POST = Z_ARRVAL(PG(http_globals)[TRACK_VARS_POST]);

    v8::Isolate *isolate = info.GetIsolate();
    v8::Local<v8::Object> obj = v8::Object::New(isolate);

    zval *value;
    zend_string *key;
    zend_ulong idx;
    ZEND_HASH_FOREACH_KEY_VAL(_GET, idx, key, value)
    {
        v8::Local<v8::Value> v8_value = zval_to_v8val(value, isolate TSRMLS_CC);
        if (v8_value->IsNullOrUndefined())
        {
            continue;
        }
        if (!v8_value->IsArray())
        {
            v8::Local<v8::Array> v8_arr = v8::Array::New(isolate);
            v8_arr->Set(0, v8_value);
            v8_value = v8_arr;
        }
        if (key)
        {
            obj->Set(V8STRING_I(key->val).ToLocalChecked(), v8_value);
        }
        else
        {
            obj->Set(idx, v8_value);
        }
    }
    ZEND_HASH_FOREACH_END();

    ZEND_HASH_FOREACH_KEY_VAL(_POST, idx, key, value)
    {
        v8::Local<v8::Value> v8_value = zval_to_v8val(value, isolate TSRMLS_CC);
        if (v8_value->IsNullOrUndefined())
        {
            continue;
        }
        if (!v8_value->IsArray())
        {
            v8::Local<v8::Array> v8_arr = v8::Array::New(isolate);
            v8_arr->Set(0, v8_value);
            v8_value = v8_arr;
        }
        v8::Local<v8::Value> v8_key;
        if (key)
        {
            v8_key = V8STRING_I(key->val).ToLocalChecked();
        }
        else
        {
            v8_key = v8::Integer::New(isolate, idx);
        }
        v8::Local<v8::Value> v8_existed_value = obj->Get(v8_key);
        if (!v8_existed_value.IsEmpty() &&
            v8_existed_value->IsArray())
        {
            v8::Local<v8::Array> v8_arr1 = v8_existed_value.As<v8::Array>();
            int v8_arr1_len = v8_arr1->Length();
            v8::Local<v8::Array> v8_arr2 = v8_value.As<v8::Array>();
            int v8_arr2_len = v8_arr2->Length();
            v8::Local<v8::Array> v8_arr = v8::Array::New(isolate, v8_arr1_len + v8_arr2_len);
            for (int i = 0; i < v8_arr1_len; i++)
            {
                v8_arr->Set(i, v8_arr1->Get(i));
            }
            for (int i = 0; i < v8_arr2_len; i++)
            {
                v8_arr->Set(v8_arr1_len + i, v8_arr2->Get(i));
            }
            v8_value = v8_arr;
        }
        obj->Set(v8_key, v8_value);
    }
    ZEND_HASH_FOREACH_END();

    info.GetReturnValue().Set(obj);
}
static void header_getter(v8::Local<v8::Name> name, const v8::PropertyCallbackInfo<v8::Value> &info)
{
    TSRMLS_FETCH();
    if (Z_TYPE(PG(http_globals)[TRACK_VARS_SERVER]) != IS_ARRAY && !zend_is_auto_global_str(ZEND_STRL("_SERVER")))
    {
        return;
    }
    const HashTable *_SERVER = Z_ARRVAL(PG(http_globals)[TRACK_VARS_SERVER]);

    v8::Isolate *isolate = info.GetIsolate();
    v8::Local<v8::Object> obj = v8::Object::New(isolate);

    zval *value;
    zend_string *key;
    ZEND_HASH_FOREACH_STR_KEY_VAL(_SERVER, key, value)
    {
        if (key == NULL ||
            strncmp(key->val, "HTTP_", 5) != 0)
        {
            continue;
        }
        std::string tmp(key->val + 5, key->len - 5);
        for (auto &ch : tmp)
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
        v8::Local<v8::String> v8_key;
        if (V8STRING_EX(tmp.c_str(), v8::NewStringType::kInternalized, tmp.length()).ToLocal(&v8_key))
        {
            obj->Set(v8_key, zval_to_v8val(value, isolate TSRMLS_CC));
        }
    }
    ZEND_HASH_FOREACH_END();

    info.GetReturnValue().Set(obj);
}
static void body_getter(v8::Local<v8::Name> name, const v8::PropertyCallbackInfo<v8::Value> &info)
{
    info.GetReturnValue().Set(v8::ArrayBuffer::New(info.GetIsolate(), nullptr, 0, v8::ArrayBufferCreationMode::kInternalized));
    TSRMLS_FETCH();
    php_stream *stream = php_stream_open_wrapper("php://input", "rb", 0, NULL);
    if (!stream)
    {
        return;
    }

    size_t len = 0, maxlen = 4 * 1024;
    char *buffer = (char *)malloc(maxlen);
    char *ptr = buffer;
    while ((len < maxlen) && !php_stream_eof(stream))
    {
        size_t ret = php_stream_read(stream, ptr, maxlen - len);
        if (!ret)
        {
            break;
        }
        len += ret;
        ptr += ret;
    }
    if (len)
    {
        *ptr = '\0';
    }
    else
    {
        free(buffer);
        buffer = nullptr;
    }

    stream->is_persistent ? php_stream_pclose(stream) : php_stream_close(stream);
    if (!buffer)
    {
        return;
    }
    v8::Isolate *isolate = info.GetIsolate();
    v8::Local<v8::ArrayBuffer> arraybuffer = v8::ArrayBuffer::New(isolate, buffer, MIN(len, maxlen), v8::ArrayBufferCreationMode::kInternalized);
    info.GetReturnValue().Set(arraybuffer);
}
static void server_getter(v8::Local<v8::Name> name, const v8::PropertyCallbackInfo<v8::Value> &info)
{
    TSRMLS_FETCH();
    v8::Isolate *isolate = info.GetIsolate();
    v8::Local<v8::Object> server = v8::Object::New(isolate);
    server->Set(V8STRING_I("language").ToLocalChecked(), V8STRING_N("php").ToLocalChecked());
    server->Set(V8STRING_I("name").ToLocalChecked(), V8STRING_N("PHP").ToLocalChecked());
    server->Set(V8STRING_I("version").ToLocalChecked(), V8STRING_N(OPENRASP_PHP_VERSION).ToLocalChecked());
#ifdef PHP_WIN32
    server->Set(V8STRING_I("os").ToLocalChecked(), V8STRING_N("Windows").ToLocalChecked());
#else
    if (strstr(PHP_OS, "Darwin"))
    {
        server->Set(V8STRING_I("os").ToLocalChecked(), V8STRING_N("Mac").ToLocalChecked());
    }
    else if (strstr(PHP_OS, "Linux"))
    {
        server->Set(V8STRING_I("os").ToLocalChecked(), V8STRING_N("Linux").ToLocalChecked());
    }
    else
    {
        server->Set(V8STRING_I("os").ToLocalChecked(), V8STRING_N(PHP_OS).ToLocalChecked());
    }
#endif
    info.GetReturnValue().Set(server);
}
v8::Local<v8::Object> openrasp::RequestContext::New(v8::Isolate *isolate)
{
    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    v8::Local<v8::Object> obj = v8::Object::New(isolate);
    obj->SetAccessor(context, V8STRING_I("url").ToLocalChecked(), url_getter).IsNothing();
    obj->SetAccessor(context, V8STRING_I("header").ToLocalChecked(), header_getter).IsNothing();
    obj->SetAccessor(context, V8STRING_I("parameter").ToLocalChecked(), parameter_getter).IsNothing();
    obj->SetAccessor(context, V8STRING_I("path").ToLocalChecked(), path_getter).IsNothing();
    obj->SetAccessor(context, V8STRING_I("querystring").ToLocalChecked(), querystring_getter).IsNothing();
    obj->SetAccessor(context, V8STRING_I("method").ToLocalChecked(), method_getter).IsNothing();
    obj->SetAccessor(context, V8STRING_I("protocol").ToLocalChecked(), protocol_getter).IsNothing();
    obj->SetAccessor(context, V8STRING_I("remoteAddr").ToLocalChecked(), remoteAddr_getter).IsNothing();
    obj->SetAccessor(context, V8STRING_I("appBasePath").ToLocalChecked(), appBasePath_getter).IsNothing();
    obj->SetAccessor(context, V8STRING_I("body").ToLocalChecked(), body_getter).IsNothing();
    obj->SetAccessor(context, V8STRING_I("server").ToLocalChecked(), server_getter).IsNothing();
    return obj;
}