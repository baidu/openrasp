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

#include "openrasp_log_collector.h"
#include "openrasp_ini.h"
#include <sys/signalfd.h>
#include <signal.h>
extern "C"
{
#include "ext/standard/file.h"
#include "ext/date/php_date.h"
#include "ext/standard/php_smart_str.h"
#include "ext/json/php_json.h"
}

namespace openrasp
{
#define MAX_EVENTS 256
#define EVENT_SIZE (sizeof(struct inotify_event))
#define BUF_LEN (EVENT_SIZE + MAXPATHLEN)

#define MODIFY_TAG 'M'
#define CREATE_TAG 'C'
#define DELETE_TAG 'D'
#define REREAD_TAG 'R'

LogCollector::LogCollector()
    : _root_dir(openrasp_ini.root_dir),
      _default_slash(1, DEFAULT_SLASH),
      _backend(openrasp_ini.backend)
{
    TSRMLS_FETCH();
    update_formatted_date_suffix();
    log_dirs.push_back(std::move(LogDirEntry(_root_dir + _default_slash + "logs" + _default_slash + ALARM_LOG_DIR_NAME, "alarm.log.", "/v1/log/attack")));
    log_dirs.push_back(std::move(LogDirEntry(_root_dir + _default_slash + "logs" + _default_slash + POLICY_LOG_DIR_NAME, "policy.log.", "/v1/log/policy")));
}

void LogCollector::update_formatted_date_suffix()
{
    TSRMLS_FETCH();
    char *tmp_formatted_date_suffix = php_format_date(ZEND_STRL(DEFAULT_LOG_FILE_SUFFIX), (long)time(NULL), 1 TSRMLS_CC);
    _formatted_date_suffix = std::string(tmp_formatted_date_suffix);
    efree(tmp_formatted_date_suffix);
}

void LogCollector::run()
{
    TSRMLS_FETCH();
    int sfd;
    ssize_t s;
    sigset_t mask;
    char buffer[BUF_LEN];
    struct signalfd_siginfo fdsi;
    std::vector<std::pair<int, std::string>> created_files;

    sigemptyset(&mask);
    sigaddset(&mask, SIGTERM);

    if (sigprocmask(SIG_BLOCK, &mask, NULL) == -1)
        openrasp_error(E_ERROR, AGENT_ERROR, _("Fail to sigprocmask."));

    sfd = signalfd(-1, &mask, 0);
    if (sfd == -1)
    {
        openrasp_error(E_ERROR, AGENT_ERROR, _("Fail to signalfd."));
    }
    struct epoll_event events[MAX_EVENTS];
    bool running = true;
    if ((epoll_instance = epoll_create(2)) == -1)
    {
        openrasp_error(E_ERROR, AGENT_ERROR, _("Fail to epoll_crate."));
    }
    inotify_instance = inotify_init1(IN_NONBLOCK | IN_CLOEXEC);

    _ev.events = EPOLLIN;
    _ev.data.fd = sfd;
    if (epoll_ctl(epoll_instance, EPOLL_CTL_ADD, sfd, &_ev) == -1)
    {
        openrasp_error(E_ERROR, AGENT_ERROR, _("Fail to epoll_ctl: signal_fd."));
    }

    _ev.events = EPOLLIN;
    _ev.data.fd = inotify_instance;
    if (epoll_ctl(epoll_instance, EPOLL_CTL_ADD, inotify_instance, &_ev) == -1)
    {
        openrasp_error(E_ERROR, AGENT_ERROR, _("Fail to epoll_ctl: inotify."));
    }
    for (int i = 0; i < log_dirs.size(); ++i)
    {
        const char *dir_abs_name = log_dirs[i].dir_abs_path.c_str();
        if (VCWD_STAT(dir_abs_name, &sb) == 0)
        {
            if (S_ISDIR(sb.st_mode) && VCWD_ACCESS(dir_abs_name, R_OK | F_OK) == 0)
            {
                int inotify_watch = inotify_add_watch(inotify_instance, dir_abs_name, IN_CREATE | IN_DELETE);
                log_dir_umap.insert(std::make_pair(inotify_watch, &(log_dirs[i])));
                add_watch_file(log_dirs[i].prefix + _formatted_date_suffix, false, inotify_watch);
            }
        }
    }
    for (auto iter = log_dir_umap.begin(); iter != log_dir_umap.end(); iter++)
    {
        iter->second->active_thread = std::thread{&LogCollector::process_log_push, this, iter->first};
    }
    while (running)
    {
        int i;
        int num_events;
        if ((num_events = epoll_wait(epoll_instance, events, MAX_EVENTS, 1000)) < 0)
        {
            openrasp_error(E_ERROR, AGENT_ERROR, _("Fail to epoll_wait %d."), errno);
        }
        for (i = 0; i < num_events; ++i)
        {
            if (events[i].data.fd == inotify_instance)
            {
                int len;
                char *ptr;
                const struct inotify_event *event;
                while (1)
                {
                    len = read(inotify_instance, buffer, BUF_LEN);
                    if (len == -1 && errno != EAGAIN)
                    {
                        openrasp_error(E_ERROR, AGENT_ERROR, _("Fail to read."));
                    }
                    if (len <= 0)
                    {
                        break;
                    }
                    for (ptr = buffer; ptr < buffer + len;
                         ptr += sizeof(struct inotify_event) + event->len)
                    {
                        event = (const struct inotify_event *)ptr;
                        auto dir_got = log_dir_umap.find(event->wd);
                        if (dir_got != log_dir_umap.end())
                        {
                            if (event->mask & IN_CREATE && !(event->mask & IN_ISDIR))
                            {
                                update_formatted_date_suffix();
                                std::string added_filename = std::string(event->name, event->len);
                                if (dir_got->second->prefix + _formatted_date_suffix == added_filename)
                                {
                                    created_files.push_back(std::make_pair(dir_got->first, added_filename));
                                }
                            }
                        }
                        auto file_got = log_file_umap.find(event->wd);
                        if (file_got != log_file_umap.end())
                        {
                            if (event->mask & IN_MODIFY)
                            {
                                log_dir_umap[file_got->second]->log_queue.try_enqueue(MODIFY_TAG);
                            }
                        }
                    }
                }
                for (auto created_file_pair : created_files)
                {
                    if (log_dir_umap[created_file_pair.first]->active_file_fd > 0)
                    {
                        inotify_rm_watch(inotify_instance, created_file_pair.first);
                    }
                    add_watch_file(created_file_pair.second, true, created_file_pair.first);
                }
                created_files.clear();
            }
            else
            {
                ssize_t len = read(sfd, &fdsi, sizeof(struct signalfd_siginfo));
                if (len != sizeof(struct signalfd_siginfo))
                    openrasp_error(E_ERROR, AGENT_ERROR, _("Fail to read."));
                if (fdsi.ssi_signo == SIGTERM)
                {
                    _exit = true;
                    running = false;
                    for (auto iter = log_dir_umap.begin(); iter != log_dir_umap.end(); iter++)
                    {
                        inotify_rm_watch(inotify_instance, iter->first);
                    }
                    for (int i = 0; i < log_dirs.size(); ++i)
                    {
                        log_dirs[i].active_thread.join();
                    }
                    exit(0);
                }
            }
        }
        for (int i = 0; i < openrasp_ini.log_push_interval; ++i)
        {
            sleep(1);
        }
    }
}

void LogCollector::post_logs_via_curl(zval *log_arr, CURL *curl, std::string url_string)
{
    TSRMLS_FETCH();
    smart_str buf_json = {0};
    php_json_encode(&buf_json, log_arr, 0 TSRMLS_CC);
    ResponseInfo res_info;
    if (buf_json.a > buf_json.len)
    {
        buf_json.c[buf_json.len] = '\0';
        buf_json.len++;
    }
    perform_curl(curl, url_string, buf_json.c, res_info);
    if (CURLE_OK != res_info.res)
    {
        openrasp_error(E_WARNING, AGENT_ERROR, _("Fail to post logs to %s."), url_string.c_str());
    }
    smart_str_free(&buf_json);
}

void LogCollector::add_watch_file(std::string filename, bool newly_created, int dir_watch_fd)
{
    TSRMLS_FETCH();
    const char *file_abs_name = (log_dir_umap[dir_watch_fd]->dir_abs_path + _default_slash + filename).c_str();
    if (VCWD_STAT(file_abs_name, &sb) == 0)
    {
        if (S_ISREG(sb.st_mode) && VCWD_ACCESS(file_abs_name, R_OK | F_OK) == 0)
        {
            int inotify_file_watch = inotify_add_watch(inotify_instance, file_abs_name, IN_MODIFY);
            log_file_umap[inotify_file_watch] = dir_watch_fd;
            log_dir_umap[dir_watch_fd]->active_file_fd = inotify_file_watch;
            log_dir_umap[dir_watch_fd]->active_file_name = filename;
            log_dir_umap[dir_watch_fd]->log_queue.try_enqueue(newly_created ? CREATE_TAG : REREAD_TAG);
        }
    }
}

void LogCollector::process_log_push(int dir_watch_fd)
{
    TSRMLS_FETCH();
    zval *log_arr = nullptr;
    MAKE_STD_ZVAL(log_arr);
    array_init(log_arr);
    std::ifstream ifs;
    char task_tag;
    std::string line;
    CURL *curl = curl_easy_init();
    const std::string dir_abs_path = log_dir_umap.at(dir_watch_fd)->dir_abs_path;
    std::string url_string = _backend + log_dir_umap[dir_watch_fd]->backend_url;
    while (true)
    {
        while (log_dir_umap[dir_watch_fd]->log_queue.try_dequeue(task_tag))
        {
            if (task_tag == CREATE_TAG || task_tag == REREAD_TAG)
            {
                if (ifs.is_open())
                {
                    ifs.close();
                    ifs.clear();
                }
                ifs.open(dir_abs_path + _default_slash + log_dir_umap[dir_watch_fd]->active_file_name);
                ifs.seekg(0, task_tag == REREAD_TAG ? std::ios_base::end : std::ios_base::beg);
            }
            else if (task_tag == MODIFY_TAG)
            {
                if (ifs.is_open())
                {
                    while (std::getline(ifs, line))
                    {
                        add_next_index_stringl(log_arr, line.c_str(), line.size(), 1);
                    }
                    ifs.clear();
                }
            }
            else if (task_tag == DELETE_TAG)
            {
                if (ifs.is_open())
                {
                    ifs.close();
                    ifs.clear();
                }
            }
            else
            {
                //skip
            }
        }
        if (zend_hash_num_elements(Z_ARRVAL_P(log_arr)) > 0)
        {
            post_logs_via_curl(log_arr, curl, url_string);
            zend_hash_clean(Z_ARRVAL_P(log_arr));
        }
        for (int i = 0; i < openrasp_ini.log_push_interval; ++i)
        {
            sleep(1);
            if (_exit)
            {
                zval_ptr_dtor(&log_arr);
                curl_easy_cleanup(curl);
                curl = nullptr;
                break;
            }
        }
    }
}

} // namespace openrasp