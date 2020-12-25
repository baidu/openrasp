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

#include "utils/hostname.h"
#include "utils/file.h"
#include "crash_reporter.h"
#include "openrasp_log.h"
#include "openrasp_ini.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "agent/shared_config_manager.h"

namespace openrasp
{
CrashReporter::CrashReporter(const std::string &crash_file_path)
    : crash_file_path(crash_file_path)
{
    std::string crash_contents;
    if (read_entire_content(crash_file_path, crash_contents))
    {
        openrasp_error(LEVEL_WARNING, CRASH_ERROR, _("Crash occurred\nlog %s contents:\n%s"), crash_file_path.c_str(), crash_contents.c_str());
    }
}

int CrashReporter::fork_and_exec(const char *path, char *const argv[])
{
    pid_t pid = fork();

    if (pid < 0)
    {
        // fork failed
        return -1;
    }
    else if (pid == 0)
    {
        // child process
        execvp(path, argv);
        // execve failed
        _exit(-1);
    }
    else
    {
        // copied from J2SE ..._waitForProcessExit() in UNIXProcess_md.c; we don't
        // care about the actual exit code, for now.

        int status;

        // Wait for the child process to exit.  This returns immediately if
        // the child has already exited. */
        while (waitpid(pid, &status, 0) < 0)
        {
            switch (errno)
            {
            case ECHILD:
                return 0;
            case EINTR:
                break;
            default:
                return -1;
            }
        }

        if (WIFEXITED(status))
        {
            // The child exited normally; get its exit code.
            return WEXITSTATUS(status);
        }
        else if (WIFSIGNALED(status))
        {
            // The child exited because of a signal
            // The best value to return is 0x80 + signal number,
            // because that is what all Unix shells do, and because
            // it allows callers to distinguish between process exit and
            // process death by signal.
            return 0x80 + WTERMSIG(status);
        }
        else
        {
            // Unknown exit code; pass it through
            return status;
        }
    }
}

CrashReporter::~CrashReporter()
{
    std::string app_id_header = "X-OpenRASP-AppID: " + std::string(openrasp_ini.app_id);
    std::string app_secret_header = "X-OpenRASP-AppSecret: " + std::string(openrasp_ini.app_secret);
    std::string crash_reporting_url = std::string(openrasp_ini.backend_url) + "/v1/agent/crash/report";
    std::string rasp_id_form = "rasp_id=" + scm->get_rasp_id();
    std::string hostname_form = "hostname=" + get_hostname();
    std::string crash_log_form = "crash_log=@" + crash_file_path;
    char *const argv[] = {
        "curl",
        (char *)(crash_reporting_url.c_str()),
        "--connect-timeout",
        "5",
        "-H",
        (char *)(app_id_header.c_str()),
        "-H",
        (char *)(app_secret_header.c_str()),
        "-F",
        "language=php",
        "-F",
        (char *)(rasp_id_form.c_str()),
        "-F",
        (char *)(hostname_form.c_str()),
        "-F",
        (char *)(crash_log_form.c_str()),
        nullptr};

    if (fork_and_exec("curl", argv) != 0)
    {
        openrasp_error(LEVEL_WARNING, CRASH_ERROR, _("Fail to report crash file => %s"),
                       crash_file_path.c_str());
    }
}
} // namespace openrasp
