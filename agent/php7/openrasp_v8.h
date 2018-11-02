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

#ifndef OPENRASP_V8_H
#define OPENRASP_V8_H

#include "openrasp.h"
#include "openrasp_log.h"
#undef COMPILER // conflict with v8 defination
#include <v8.h>
#include <libplatform/libplatform.h>
#include <v8-platform.h>
#include <mutex>
#include <vector>
#include <chrono>
#include <string>

namespace openrasp
{
inline v8::Local<v8::String> NewV8String(v8::Isolate *isolate, const char *str, size_t len = -1)
{
  return v8::String::NewFromUtf8(isolate, str, v8::NewStringType::kNormal, len).ToLocalChecked();
}

inline v8::Local<v8::String> NewV8String(v8::Isolate *isolate, const std::string &str)
{
  return NewV8String(isolate, str.c_str(), str.length());
}

class Exception : public std::string
{
public:
  Exception(v8::Isolate *isolate, v8::TryCatch &try_catch);
};

class Platform
{
public:
  static v8::Platform *platform;
  static void Initialize();
  static void Shutdown();

private:
  static std::mutex mtx;
};

class TimeoutTask : public v8::Task
{
public:
  TimeoutTask(v8::Isolate *_isolate, int _milliseconds = 100);
  void Run() override;
  std::timed_mutex &GetMtx();

private:
  v8::Isolate *isolate;
  std::chrono::time_point<std::chrono::high_resolution_clock> time_point;
  std::timed_mutex mtx;
};

class PluginFile
{
public:
  PluginFile(const std::string &_filename, const std::string &_source) : filename(_filename), source(_source) {}
  std::string filename;
  std::string source;
};

class Snapshot : public v8::StartupData
{
public:
  uint64_t timestamp = 0;
  intptr_t *external_references;
  Snapshot(const char *data = nullptr, size_t raw_size = 0, uint64_t timestamp = 0);
  Snapshot(const v8::StartupData &blob) : Snapshot(blob.data, blob.raw_size){};
  Snapshot(const std::string &path, uint64_t timestamp = 0);
  Snapshot(const std::string &config, const std::vector<PluginFile> &plugin_list);
  ~Snapshot();
  bool Save(const std::string &path) const; // check errno when return value is false
  bool IsOk() const { return data && raw_size; };
  bool IsExpired(uint64_t timestamp) const { return timestamp > this->timestamp; };
};

class Isolate : public v8::Isolate
{
public:
  class Data
  {
  public:
    v8::Isolate::CreateParams create_params;
    v8::Persistent<v8::Object> RASP;
    v8::Persistent<v8::Function> check;
    v8::Persistent<v8::Object> request_context;
    v8::Persistent<v8::ObjectTemplate> request_context_templ;
    v8::Persistent<v8::String> key_action;
    v8::Persistent<v8::String> key_message;
    v8::Persistent<v8::String> key_name;
    v8::Persistent<v8::String> key_confidence;
    v8::Persistent<v8::String> key_algorithm;
    v8::Persistent<v8::Function> console_log;
    v8::Persistent<v8::Function> JSON_stringify;
    int action_hash_ignore = 0;
    int action_hash_log = 0;
    int action_hash_block = 0;
    uint64_t timestamp = 0;
  };

  static Isolate *New(Snapshot *snapshot_blob);
  Data *GetData();
  void SetData() = delete;
  void Dispose();
  bool IsExpired(uint64_t timestamp);
  static bool Check(Isolate *isolate, v8::Local<v8::String> type, v8::Local<v8::Object> params, int timeout = 100);
  bool Check(v8::Local<v8::String> type, v8::Local<v8::Object> params, int timeout = 100);
  static v8::MaybeLocal<v8::Value> ExecScript(Isolate *isolate, std::string source, std::string filename, int line_offset = 0);
  v8::MaybeLocal<v8::Value> ExecScript(std::string source, std::string filename, int line_offset = 0);
};

class openrasp_v8_process_globals
{
public:
  Snapshot *snapshot_blob = nullptr;
  std::mutex mtx;
  std::string plugin_config;
  std::vector<PluginFile> plugin_src_list;
};

extern openrasp_v8_process_globals process_globals;

v8::Local<v8::Value> NewV8ValueFromZval(v8::Isolate *isolate, zval *val);
v8::Local<v8::ObjectTemplate> NewRequestContextTemplate(v8::Isolate *isolate);
void log_callback(const v8::FunctionCallbackInfo<v8::Value> &info);
void plugin_info(const std::string &message);
void plugin_info(Isolate *isolate, v8::Local<v8::Value> value);
void alarm_info(Isolate *isolate, v8::Local<v8::String> type, v8::Local<v8::Object> params, v8::Local<v8::Object> result);
void load_plugins();
} // namespace openrasp

ZEND_BEGIN_MODULE_GLOBALS(openrasp_v8)
openrasp::Isolate *isolate = nullptr;
ZEND_END_MODULE_GLOBALS(openrasp_v8)

ZEND_EXTERN_MODULE_GLOBALS(openrasp_v8)

PHP_GINIT_FUNCTION(openrasp_v8);
PHP_GSHUTDOWN_FUNCTION(openrasp_v8);
PHP_MINIT_FUNCTION(openrasp_v8);
PHP_MSHUTDOWN_FUNCTION(openrasp_v8);
PHP_RINIT_FUNCTION(openrasp_v8);

#define OPENRASP_V8_G(v) ZEND_MODULE_GLOBALS_ACCESSOR(openrasp_v8, v)

#endif /* OPENRASP_v8_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
