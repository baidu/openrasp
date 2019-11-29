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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

extern "C"
{
#include "php.h"
}
#include "openrasp_ini.h"
#include "openrasp_utils.h"
#include "agent/shared_config_manager.h"
#ifdef HAVE_OPENRASP_REMOTE_MANAGER
#include "agent/openrasp_agent_manager.h"
#endif
#include <execinfo.h>
#include <signal.h>
#include <iostream>
#include <fstream>

typedef void (*sa_handler_t)(int);
typedef void (*sa_sigaction_t)(int, siginfo_t *, void *);
struct sigaction old_acts[SIGUSR2];
bool is_set_handler = false;
std::mutex mtx;

bool fork_and_exec(const char *cmd)
{
    const char *argv[4] = {"sh", "-c", cmd, nullptr};

    pid_t pid = fork();

    if (pid < 0)
    {
        // fork failed
        return false;
    }
    else if (pid == 0)
    {
        // child process

        execve("/bin/sh", (char *const *)argv, environ);

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
                return true;
            case EINTR:
                break;
            default:
                return false;
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

void report_crash_log(int sig)
{
    std::string log_path(std::string(openrasp_ini.root_dir) + DEFAULT_SLASH + std::string("crash_") + std::to_string(getpid()) + ".log");
    std::ofstream log(log_path);
    log << "Worker " << getpid() << " received signal: " << sig << std::endl;
    log << "PHP version: " << get_phpversion()
#ifdef ZTS
           << " (ZTS)"
#endif
           << std::endl;
    log << "OpenRASP version: " << OpenRASPInfo::PHP_OPENRASP_VERSION << std::endl;
    log << "V8 version: " << ZEND_TOSTR(V8_MAJOR_VERSION) "." ZEND_TOSTR(V8_MINOR_VERSION) << std::endl;
#ifdef OPENRASP_BUILD_TIME
    log << "Build time: " << OPENRASP_BUILD_TIME << std::endl;
#endif
#ifdef OPENRASP_COMMIT_ID
    log << "Commit ID: " << OPENRASP_COMMIT_ID << std::endl;
#endif
#ifdef HAVE_OPENRASP_REMOTE_MANAGER
    if (remote_active && openrasp::oam)
    {
        const char *plugin_version = openrasp::oam->get_plugin_version();
        log << "Plugin version: " << std::string(plugin_version ? plugin_version : "") << std::endl;
    }
#endif
    log << "RASP ID: " << openrasp::scm->get_rasp_id() << std::endl;
    log << "APP ID: " << std::string(openrasp_ini.app_id ? openrasp_ini.app_id : "") << std::endl;
    log << std::endl;
    {
        void *buf[100] = {0};
        int size = backtrace(buf, 100);
        log << "Native stacks: " << std::endl;
        char **strings = backtrace_symbols(buf, size);
        if (strings == nullptr)
        {
            log << "cannot get stacks" << std::endl;
        }
        else
        {
            for (int i = 0; i < size; i++)
            {
                log << strings[i] << std::endl;
            }
            log << std::endl;
            free(strings);
        }
    }
    {
        log << "PHP stacks: " << std::endl;
        auto arr = format_debug_backtrace_arr();
        for (auto &i : arr)
        {
            log << i << std::endl;
        }
        log << std::endl;
    }
    char cmd[4 * 1024];
    snprintf(cmd, 4 * 1024, "cd %s && sh report.sh %s", openrasp_ini.root_dir, log_path.c_str());
    if (!fork_and_exec(cmd))
    {
        std::cout << "failed to report crash log" << std::endl;
    }
}

void call_old_signal_handler(struct sigaction *old_act, int sig, siginfo_t *info, void *ctx)
{
    if (old_act->sa_handler != SIG_IGN && old_act->sa_handler != SIG_DFL)
    {
        if ((old_act->sa_flags & SA_NODEFER) == 0)
        {
            // automaticlly block the signal
            sigaddset(&(old_act->sa_mask), sig);
        }

        sa_handler_t hand;
        sa_sigaction_t sa;
        bool siginfo_flag_set = (old_act->sa_flags & SA_SIGINFO) != 0;
        // retrieve the chained handler
        if (siginfo_flag_set)
        {
            sa = old_act->sa_sigaction;
        }
        else
        {
            hand = old_act->sa_handler;
        }

        if ((old_act->sa_flags & SA_RESETHAND) != 0)
        {
            old_act->sa_handler = SIG_DFL;
        }

        // try to honor the signal mask
        sigset_t oset;
        pthread_sigmask(SIG_SETMASK, &(old_act->sa_mask), &oset);

        // call into the chained handler
        if (siginfo_flag_set)
        {
            (*sa)(sig, info, ctx);
        }
        else
        {
            (*hand)(sig);
        }

        // restore the signal mask
        pthread_sigmask(SIG_SETMASK, &oset, 0);
    }
}

void signal_handler(int sig, siginfo_t *info, void *ctx)
{
    report_crash_log(sig);
    call_old_signal_handler(&old_acts[sig], sig, info, ctx);
}

int set_signal_handler(int sig, sa_sigaction_t handler)
{
    struct sigaction act;
    sigfillset(&(act.sa_mask));
    act.sa_handler = SIG_DFL;
    act.sa_sigaction = handler;
    act.sa_flags = SA_SIGINFO;
    return sigaction(sig, &act, &old_acts[sig]);
}

PHP_RINIT_FUNCTION(openrasp_signal)
{
    if (!is_set_handler)
    {
        std::lock_guard<std::mutex> lock(mtx);
        if (!is_set_handler)
        {
            is_set_handler = true;
            set_signal_handler(SIGSEGV, signal_handler);
            set_signal_handler(SIGPIPE, signal_handler);
            set_signal_handler(SIGBUS, signal_handler);
            set_signal_handler(SIGILL, signal_handler);
            set_signal_handler(SIGFPE, signal_handler);
            set_signal_handler(SIGXFSZ, signal_handler);
        }
    }
    return SUCCESS;
}
