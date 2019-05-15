/*
 * Copyright 2017-2019 Baidu Inc.
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
#include "openrasp_log.h"
#include "openrasp_utils.h"
#include "openrasp_content_type.h"
#include "openrasp_inject.h"

using namespace openrasp;

enum FieldIndex
{
    kUrl = 0,
    kHeader,
    kParameter,
    kPath,
    kQuerystring,
    kMethod,
    kProtocol,
    kRemoteAddr,
    kAppBasePath,
    kBody,
    kServer,
    kJsonBody,
    kEndForCount
};
static void url_getter(v8::Local<v8::Name> name, const v8::PropertyCallbackInfo<v8::Value> &info)
{
    auto self = info.Holder();
    auto cache = self->GetInternalField(kUrl);
    if (!cache->IsUndefined())
    {
        info.GetReturnValue().Set(cache);
        return;
    }
    info.GetReturnValue().SetEmptyString();
    zval *origin_zv;
    zval *alarm_common_info = LOG_G(alarm_logger).get_common_info();
    if (Z_TYPE_P(alarm_common_info) == IS_ARRAY &&
        (origin_zv = zend_hash_str_find(Z_ARRVAL_P(alarm_common_info), ZEND_STRL("url"))) != nullptr)
    {
        v8::Isolate *isolate = info.GetIsolate();
        auto obj = NewV8ValueFromZval(isolate, origin_zv);
        info.GetReturnValue().Set(obj);
        self->SetInternalField(kUrl, obj);
    }
}
static void method_getter(v8::Local<v8::Name> name, const v8::PropertyCallbackInfo<v8::Value> &info)
{
    auto self = info.Holder();
    auto cache = self->GetInternalField(kMethod);
    if (!cache->IsUndefined())
    {
        info.GetReturnValue().Set(cache);
        return;
    }
    info.GetReturnValue().SetEmptyString();

    if (Z_TYPE(PG(http_globals)[TRACK_VARS_SERVER]) != IS_ARRAY && !zend_is_auto_global_str(ZEND_STRL("_SERVER")))
    {
        return;
    }
    HashTable *_SERVER = Z_ARRVAL(PG(http_globals)[TRACK_VARS_SERVER]);

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
    auto obj = NewV8String(isolate, method);
    info.GetReturnValue().Set(obj);
    self->SetInternalField(kMethod, obj);
}
static void querystring_getter(v8::Local<v8::Name> name, const v8::PropertyCallbackInfo<v8::Value> &info)
{
    auto self = info.Holder();
    auto cache = self->GetInternalField(kQuerystring);
    if (!cache->IsUndefined())
    {
        info.GetReturnValue().Set(cache);
        return;
    }
    info.GetReturnValue().SetEmptyString();

    if (Z_TYPE(PG(http_globals)[TRACK_VARS_SERVER]) != IS_ARRAY && !zend_is_auto_global_str(ZEND_STRL("_SERVER")))
    {
        return;
    }
    HashTable *_SERVER = Z_ARRVAL(PG(http_globals)[TRACK_VARS_SERVER]);

    zval *QUERY_STRING = zend_hash_str_find(_SERVER, ZEND_STRL("QUERY_STRING"));
    if (QUERY_STRING)
    {
        auto obj = NewV8ValueFromZval(info.GetIsolate(), QUERY_STRING);
        info.GetReturnValue().Set(obj);
        self->SetInternalField(kQuerystring, obj);
    }
}
static void appBasePath_getter(v8::Local<v8::Name> name, const v8::PropertyCallbackInfo<v8::Value> &info)
{
    auto self = info.Holder();
    auto cache = self->GetInternalField(kAppBasePath);
    if (!cache->IsUndefined())
    {
        info.GetReturnValue().Set(cache);
        return;
    }
    info.GetReturnValue().SetEmptyString();

    if (Z_TYPE(PG(http_globals)[TRACK_VARS_SERVER]) != IS_ARRAY && !zend_is_auto_global_str(ZEND_STRL("_SERVER")))
    {
        return;
    }
    HashTable *_SERVER = Z_ARRVAL(PG(http_globals)[TRACK_VARS_SERVER]);

    zval *DOCUMENT_ROOT = zend_hash_str_find(_SERVER, ZEND_STRL("DOCUMENT_ROOT"));
    if (DOCUMENT_ROOT)
    {
        auto obj = NewV8ValueFromZval(info.GetIsolate(), DOCUMENT_ROOT);
        info.GetReturnValue().Set(obj);
        self->SetInternalField(kAppBasePath, obj);
    }
}
static void protocol_getter(v8::Local<v8::Name> name, const v8::PropertyCallbackInfo<v8::Value> &info)
{
    auto self = info.Holder();
    auto cache = self->GetInternalField(kProtocol);
    if (!cache->IsUndefined())
    {
        info.GetReturnValue().Set(cache);
        return;
    }
    info.GetReturnValue().SetEmptyString();

    if (Z_TYPE(PG(http_globals)[TRACK_VARS_SERVER]) != IS_ARRAY && !zend_is_auto_global_str(ZEND_STRL("_SERVER")))
    {
        return;
    }
    HashTable *_SERVER = Z_ARRVAL(PG(http_globals)[TRACK_VARS_SERVER]);

    zval *REQUEST_SCHEME = zend_hash_str_find(_SERVER, ZEND_STRL("REQUEST_SCHEME"));
    if (REQUEST_SCHEME)
    {
        auto obj = NewV8ValueFromZval(info.GetIsolate(), REQUEST_SCHEME);
        info.GetReturnValue().Set(obj);
        self->SetInternalField(kProtocol, obj);
    }
}
static void remoteAddr_getter(v8::Local<v8::Name> name, const v8::PropertyCallbackInfo<v8::Value> &info)
{
    auto self = info.Holder();
    auto cache = self->GetInternalField(kRemoteAddr);
    if (!cache->IsUndefined())
    {
        info.GetReturnValue().Set(cache);
        return;
    }
    info.GetReturnValue().SetEmptyString();

    if (Z_TYPE(PG(http_globals)[TRACK_VARS_SERVER]) != IS_ARRAY && !zend_is_auto_global_str(ZEND_STRL("_SERVER")))
    {
        return;
    }
    HashTable *_SERVER = Z_ARRVAL(PG(http_globals)[TRACK_VARS_SERVER]);

    zval *REMOTE_ADDR = zend_hash_str_find(_SERVER, ZEND_STRL("REMOTE_ADDR"));
    if (REMOTE_ADDR)
    {
        auto obj = NewV8ValueFromZval(info.GetIsolate(), REMOTE_ADDR);
        info.GetReturnValue().Set(obj);
        self->SetInternalField(kRemoteAddr, obj);
    }
}
static void path_getter(v8::Local<v8::Name> name, const v8::PropertyCallbackInfo<v8::Value> &info)
{
    auto self = info.Holder();
    auto cache = self->GetInternalField(kPath);
    if (!cache->IsUndefined())
    {
        info.GetReturnValue().Set(cache);
        return;
    }
    info.GetReturnValue().SetEmptyString();

    if (Z_TYPE(PG(http_globals)[TRACK_VARS_SERVER]) != IS_ARRAY && !zend_is_auto_global_str(ZEND_STRL("_SERVER")))
    {
        return;
    }
    HashTable *_SERVER = Z_ARRVAL(PG(http_globals)[TRACK_VARS_SERVER]);

    zval *REQUEST_URI = zend_hash_str_find(_SERVER, ZEND_STRL("REQUEST_URI"));

    if (!REQUEST_URI)
    {
        return;
    }

    std::string path(Z_STRVAL_P(REQUEST_URI), Z_STRLEN_P(REQUEST_URI));
    size_t len = path.find_first_of('?');

    v8::Isolate *isolate = info.GetIsolate();
    auto obj = NewV8String(isolate, path.c_str(), len != std::string::npos ? len : path.length());
    info.GetReturnValue().Set(obj);
    self->SetInternalField(kPath, obj);
}
static void parameter_getter(v8::Local<v8::Name> name, const v8::PropertyCallbackInfo<v8::Value> &info)
{
    auto self = info.Holder();
    auto cache = self->GetInternalField(kParameter);
    if (!cache->IsUndefined())
    {
        info.GetReturnValue().Set(cache);
        return;
    }
    if ((Z_TYPE(PG(http_globals)[TRACK_VARS_GET]) != IS_ARRAY && !zend_is_auto_global_str(ZEND_STRL("_GET"))) ||
        (Z_TYPE(PG(http_globals)[TRACK_VARS_POST]) != IS_ARRAY && !zend_is_auto_global_str(ZEND_STRL("_POST"))))
    {
        return;
    }
    HashTable *_GET = Z_ARRVAL(PG(http_globals)[TRACK_VARS_GET]);
    HashTable *_POST = Z_ARRVAL(PG(http_globals)[TRACK_VARS_POST]);

    v8::Isolate *isolate = info.GetIsolate();
    v8::Local<v8::Object> obj = v8::Object::New(isolate);

    zval *value;
    zend_string *key;
    zend_ulong idx;
    ZEND_HASH_FOREACH_KEY_VAL(_GET, idx, key, value)
    {
        v8::Local<v8::Value> v8_value = NewV8ValueFromZval(isolate, value);
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
            obj->Set(NewV8String(isolate, key->val), v8_value);
        }
        else
        {
            obj->Set(idx, v8_value);
        }
    }
    ZEND_HASH_FOREACH_END();

    ZEND_HASH_FOREACH_KEY_VAL(_POST, idx, key, value)
    {
        v8::Local<v8::Value> v8_value = NewV8ValueFromZval(isolate, value);
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
            v8_key = NewV8String(isolate, key->val);
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
    self->SetInternalField(kParameter, obj);
}
static void header_getter(v8::Local<v8::Name> name, const v8::PropertyCallbackInfo<v8::Value> &info)
{
    auto self = info.Holder();
    auto cache = self->GetInternalField(kHeader);
    if (!cache->IsUndefined())
    {
        info.GetReturnValue().Set(cache);
        return;
    }
    if (Z_TYPE(PG(http_globals)[TRACK_VARS_SERVER]) != IS_ARRAY && !zend_is_auto_global_str(ZEND_STRL("_SERVER")))
    {
        return;
    }
    HashTable *_SERVER = Z_ARRVAL(PG(http_globals)[TRACK_VARS_SERVER]);

    v8::Isolate *isolate = info.GetIsolate();
    v8::Local<v8::Object> obj = v8::Object::New(isolate);

    zval *value;
    zend_string *key;
    ZEND_HASH_FOREACH_STR_KEY_VAL(_SERVER, key, value)
    {
        std::string tmp = convert_to_header_key(key->val, key->len);
        if (!tmp.empty())
        {
            obj->Set(NewV8String(isolate, tmp), NewV8ValueFromZval(isolate, value));
        }
    }
    ZEND_HASH_FOREACH_END();

    info.GetReturnValue().Set(obj);
    self->SetInternalField(kHeader, obj);
}
static void body_getter(v8::Local<v8::Name> name, const v8::PropertyCallbackInfo<v8::Value> &info)
{
    auto self = info.Holder();
    auto cache = self->GetInternalField(kBody);
    if (!cache->IsUndefined())
    {
        info.GetReturnValue().Set(cache);
        return;
    }
    info.GetReturnValue().Set(v8::ArrayBuffer::New(info.GetIsolate(), nullptr, 0, v8::ArrayBufferCreationMode::kInternalized));

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
    self->SetInternalField(kBody, arraybuffer);
}
static void server_getter(v8::Local<v8::Name> name, const v8::PropertyCallbackInfo<v8::Value> &info)
{
    auto self = info.Holder();
    auto cache = self->GetInternalField(kServer);
    if (!cache->IsUndefined())
    {
        info.GetReturnValue().Set(cache);
        return;
    }
    v8::Isolate *isolate = info.GetIsolate();
    v8::Local<v8::Object> server = v8::Object::New(isolate);
    server->Set(NewV8String(isolate, "language"), NewV8String(isolate, "php"));
    server->Set(NewV8String(isolate, "name"), NewV8String(isolate, "PHP"));
    server->Set(NewV8String(isolate, "version"), NewV8String(isolate, OPENRASP_PHP_VERSION));
#ifdef PHP_WIN32
    server->Set(NewV8String(isolate, "os"), NewV8String(isolate, "Windows"));
#else
    if (strstr(PHP_OS, "Darwin"))
    {
        server->Set(NewV8String(isolate, "os"), NewV8String(isolate, "Mac"));
    }
    else if (strstr(PHP_OS, "Linux"))
    {
        server->Set(NewV8String(isolate, "os"), NewV8String(isolate, "Linux"));
    }
    else
    {
        server->Set(NewV8String(isolate, "os"), NewV8String(isolate, PHP_OS));
    }
#endif
    info.GetReturnValue().Set(server);
    self->SetInternalField(kServer, server);
}

static void json_body_getter(v8::Local<v8::Name> name, const v8::PropertyCallbackInfo<v8::Value> &info)
{
    auto self = info.Holder();
    auto cache = self->GetInternalField(kJsonBody);
    if (!cache->IsUndefined())
    {
        info.GetReturnValue().Set(cache);
        return;
    }
    if (Z_TYPE(PG(http_globals)[TRACK_VARS_SERVER]) != IS_ARRAY && !zend_is_auto_global_str(ZEND_STRL("_SERVER")))
    {
        return;
    }
    HashTable *_SERVER = Z_ARRVAL(PG(http_globals)[TRACK_VARS_SERVER]);
    v8::Isolate *isolate = info.GetIsolate();
    v8::Local<v8::Object> obj = v8::Object::New(isolate);

    std::string complete_body = "{}";
    zval *origin_zv;
    if (((origin_zv = zend_hash_str_find(_SERVER, ZEND_STRL("HTTP_CONTENT_TYPE"))) != nullptr ||
         (origin_zv = zend_hash_str_find(_SERVER, ZEND_STRL("CONTENT_TYPE"))) != nullptr) &&
        Z_TYPE_P(origin_zv) == IS_STRING)
    {
        openrasp_error(LEVEL_DEBUG, RUNTIME_ERROR, _("Content-type of request (%s) is %s."),
                       OPENRASP_INJECT_G(request_id), Z_STRVAL_P(origin_zv));
        std::string content_type_vlaue = std::string(Z_STRVAL_P(origin_zv));
        OpenRASPContentType::ContentType k_type = OpenRASPContentType::classify_content_type(content_type_vlaue);
        if (OpenRASPContentType::ContentType::cApplicationJson == k_type)
        {
            zend_string *body = fetch_request_body(PHP_STREAM_COPY_ALL);
            complete_body = std::string(ZSTR_VAL(body));
            zend_string_release(body);
        }
    }
    openrasp_error(LEVEL_DEBUG, RUNTIME_ERROR, _("Complete body of request (%s) is %s."),
                   OPENRASP_INJECT_G(request_id), complete_body.c_str());
    v8::TryCatch trycatch(isolate);
    auto v8_body = NewV8String(isolate, complete_body);
    auto v8_json_obj = v8::JSON::Parse(isolate->GetCurrentContext(), v8_body);
    if (v8_json_obj.IsEmpty())
    {
        v8::Local<v8::Value> exception = trycatch.Exception();
        v8::String::Utf8Value exception_str(isolate, exception);
        openrasp_error(LEVEL_DEBUG, RUNTIME_ERROR, _("Fail to parse json body, cuz of %s."), *exception_str);
    }
    else
    {
        auto v8_json_obj_l = v8_json_obj.ToLocalChecked();
        if (v8_json_obj_l->IsObject())
        {
            obj = v8_json_obj_l.As<v8::Object>();
        }
    }
    info.GetReturnValue().Set(obj);
    self->SetInternalField(kJsonBody, obj);
}

v8::Local<v8::ObjectTemplate> openrasp::CreateRequestContextTemplate(Isolate *isolate)
{
    auto obj_templ = v8::ObjectTemplate::New(isolate);
    obj_templ->SetAccessor(NewV8String(isolate, "url"), url_getter);
    obj_templ->SetAccessor(NewV8String(isolate, "header"), header_getter);
    obj_templ->SetAccessor(NewV8String(isolate, "parameter"), parameter_getter);
    obj_templ->SetAccessor(NewV8String(isolate, "path"), path_getter);
    obj_templ->SetAccessor(NewV8String(isolate, "querystring"), querystring_getter);
    obj_templ->SetAccessor(NewV8String(isolate, "method"), method_getter);
    obj_templ->SetAccessor(NewV8String(isolate, "protocol"), protocol_getter);
    obj_templ->SetAccessor(NewV8String(isolate, "remoteAddr"), remoteAddr_getter);
    obj_templ->SetAccessor(NewV8String(isolate, "appBasePath"), appBasePath_getter);
    obj_templ->SetAccessor(NewV8String(isolate, "body"), body_getter);
    obj_templ->SetAccessor(NewV8String(isolate, "server"), server_getter);
    obj_templ->SetAccessor(NewV8String(isolate, "json"), json_body_getter);
    obj_templ->SetInternalFieldCount(kEndForCount);
    return obj_templ;
}