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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

extern "C"
{
#include "php.h"
#include "php_main.h"
}
#include "utils/hostname.h"
#include "openrasp.h"
#include "php/header.h"
#include "signal_interceptor.h"
#include "openrasp_utils.h"
#include "openrasp_ini.h"
#ifdef HAVE_OPENRASP_REMOTE_MANAGER
#include "agent/crash_reporter.h"
#include "agent/openrasp_agent_manager.h"
#endif
#include <execinfo.h>
#include <signal.h>
#include <unistd.h>
#include <iostream>
#include <fstream>

namespace openrasp
{
extern "C"
{
#ifdef HAVE_LINE_COVERAGE
    void __gcov_flush();
#endif
}

typedef void (*sa_handler_t)(int);
typedef void (*sa_sigaction_t)(int, siginfo_t *, void *);
struct sigaction old_acts[SIGUSR2];
bool is_set_handler = false;
std::mutex mtx;

void report_crash_log(int sig)
{
    std::string log_path(std::string(openrasp_ini.root_dir) + DEFAULT_SLASH + "logs" + DEFAULT_SLASH + "crash" + DEFAULT_SLASH +
                         std::string("crash-") + (sapi_module.name ? std::string(sapi_module.name) : "unknown-sapi") + "-" +
                         std::to_string(getpid()) + ".log");
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
            free(strings);
        }
        log << std::endl;
    }
#ifdef HAVE_OPENRASP_REMOTE_MANAGER
    if (openrasp::oam)
    {
        CrashReporter cr(log_path);
    }
#endif
#ifdef HAVE_LINE_COVERAGE
    __gcov_flush();
#endif
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
    else if (old_act->sa_handler == SIG_DFL)
    {
        bool siginfo_flag_set = (old_act->sa_flags & SA_SIGINFO) != 0;
        if ((old_act->sa_flags & SA_SIGINFO) == 0)
        {
            sa_handler_t hand = old_act->sa_handler;
            (*hand)(sig);
        }
    }
#ifdef HAVE_LINE_COVERAGE
    __gcov_flush();
#endif
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
    act.sa_flags = SA_SIGINFO | SA_RESETHAND;
    return sigaction(sig, &act, &old_acts[sig]);
}

void general_signal_hook()
{
    if (!is_set_handler)
    {
        std::lock_guard<std::mutex> lock(mtx);
        if (!is_set_handler)
        {
            is_set_handler = true;
            set_signal_handler(SIGSEGV, signal_handler);
            set_signal_handler(SIGABRT, signal_handler);
            set_signal_handler(SIGBUS, signal_handler);
            set_signal_handler(SIGILL, signal_handler);
            set_signal_handler(SIGFPE, signal_handler);
        }
    }
}

} // namespace openrasp
