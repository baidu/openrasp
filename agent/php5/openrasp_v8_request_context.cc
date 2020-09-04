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

#include "openrasp_v8.h"
#include "openrasp_log.h"
#include "openrasp_utils.h"
#include "openrasp_content_type.h"
#include "openrasp_inject.h"
#include "agent/shared_config_manager.h"
#include "utils/hostname.h"

using namespace openrasp;

static void url_getter(v8::Local<v8::Name> name, const v8::PropertyCallbackInfo<v8::Value> &info)
{
    info.GetReturnValue().SetEmptyString();
    TSRMLS_FETCH();
    auto obj = NewV8String(info.GetIsolate(), OPENRASP_G(request).url.get_complete_url());
    info.GetReturnValue().Set(obj);
}
static void method_getter(v8::Local<v8::Name> name, const v8::PropertyCallbackInfo<v8::Value> &info)
{
    info.GetReturnValue().SetEmptyString();
    TSRMLS_FETCH();
    auto obj = NewV8String(info.GetIsolate(), OPENRASP_G(request).get_method());
    info.GetReturnValue().Set(obj);
}
static void querystring_getter(v8::Local<v8::Name> name, const v8::PropertyCallbackInfo<v8::Value> &info)
{
    info.GetReturnValue().SetEmptyString();
    TSRMLS_FETCH();
    auto obj = NewV8String(info.GetIsolate(), OPENRASP_G(request).url.get_query_string());
    info.GetReturnValue().Set(obj);
}
static void appBasePath_getter(v8::Local<v8::Name> name, const v8::PropertyCallbackInfo<v8::Value> &info)
{
    info.GetReturnValue().SetEmptyString();
    TSRMLS_FETCH();
    auto obj = NewV8String(info.GetIsolate(), OPENRASP_G(request).get_document_root());
    info.GetReturnValue().Set(obj);
}
static void protocol_getter(v8::Local<v8::Name> name, const v8::PropertyCallbackInfo<v8::Value> &info)
{
    info.GetReturnValue().SetEmptyString();
    TSRMLS_FETCH();
    auto obj = NewV8String(info.GetIsolate(), OPENRASP_G(request).url.get_request_scheme());
    info.GetReturnValue().Set(obj);
}
static void remoteAddr_getter(v8::Local<v8::Name> name, const v8::PropertyCallbackInfo<v8::Value> &info)
{
    info.GetReturnValue().SetEmptyString();
    TSRMLS_FETCH();
    auto obj = NewV8String(info.GetIsolate(), OPENRASP_G(request).get_remote_addr());
    info.GetReturnValue().Set(obj);
}
static void path_getter(v8::Local<v8::Name> name, const v8::PropertyCallbackInfo<v8::Value> &info)
{
    info.GetReturnValue().SetEmptyString();
    TSRMLS_FETCH();
    auto obj = NewV8String(info.GetIsolate(), OPENRASP_G(request).url.get_path());
    info.GetReturnValue().Set(obj);
}
static void parameter_getter(v8::Local<v8::Name> name, const v8::PropertyCallbackInfo<v8::Value> &info)
{
    TSRMLS_FETCH();
    if ((!PG(http_globals)[TRACK_VARS_GET] && !zend_is_auto_global(ZEND_STRL("_GET") TSRMLS_CC)) ||
        (!PG(http_globals)[TRACK_VARS_POST] && !zend_is_auto_global(ZEND_STRL("_POST") TSRMLS_CC)))
    {
        return;
    }
    v8::Isolate *isolate = info.GetIsolate();
    auto context = isolate->GetCurrentContext();
    v8::Local<v8::Object> obj = v8::Object::New(isolate);
    HashTable *GET = Z_ARRVAL_P(PG(http_globals)[TRACK_VARS_GET]);
    for (zend_hash_internal_pointer_reset(GET); zend_hash_has_more_elements(GET) == SUCCESS; zend_hash_move_forward(GET))
    {
        char *key = nullptr;
        ulong idx;
        int type = 0;
        zval **value;
        type = zend_hash_get_current_key(GET, &key, &idx, 0);
        if (type == HASH_KEY_NON_EXISTENT ||
            zend_hash_get_current_data(GET, (void **)&value) != SUCCESS)
        {
            continue;
        }
        v8::Local<v8::Value> v8_value = NewV8ValueFromZval(isolate, *value);
        if (!v8_value->IsArray())
        {
            v8::Local<v8::Array> v8_arr = v8::Array::New(isolate);
            v8_arr->Set(context, 0, v8_value).IsJust();
            v8_value = v8_arr;
        }
        v8::Local<v8::Value> v8_key;
        if (type == HASH_KEY_IS_STRING)
        {
            v8_key = NewV8String(isolate, key);
        }
        else
        {
            v8_key = v8::Uint32::New(isolate, idx);
        }
        obj->Set(context, v8_key, v8_value).IsJust();
    }
    HashTable *POST = Z_ARRVAL_P(PG(http_globals)[TRACK_VARS_POST]);
    for (zend_hash_internal_pointer_reset(POST); zend_hash_has_more_elements(POST) == SUCCESS; zend_hash_move_forward(POST))
    {
        char *key = nullptr;
        ulong idx;
        int type = 0;
        zval **value;
        type = zend_hash_get_current_key(POST, &key, &idx, 0);
        if (type == HASH_KEY_NON_EXISTENT ||
            zend_hash_get_current_data(POST, (void **)&value) != SUCCESS)
        {
            continue;
        }
        v8::Local<v8::Value> v8_value = NewV8ValueFromZval(isolate, *value);
        if (!v8_value->IsArray())
        {
            v8::Local<v8::Array> v8_arr = v8::Array::New(isolate);
            v8_arr->Set(context, 0, v8_value).IsJust();
            v8_value = v8_arr;
        }
        v8::Local<v8::Value> v8_key;
        if (type == HASH_KEY_IS_STRING)
        {
            v8_key = NewV8String(isolate, key);
        }
        else
        {
            v8_key = v8::Uint32::New(isolate, idx);
        }
        v8::Local<v8::Value> v8_existed_value = obj->Get(context, v8_key).ToLocalChecked();
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
                v8_arr->Set(context, i, v8_arr1->Get(context, i).ToLocalChecked()).IsJust();
            }
            for (int i = 0; i < v8_arr2_len; i++)
            {
                v8_arr->Set(context, v8_arr1_len + i, v8_arr2->Get(context, i).ToLocalChecked()).IsJust();
            }
            v8_value = v8_arr;
        }
        obj->Set(context, v8_key, v8_value).IsJust();
    }
    info.GetReturnValue().Set(obj);
}
static void header_getter(v8::Local<v8::Name> name, const v8::PropertyCallbackInfo<v8::Value> &info)
{
    TSRMLS_FETCH();
    v8::Isolate *isolate = info.GetIsolate();
    auto context = isolate->GetCurrentContext();
    v8::Local<v8::Object> obj = v8::Object::New(isolate);
    std::map<std::string, std::string> headers = OPENRASP_G(request).get_header();
    for (auto iter = headers.begin(); iter != headers.end(); iter++)
    {
        obj->Set(context, NewV8String(isolate, iter->first), NewV8String(isolate, iter->second)).IsJust();
    }
    info.GetReturnValue().Set(obj);
}
static void body_getter(v8::Local<v8::Name> name, const v8::PropertyCallbackInfo<v8::Value> &info)
{
    info.GetReturnValue().Set(v8::ArrayBuffer::New(info.GetIsolate(), nullptr, 0, v8::ArrayBufferCreationMode::kInternalized));
    TSRMLS_FETCH();
    php_stream *stream = php_stream_open_wrapper("php://input", "rb", 0, nullptr);
    if (!stream)
    {
        return;
    }
    char *contents = nullptr;
    int len = php_stream_copy_to_mem(stream, &contents, 4 * 1024, 1);
    stream->is_persistent ? php_stream_pclose(stream) : php_stream_close(stream);
    if (len <= 0 || !contents)
    {
        return;
    }
    v8::Isolate *isolate = info.GetIsolate();
    v8::Local<v8::ArrayBuffer> arraybuffer = v8::ArrayBuffer::New(isolate, contents, MIN(len, 4 * 1024), v8::ArrayBufferCreationMode::kInternalized);
    info.GetReturnValue().Set(arraybuffer);
}
static void server_getter(v8::Local<v8::Name> name, const v8::PropertyCallbackInfo<v8::Value> &info)
{
    TSRMLS_FETCH();
    v8::Isolate *isolate = info.GetIsolate();
    auto context = isolate->GetCurrentContext();
    v8::Local<v8::Object> server = v8::Object::New(isolate);
    server->Set(context, NewV8String(isolate, "language"), NewV8String(isolate, "php")).IsJust();
    server->Set(context, NewV8String(isolate, "server"), NewV8String(isolate, "PHP")).IsJust();
    server->Set(context, NewV8String(isolate, "version"), NewV8String(isolate, get_phpversion())).IsJust();
#ifdef PHP_WIN32
    server->Set(context, NewV8String(isolate, "os"), NewV8String(isolate, "Windows")).IsJust();
#else
    if (strstr(PHP_OS, "Darwin"))
    {
        server->Set(context, NewV8String(isolate, "os"), NewV8String(isolate, "Mac")).IsJust();
    }
    else if (strstr(PHP_OS, "Linux"))
    {
        server->Set(context, NewV8String(isolate, "os"), NewV8String(isolate, "Linux")).IsJust();
    }
    else
    {
        server->Set(context, NewV8String(isolate, "os"), NewV8String(isolate, PHP_OS)).IsJust();
    }
#endif
    info.GetReturnValue().Set(server);
}

static void json_body_getter(v8::Local<v8::Name> name, const v8::PropertyCallbackInfo<v8::Value> &info)
{
    TSRMLS_FETCH();
    if (!PG(http_globals)[TRACK_VARS_SERVER] && !zend_is_auto_global(ZEND_STRL("_SERVER") TSRMLS_CC))
    {
        return;
    }
    v8::Isolate *isolate = info.GetIsolate();
    v8::Local<v8::Object> obj = v8::Object::New(isolate);
    HashTable *SERVER = Z_ARRVAL_P(PG(http_globals)[TRACK_VARS_SERVER]);
    std::string complete_body = "{}";
    zval **origin_zv;
    if ((zend_hash_find(SERVER, ZEND_STRS("HTTP_CONTENT_TYPE"), (void **)&origin_zv) == SUCCESS ||
         zend_hash_find(SERVER, ZEND_STRS("CONTENT_TYPE"), (void **)&origin_zv) == SUCCESS) &&
        Z_TYPE_PP(origin_zv) == IS_STRING)
    {
        openrasp_error(LEVEL_DEBUG, RUNTIME_ERROR, _("Content-type of request (%s) is %s."),
                OPENRASP_G(request).get_id().c_str(), Z_STRVAL_PP(origin_zv));
        std::string content_type_vlaue = std::string(Z_STRVAL_PP(origin_zv));
        OpenRASPContentType::ContentType k_type = OpenRASPContentType::classify_content_type(content_type_vlaue);
        char *body = nullptr;
        if (OpenRASPContentType::ContentType::cApplicationJson == k_type &&
            (body = fetch_request_body(PHP_STREAM_COPY_ALL TSRMLS_CC)) != nullptr)
        {
            if (strcmp(body, "") != 0)
            {
                complete_body = std::string(body);
            }
            efree(body);
        }
    }
    openrasp_error(LEVEL_DEBUG, RUNTIME_ERROR, _("Complete body of request (%s) is %s."),
                   OPENRASP_G(request).get_id().c_str(), complete_body.c_str());
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
}
static void requestId_getter(v8::Local<v8::Name> name, const v8::PropertyCallbackInfo<v8::Value> &info)
{
    TSRMLS_FETCH();
    auto obj = NewV8String(info.GetIsolate(), OPENRASP_G(request).get_id().c_str());
    info.GetReturnValue().Set(obj);
}
static void raspId_getter(v8::Local<v8::Name> name, const v8::PropertyCallbackInfo<v8::Value> &info)
{
    info.GetReturnValue().SetEmptyString();
    TSRMLS_FETCH();
    auto obj = NewV8String(info.GetIsolate(), openrasp::scm->get_rasp_id());
    info.GetReturnValue().Set(obj);
}
static void appId_getter(v8::Local<v8::Name> name, const v8::PropertyCallbackInfo<v8::Value> &info)
{
    info.GetReturnValue().SetEmptyString();
    TSRMLS_FETCH();
    auto obj = NewV8String(info.GetIsolate(), (openrasp_ini.app_id ? openrasp_ini.app_id : ""));
    info.GetReturnValue().Set(obj);
}
static void hostname_getter(v8::Local<v8::Name> name, const v8::PropertyCallbackInfo<v8::Value> &info)
{
    info.GetReturnValue().SetEmptyString();
    TSRMLS_FETCH();
    auto obj = NewV8String(info.GetIsolate(), openrasp::get_hostname());
    info.GetReturnValue().Set(obj);
}
static void nic_getter(v8::Local<v8::Name> name, const v8::PropertyCallbackInfo<v8::Value> &info)
{
    info.GetReturnValue().SetEmptyString();
    TSRMLS_FETCH();
    std::map<std::string, std::string> if_addr_map = get_if_addr_map();
    v8::Isolate *isolate = info.GetIsolate();
    auto context = isolate->GetCurrentContext();
    int num = if_addr_map.size();
    v8::Local<v8::Array> arr = v8::Array::New(isolate, num);
    int index = 0;
    for (auto iter = if_addr_map.begin(); iter != if_addr_map.end(); iter++)
    {
        v8::Local<v8::Object> pair_obj = v8::Object::New(isolate);
        pair_obj->Set(context, NewV8String(isolate, "name"), NewV8String(isolate, iter->first)).IsJust();
        pair_obj->Set(context, NewV8String(isolate, "ip"), NewV8String(isolate, iter->second)).IsJust();
        arr->Set(context, index++, pair_obj).IsJust();
    }
    info.GetReturnValue().Set(arr);
}
static void source_getter(v8::Local<v8::Name> name, const v8::PropertyCallbackInfo<v8::Value> &info)
{
    info.GetReturnValue().SetEmptyString();
    TSRMLS_FETCH();
    auto obj = NewV8String(info.GetIsolate(), OPENRASP_G(request).get_remote_addr());
    info.GetReturnValue().Set(obj);
}
static void target_getter(v8::Local<v8::Name> name, const v8::PropertyCallbackInfo<v8::Value> &info)
{
    info.GetReturnValue().SetEmptyString();
    TSRMLS_FETCH();
    auto obj = NewV8String(info.GetIsolate(), OPENRASP_G(request).url.get_server_addr());
    info.GetReturnValue().Set(obj);
}
static void clientIp_getter(v8::Local<v8::Name> name, const v8::PropertyCallbackInfo<v8::Value> &info)
{
    info.GetReturnValue().SetEmptyString();
    TSRMLS_FETCH();
    std::string clientip_header = OPENRASP_CONFIG(clientip.header);
    std::transform(clientip_header.begin(), clientip_header.end(), clientip_header.begin(), ::tolower);
    auto obj = NewV8String(info.GetIsolate(), OPENRASP_G(request).get_header(clientip_header));
    info.GetReturnValue().Set(obj);
}
v8::Local<v8::ObjectTemplate> openrasp::CreateRequestContextTemplate(Isolate *isolate)
{
    auto obj_templ = v8::ObjectTemplate::New(isolate);
    obj_templ->SetLazyDataProperty(NewV8String(isolate, "url"), url_getter);
    obj_templ->SetLazyDataProperty(NewV8String(isolate, "header"), header_getter);
    obj_templ->SetLazyDataProperty(NewV8String(isolate, "parameter"), parameter_getter);
    obj_templ->SetLazyDataProperty(NewV8String(isolate, "path"), path_getter);
    obj_templ->SetLazyDataProperty(NewV8String(isolate, "querystring"), querystring_getter);
    obj_templ->SetLazyDataProperty(NewV8String(isolate, "method"), method_getter);
    obj_templ->SetLazyDataProperty(NewV8String(isolate, "protocol"), protocol_getter);
    obj_templ->SetLazyDataProperty(NewV8String(isolate, "remoteAddr"), remoteAddr_getter);
    obj_templ->SetLazyDataProperty(NewV8String(isolate, "appBasePath"), appBasePath_getter);
    obj_templ->SetLazyDataProperty(NewV8String(isolate, "body"), body_getter);
    obj_templ->SetLazyDataProperty(NewV8String(isolate, "server"), server_getter);
    obj_templ->SetLazyDataProperty(NewV8String(isolate, "json"), json_body_getter);
    obj_templ->SetLazyDataProperty(NewV8String(isolate, "requestId"), requestId_getter);
    obj_templ->SetLazyDataProperty(NewV8String(isolate, "raspId"), raspId_getter);
    obj_templ->SetLazyDataProperty(NewV8String(isolate, "appId"), appId_getter);
    obj_templ->SetLazyDataProperty(NewV8String(isolate, "hostname"), hostname_getter);
    obj_templ->SetLazyDataProperty(NewV8String(isolate, "nic"), nic_getter);
    obj_templ->SetLazyDataProperty(NewV8String(isolate, "source"), source_getter);
    obj_templ->SetLazyDataProperty(NewV8String(isolate, "target"), target_getter);
    obj_templ->SetLazyDataProperty(NewV8String(isolate, "clientIp"), clientIp_getter);
    return obj_templ;
}