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

#ifndef _OPENRASP_AGENT_RUNNER_
#define _OPENRASP_AGENT_RUNNER_

namespace openrasp
{

class BaseAgentRunner
{
public:
  BaseAgentRunner(int autoreload_interval);
  virtual void run() = 0;

protected:
  int _autoreload_interval;
};

class PluginAgentRunner : public BaseAgentRunner
{
public:
  PluginAgentRunner();
  virtual void run();
};

class LogAgentRunner : public BaseAgentRunner
{
public:
  LogAgentRunner();
  virtual void run();
};

} // namespace openrasp
#endif