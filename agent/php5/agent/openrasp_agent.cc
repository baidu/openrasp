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

#include "openrasp_agent.h"

namespace openrasp
{

BaseAgent::BaseAgent(std::string name)
	: default_slash(1, DEFAULT_SLASH)
{
	this->name = name;
}

void BaseAgent::install_sigterm_handler(sighandler_t signal_handler)
{
	struct sigaction sa_usr = {0};
	sa_usr.sa_flags = 0;
	sa_usr.sa_handler = signal_handler;
	sigaction(SIGTERM, &sa_usr, NULL);
}

std::string BaseAgent::get_name() const
{
	return name;
}
} // namespace openrasp
