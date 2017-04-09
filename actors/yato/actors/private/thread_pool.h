/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
*/

#ifndef _YATO_ACTORS_THREADPOOL_H_
#define _YATO_ACTORS_THREADPOOL_H_

#include <vector>
#include <thread>
#include <queue>
#include <future>

namespace yato
{
namespace actors
{

    class pinned_thread_pool
    {
    private:
        std::vector<std::thread> m_threads;

    public:
        pinned_thread_pool() {
        }

        ~pinned_thread_pool() {
            for(auto & t : m_threads) {
                t.join();
            }
        }

        pinned_thread_pool(const pinned_thread_pool&) = delete;
        pinned_thread_pool& operator=(const pinned_thread_pool&) = delete;

        template <typename Callable_>
        void create_thread_for_task(Callable_ && task) {
            m_threads.emplace_back([](Callable_ && t) { t(); }, std::move(task));
        }
    };

} // namespace actors

} // namespace yato

#endif //_YATO_ACTORS_THREADPOOL_H_
