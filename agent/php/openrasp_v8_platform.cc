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

#include "openrasp_v8.h"
#include <mutex>
#include <thread>
#include <map>
#include <condition_variable>
#include <chrono>

using namespace openrasp;

class openrasp::TaskQueue
{
  public:
    TaskQueue()
        : terminated_(false) {}

    ~TaskQueue()
    {
        std::lock_guard<std::mutex> guard(lock_);
    }

    void Append(v8::Task *task)
    {
        std::lock_guard<std::mutex> guard(lock_);
        if (!terminated_)
        {
            task_queue_.push(task);
            cv.notify_one();
        }
    }

    v8::Task *GetNext()
    {
        std::unique_lock<std::mutex> lock(lock_);
        cv.wait(lock, [this]() {
            return !task_queue_.empty() || terminated_;
        });
        if (!task_queue_.empty())
        {
            v8::Task *result = task_queue_.front();
            task_queue_.pop();
            return result;
        }
        if (terminated_)
        {
            cv.notify_all();
        }
        return nullptr;
    }

    void Terminate()
    {
        std::lock_guard<std::mutex> guard(lock_);
        terminated_ = true;
        cv.notify_all();
    }

  private:
    std::condition_variable cv;
    std::mutex lock_;
    std::queue<v8::Task *> task_queue_;
    bool terminated_;
};

class openrasp::WorkerThread
{
  public:
    WorkerThread(TaskQueue *queue)
    {
        thread_ = new std::thread([queue]() {
            while (v8::Task *task = queue->GetNext())
            {
                task->Run();
                delete task;
            }
        });
    }

    virtual ~WorkerThread()
    {
        thread_->join();
        delete thread_;
    }

  private:
    std::thread *thread_;
};

V8Platform::V8Platform()
    : initialized_(false)
{
#ifdef ZTS
    thread_pool_size_ = 2;
#else
    thread_pool_size_ = 1;
#endif
    queue_ = new TaskQueue();
}

V8Platform::V8Platform(int thread_pool_size)
    : initialized_(false), thread_pool_size_(thread_pool_size)
{
    queue_ = new TaskQueue();
}

V8Platform::~V8Platform()
{
#ifdef ZTS
    std::lock_guard<std::mutex> guard(lock_);
#endif
    queue_->Terminate();
    if (initialized_)
    {
        for (auto i = thread_pool_.begin(); i != thread_pool_.end(); ++i)
        {
            delete *i;
        }
    }
    delete queue_;
}

size_t V8Platform::NumberOfAvailableBackgroundThreads()
{
    return thread_pool_size_;
}

void V8Platform::CallOnBackgroundThread(v8::Task *task, ExpectedRuntime expected_runtime)
{
    EnsureBackgroundThreadInitialized();
    queue_->Append(task);
}

void V8Platform::CallOnBackgroundThread(v8::Task *task)
{
    CallOnBackgroundThread(task, v8::Platform::kShortRunningTask);
}

void V8Platform::CallOnForegroundThread(v8::Isolate *isolate, v8::Task *task)
{
    task->Run();
    delete task;
}

void V8Platform::CallDelayedOnForegroundThread(v8::Isolate *isolate, v8::Task *task, double delay_in_seconds)
{
    task->Run();
    delete task;
}

double V8Platform::MonotonicallyIncreasingTime()
{
    auto now = std::chrono::high_resolution_clock::now().time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now).count() / 1000;
}

void V8Platform::EnsureBackgroundThreadInitialized()
{
#ifdef ZTS
    std::lock_guard<std::mutex> guard(lock_);
#endif
    if (!initialized_)
    {
        for (int i = 0; i < thread_pool_size_; ++i)
        {
            thread_pool_.push_back(new WorkerThread(queue_));
        }
        initialized_ = true;
    }
}

openrasp::TimeoutTask::TimeoutTask(v8::Isolate *_isolate, int _milliseconds)
    : isolate(_isolate)
{
    time_point = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(_milliseconds);
}

openrasp::TimeoutTask::~TimeoutTask()
{
    std::lock_guard<std::timed_mutex> lock(mtx);
}

void openrasp::TimeoutTask::Run()
{
    do
    {
        // try_lock_until is allowed to fail spuriously and return false
        // even if the mutex is not currently locked by any other thread
        // http://en.cppreference.com/w/cpp/thread/timed_mutex/try_lock_until
        if (mtx.try_lock_until(time_point))
        {
            mtx.unlock();
            return;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    } while (std::chrono::high_resolution_clock::now() < time_point);

    // TerminateExecution can be used by any thread
    // even if that thread has not acquired the V8 lock with a Locker object
    isolate->TerminateExecution();
}

std::timed_mutex &openrasp::TimeoutTask::GetMtx()
{
    return mtx;
}