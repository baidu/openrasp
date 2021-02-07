/*
 * Copyright 2017-2021 Baidu Inc.
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

#ifndef _OPENRASP_WEBDIR_AGENT_H_
#define _OPENRASP_WEBDIR_AGENT_H_

#include "agent/openrasp_agent.h"
#include "webdir_detector.h"

namespace openrasp
{

class WebDirAgent : public BaseAgent
{
  public:
    static volatile int signal_received;

  public:
    WebDirAgent();
    virtual void run();
    virtual void write_pid_to_shm(pid_t agent_pid);
    virtual pid_t get_pid_from_shm();

  private:
    WebDirDetector webdir_detector;
    bool collect_webroot_path();
    void sensitive_file_scan();
    void dependency_check();
};

} // namespace openrasp

#endif
