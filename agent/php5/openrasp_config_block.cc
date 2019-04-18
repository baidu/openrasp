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

#include "openrasp_config_block.h"
#include "utils/regex.h"
#include "openrasp_v8.h"

namespace openrasp
{

void g_zero_filter(int64_t &value, const int64_t &dafault)
{
  if (value <= 0)
  {
    value = dafault;
  }
}

void ge_zero_filter(int64_t &value, const int64_t &dafault)
{
  if (value < 0)
  {
    value = dafault;
  }
}

void regex_filter(string &value, const string &regex, const string &dafault)
{
  if (!regex_match(value.c_str(), regex.c_str()))
  {
    value = dafault;
  }
}

const int64_t PluginBlock::default_timeout_millis = 100;
const int64_t PluginBlock::default_maxstack = 100;

void PluginBlock::update(BaseReader *reader)
{
  timeout.millis = reader->fetch_int64({"plugin.timeout.millis"}, PluginBlock::default_timeout_millis);
  g_zero_filter(timeout.millis, PluginBlock::default_timeout_millis);

  maxstack = reader->fetch_int64({"plugin.maxstack"}, PluginBlock::default_maxstack);
  ge_zero_filter(maxstack, PluginBlock::default_maxstack);

  filter = reader->fetch_bool({"plugin.filter"}, true);
};

const int64_t LogBlock::default_maxburst = 100;
const int64_t LogBlock::default_maxstack = 50;

void LogBlock::update(BaseReader *reader)
{
  maxburst = reader->fetch_int64({"log.maxburst"}, LogBlock::default_maxburst);
  ge_zero_filter(maxburst, LogBlock::default_maxburst);

  maxstack = reader->fetch_int64({"log.maxstack"}, LogBlock::default_maxstack);
  ge_zero_filter(maxstack, LogBlock::default_maxstack);
};

const std::string SyslogBlock::default_tag = "OpenRASP";
const int64_t SyslogBlock::default_facility = 1;
const int64_t SyslogBlock::default_connection_timeout = 50;
const int64_t SyslogBlock::default_read_timeout = 10;
const int64_t SyslogBlock::default_reconnect_interval = 300;

void SyslogBlock::update(BaseReader *reader)
{
  tag = reader->fetch_string({"syslog.tag"}, SyslogBlock::default_tag);
  regex_filter(tag, "^[0-9a-zA-Z]{1,32}$", SyslogBlock::default_tag);

  url = reader->fetch_string({"syslog.url"}, std::string(""));
  enable = reader->fetch_bool({"syslog.enable"}, false);

  facility = reader->fetch_int64({"syslog.facility"}, SyslogBlock::default_facility);
  ge_zero_filter(facility, SyslogBlock::default_facility);

  connection_timeout = reader->fetch_int64({"syslog.connection_timeout"}, SyslogBlock::default_connection_timeout);
  g_zero_filter(connection_timeout, SyslogBlock::default_connection_timeout);

  read_timeout = reader->fetch_int64({"syslog.read_timeout"}, SyslogBlock::default_read_timeout);
  g_zero_filter(read_timeout, SyslogBlock::default_read_timeout);

  reconnect_interval = reader->fetch_int64({"syslog.reconnect_interval"}, SyslogBlock::default_reconnect_interval);
  g_zero_filter(reconnect_interval, SyslogBlock::default_reconnect_interval);
};

const int64_t BlockBlock::default_status_code = 302;

void BlockBlock::update(BaseReader *reader)
{
  status_code = reader->fetch_int64({"block.status_code"}, BlockBlock::default_status_code);
  g_zero_filter(status_code, BlockBlock::default_status_code);

  redirect_url = reader->fetch_string({"block.redirect_url"}, std::string(R"(https://rasp.baidu.com/blocked/?request_id=%request_id%)"));
  content_json = reader->fetch_string({"block.content_json"}, std::string(R"({"error":true, "reason": "Request blocked by OpenRASP", "request_id": "%request_id%"})"));
  content_xml = reader->fetch_string({"block.content_xml"}, std::string(R"(<?xml version="1.0"?><doc><error>true</error><reason>Request blocked by OpenRASP</reason><request_id>%request_id%</request_id></doc>)"));
  content_html = reader->fetch_string({"block.content_html"}, std::string(R"(</script><script>location.href="https://rasp.baidu.com/blocked2/?request_id=%request_id%"</script>)"));
};

void InjectBlock::update(BaseReader *reader)
{
  urlprefix = reader->fetch_string({"inject.urlprefix"});
  const auto custom_headers_keys = reader->fetch_object_keys({"inject.custom_headers"});
  headers.clear();
  for (const auto &key : custom_headers_keys)
  {
    const auto &value = reader->fetch_string({"inject.custom_headers", key});
    headers.emplace_back(key + ": " + value);
  }
};

const int64_t BodyBlock::default_maxbytes = 4 * 1024;

void BodyBlock::update(BaseReader *reader)
{
  maxbytes = reader->fetch_int64({"body.maxbytes"}, BodyBlock::default_maxbytes);
  ge_zero_filter(maxbytes, BodyBlock::default_maxbytes);
};

void ClientipBlock::update(BaseReader *reader)
{
  header = reader->fetch_string({"clientip.header"}, std::string(""));
};

void SecurityBlock::update(BaseReader *reader)
{
  enforce_policy = reader->fetch_bool({"security.enforce_policy"}, false);
};

const int64_t LruBlock::default_max_size = 1024;

void LruBlock::update(BaseReader *reader)
{
  max_size = reader->fetch_int64({"lru.max_size"}, LruBlock::default_max_size);
  ge_zero_filter(max_size, LruBlock::default_max_size);
};

void DecompileBlock::update(BaseReader *reader)
{
  enable = reader->fetch_bool({"decompile.enable"}, false);
};

const vector<string> CallableBlock::default_blacklist = {"system", "exec", "passthru", "proc_open", "shell_exec", "popen", "pcntl_exec", "assert"};

void CallableBlock::update()
{
  blacklist = CallableBlock::default_blacklist;
  TSRMLS_FETCH();
  if (nullptr != OPENRASP_V8_G(isolate))
  {
    extract_callable_blacklist(OPENRASP_V8_G(isolate));
  }
}

void CallableBlock::extract_callable_blacklist(Isolate *isolate)
{
  std::string blacklist_strings;
  blacklist_strings.append("[");
  for (const auto &item : CallableBlock::default_blacklist)
  {
    blacklist_strings.append("\"").append(item).append("\",");
  }
  blacklist_strings.append("]");
  std::string script;
  script.append(R"( (function () {
            var blacklist
            try {
                blacklist = RASP.algorithmConfig.webshell_callable.functions
            } catch (_) {

            }
            if (blacklist === undefined || !Array.isArray(blacklist)) {
                blacklist = )")
      .append(blacklist_strings)
      .append(R"(
            }
            return blacklist
        })())");
  v8::HandleScope handle_scope(isolate);
  auto context = isolate->GetCurrentContext();
  auto rst = isolate->ExecScript(script, "extract_callable_blacklist");
  if (!rst.IsEmpty())
  {
    blacklist.clear();
    auto arr = rst.ToLocalChecked().As<v8::Array>();
    auto len = arr->Length();
    for (size_t i = 0; i < len; i++)
    {
      v8::HandleScope handle_scope(isolate);
      v8::Local<v8::Value> item;
      if (!arr->Get(context, i).ToLocal(&item) || !item->IsString())
      {
        continue;
      }
      v8::String::Utf8Value value(isolate, item);
      blacklist.push_back(std::string(*value, value.length()));
    }
  }
}

const int64_t XssBlock::default_min_param_length = 15;
const int64_t XssBlock::default_max_detection_num = 10;
const std::string XssBlock::default_filter_regex = "<![\\\\-\\\\[A-Za-z]|<([A-Za-z]{1,12})[\\\\/ >]";
const std::string XssBlock::default_echo_filter_regex = "<![\\\\-\\\\[A-Za-z]|<([A-Za-z]{1,12})[\\\\/ >]";

void XssBlock::update()
{
  echo_filter_regex = XssBlock::default_echo_filter_regex;
  filter_regex = XssBlock::default_filter_regex;
  min_param_length = XssBlock::default_min_param_length;
  max_detection_num = XssBlock::default_max_detection_num;
  TSRMLS_FETCH();
  if (nullptr != OPENRASP_V8_G(isolate))
  {
    extract_userinput_config(OPENRASP_V8_G(isolate));
    extract_echo_config(OPENRASP_V8_G(isolate));
  }
}

void XssBlock::extract_userinput_config(Isolate *isolate)
{
  std::string script;
  script.append(R"((function () {
            var filter_regex = ")")
      .append(XssBlock::default_filter_regex)
      .append(R"("
            var min_length = )")
      .append(std::to_string(XssBlock::default_min_param_length))
      .append(R"(
            var max_detection_num = )")
      .append(std::to_string(XssBlock::default_max_detection_num))
      .append(R"(
            try {
                var xss_userinput = RASP.algorithmConfig.xss_userinput
                if (typeof xss_userinput.filter_regex === 'string') {
                    filter_regex = xss_userinput.filter_regex
                }
                if (Number.isInteger(xss_userinput.min_length)) {
                    min_length = xss_userinput.min_length
                }
                if (Number.isInteger(xss_userinput.max_detection_num)) {
                    max_detection_num = xss_userinput.max_detection_num
                }
            } catch (_) {

            }
            return [filter_regex, min_length, max_detection_num]
        })())");
  v8::HandleScope handle_scope(isolate);
  auto context = isolate->GetCurrentContext();
  auto rst = isolate->ExecScript(script, "extract_userinput_config");
  if (!rst.IsEmpty())
  {
    auto arr = rst.ToLocalChecked().As<v8::Array>();
    auto len = arr->Length();
    if (3 == len)
    {
      v8::HandleScope handle_scope(isolate);
      v8::Local<v8::Value> item0;
      if (arr->Get(context, 0).ToLocal(&item0) && item0->IsString())
      {
        v8::String::Utf8Value value(isolate, item0);
        filter_regex = std::string(*value, value.length());
      }
      v8::Local<v8::Value> item1;
      if (arr->Get(context, 1).ToLocal(&item1) && item1->IsNumber())
      {
        min_param_length = item1->IntegerValue(context).FromJust();
      }
      v8::Local<v8::Value> item2;
      if (arr->Get(context, 2).ToLocal(&item2) && item2->IsNumber())
      {
        max_detection_num = item2->IntegerValue(context).FromJust();
      }
    }
  }
}

void XssBlock::extract_echo_config(Isolate *isolate)
{
  std::string script;
  script.append(R"((function () {
            var filter_regex = ")")
      .append(XssBlock::default_echo_filter_regex)
      .append(R"("
            try {
                var xss_echo = RASP.algorithmConfig.xss_echo
                if (typeof xss_echo.filter_regex === 'string') {
                    filter_regex = xss_echo.filter_regex
                }
            } catch (_) {

            }
            return filter_regex
        })())");
  v8::HandleScope handle_scope(isolate);
  auto context = isolate->GetCurrentContext();
  auto rst = isolate->ExecScript(script, "extract_echo_config");
  if (!rst.IsEmpty())
  {
    v8::HandleScope handle_scope(isolate);
    v8::Local<v8::String> v8_filter_regex = rst.ToLocalChecked().As<v8::String>();
    v8::String::Utf8Value value(isolate, v8_filter_regex);
    echo_filter_regex = std::string(*value, value.length());
  }
}

} // namespace openrasp