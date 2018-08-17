/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016-2018 Alexey Gruzdev
*/

#ifndef _YATO_ACTORS_THREAD_POOL_H_
#define _YATO_ACTORS_THREAD_POOL_H_

#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#include "../logger.h"

namespace yato
{
namespace actors
{

    /**
     * Simplest thread pool.
     */
    class thread_pool
    {
        std::vector<std::thread> m_threads;
        std::queue<std::function<void()>> m_tasks;

        std::mutex m_mutex;
        std::condition_variable m_cvar;

        bool m_stop = false;

        logger_ptr m_log;

    public:
        thread_pool(size_t threads_num) {
            auto thread_function = [this] {
                for (;;) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(m_mutex);
                        m_cvar.wait(lock, [this] { return m_stop || !m_tasks.empty(); });
                        if (m_stop && m_tasks.empty()) {
                            break;
                        }
                        if (!m_tasks.empty()) {
                            task = std::move(m_tasks.front());
                            m_tasks.pop();
                        }
                    }
                    // Execute
                    if (task) {
                        try {
                            task();
                        }
                        catch (std::exception & e) {
                            m_log->error("yato::actors::thread_pool[thread_function]: Unhandled exception: %s", e.what());
                        }
                        catch (...) {
                            m_log->error("yato::actors::thread_pool[thread_function]: Unhandled exception.");
                        }
                    }
                }
            };

            for(size_t i = 0; i < threads_num; ++i) {
                m_threads.emplace_back(thread_function);
            }
        }

        ~thread_pool() {
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                m_stop = true;
                m_cvar.notify_all();
            }
            for(auto & thread : m_threads) {
                thread.join();
            }
        }

        thread_pool(const thread_pool&) = delete;
        thread_pool& operator=(const thread_pool&) = delete;

        // Add task
        template <typename FunTy_, typename ... Args_>
        void enqueue(FunTy_ && function, Args_ && ... args) {
            std::unique_lock<std::mutex> lock(m_mutex);
            if(!m_stop) {
                m_tasks.emplace(std::bind(std::forward<FunTy_>(function), std::forward<Args_>(args)...));
            } else {
                m_log->warning("yato::actor::thread_pool[enqueue]: Failed to enqueue a new task. Pool is already stopped.");
            }
            m_cvar.notify_one();
        }
    };

} // namespace actors

} // namespace yato

#endif //_YATO_ACTORS_THREAD_POOL_H_
