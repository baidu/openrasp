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
#undef COMPILER // conflict with v8 defination
#include <v8.h>
#include <v8-platform.h>
#include <mutex>
#include <queue>
#include <vector>
#include <chrono>
#include <string>
#include <memory>
#include <iostream>
#include <condition_variable>
#include <thread>

namespace openrasp
{
#define LOG(msg) \
  std::cerr << msg << std::endl;

#define TRYCATCH() \
  v8::TryCatch try_catch

#define V8STRING_EX(string, type, length) \
  (v8::String::NewFromUtf8(isolate, string, type, length))

#define V8STRING_N(string) \
  V8STRING_EX(string, v8::NewStringType::kNormal, -1)

#define V8STRING_I(string) \
  V8STRING_EX(string, v8::NewStringType::kInternalized, -1)

class TaskQueue;
class WorkerThread;
class V8Platform : public v8::Platform
{
public:
  V8Platform();
  V8Platform(int thread_pool_size);
  ~V8Platform();
  virtual size_t NumberOfAvailableBackgroundThreads();
  virtual void CallOnBackgroundThread(v8::Task *task, ExpectedRuntime expected_runtime);
  virtual void CallOnBackgroundThread(v8::Task *task);
  virtual void CallOnForegroundThread(v8::Isolate *isolate, v8::Task *task);
  virtual void CallDelayedOnForegroundThread(v8::Isolate *isolate, v8::Task *task, double delay_in_seconds);
  virtual double MonotonicallyIncreasingTime();
  void EnsureBackgroundThreadInitialized();

private:
  std::mutex lock_;
  bool initialized_ = false;
  int thread_pool_size_ = 0;
  std::vector<WorkerThread *> thread_pool_;
  TaskQueue *queue_ = nullptr;
};

class TimeoutTask : public v8::Task
{
public:
  v8::Isolate *isolate;
  std::chrono::time_point<std::chrono::high_resolution_clock> time_point;
  std::timed_mutex mtx;
  TimeoutTask(v8::Isolate *_isolate, int _milliseconds = 100);
  ~TimeoutTask();
  void Run() override;
  std::timed_mutex &GetMtx();
};

class openrasp_v8_plugin_src
{
public:
  std::string filename;
  std::string source;
};

class openrasp_v8_process_globals
{
public:
  v8::StartupData snapshot_blob;
  std::mutex lock;
  bool is_initialized = false;
  V8Platform *v8_platform = nullptr;
  std::vector<openrasp_v8_plugin_src> plugin_src_list;
};

extern openrasp_v8_process_globals process_globals;

namespace RequestContext
{
v8::Local<v8::Object> New(v8::Isolate *isolate);
}

void v8error_to_stream(v8::Isolate *isolate, v8::TryCatch &try_catch, std::ostream &buf);
v8::Local<v8::Value> zval_to_v8val(zval *val, v8::Isolate *isolate TSRMLS_DC);
v8::MaybeLocal<v8::Script> compile_script(std::string _source, std::string _filename, int _line_offset = 0);
v8::MaybeLocal<v8::Value> exec_script(v8::Isolate *isolate, v8::Local<v8::Context> context,
                                      std::string _source, std::string _filename, int _line_offset = 0);
} // namespace openrasp

ZEND_BEGIN_MODULE_GLOBALS(openrasp_v8)
_zend_openrasp_v8_globals()
{
  isolate = nullptr;
  action_hash_ignore = 0;
  action_hash_log = 0;
  action_hash_block = 0;
  is_isolate_initialized = false;
  is_env_initialized = false;
  is_running = false;
}
v8::Isolate *isolate;
v8::Isolate::CreateParams create_params;
v8::Persistent<v8::Context> context;
v8::Persistent<v8::Object> RASP;
v8::Persistent<v8::Function> check;
v8::Persistent<v8::Object> request_context;
v8::Persistent<v8::String> key_action;
v8::Persistent<v8::String> key_message;
v8::Persistent<v8::String> key_name;
v8::Persistent<v8::String> key_confidence;
int action_hash_ignore;
int action_hash_log;
int action_hash_block;
bool is_isolate_initialized;
bool is_env_initialized;
bool is_running;
ZEND_END_MODULE_GLOBALS(openrasp_v8)

ZEND_EXTERN_MODULE_GLOBALS(openrasp_v8)

PHP_GINIT_FUNCTION(openrasp_v8);
PHP_GSHUTDOWN_FUNCTION(openrasp_v8);
PHP_MINIT_FUNCTION(openrasp_v8);
PHP_MSHUTDOWN_FUNCTION(openrasp_v8);

#ifdef ZTS
#define OPENRASP_V8_G(v) TSRMG(openrasp_v8_globals_id, zend_openrasp_v8_globals *, v)
#define OPENRASP_V8_GP() ((zend_openrasp_v8_globals *)(*((void ***)tsrm_ls))[TSRM_UNSHUFFLE_RSRC_ID(openrasp_v8_globals_id)])
#else
#define OPENRASP_V8_G(v) (openrasp_v8_globals.v)
#define OPENRASP_V8_GP() (&openrasp_v8_globals)
#endif

#ifdef HAVE_NATIVE_ANTLR4
#include <antlr4-runtime/antlr4-runtime.h>
#include "antlr/SQLLexer.h"
class TokenizeErrorListener : public antlr4::BaseErrorListener
{
public:
  virtual void syntaxError(antlr4::Recognizer *recognizer, antlr4::Token *offendingSymbol, size_t line, size_t charPositionInLine,
                           const std::string &msg, std::exception_ptr e)
  {
    std::string err_msg = "RASP.sql_tokenize error: line " + std::to_string(line) + ":" + std::to_string(charPositionInLine) + " " + msg;
    plugin_info(err_msg.c_str(), err_msg.length() TSRMLS_CC);
  }
};
#endif

#endif /* OPENRASP_v8_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
