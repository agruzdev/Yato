/**
* YATO library
*
* Apache License, Version 2.0
* Copyright (c) 2016-2020 Alexey Gruzdev
*/

#ifndef _YATO_ACTORS_SCHEDULER_H_
#define _YATO_ACTORS_SCHEDULER_H_

#include <algorithm>
#include <chrono>
#include <functional>
#include <future>
#include <mutex>
#include <vector>

#include "functor.h"
#include "../logger.h"

#if (defined(_MSVC_LANG) && (_MSVC_LANG >= 201703L)) || (defined(__cplusplus) && (__cplusplus > 201700L))
# define YATO_RESULT_OF_T(Fun_, Args_) std::invoke_result_t<Fun_, Args_>
#else
# define YATO_RESULT_OF_T(Fun_, Args_) std::result_of_t<Fun_(Args_)>
#endif

namespace yato
{
namespace actors
{

    class scheduler
    {
    public:
        using clock_type      = std::chrono::high_resolution_clock;
        using time_point_type = std::chrono::high_resolution_clock::time_point;

    private:
        struct event
        {
            time_point_type time;
            std::unique_ptr<void_functor> task;
        };

        std::vector<event> m_events;
        std::mutex m_mutex;
        std::condition_variable m_condition;

        yato::actors::logger_ptr m_log;
        std::thread m_thread;

        bool m_soft_stop  = false;
        bool m_force_stop = false;

        static
        void thread_function(scheduler* self) noexcept
        {
            for (;;) {
                event evt;
                {
                    std::unique_lock<std::mutex> lock(self->m_mutex);
                    if (self->m_force_stop) {
                        return;
                    }
                    while (self->m_events.empty()) {
                        if(self->m_soft_stop) {
                            return;
                        }
                        self->m_condition.wait(lock);
                        if(self->m_force_stop) {
                            return;
                        }
                    }
                    event* earliest = &self->m_events.back();
                    while (earliest->time > std::chrono::high_resolution_clock::now()) {
                        auto time = earliest->time; // local copy
                        self->m_condition.wait_until(lock, time);
                        if (self->m_force_stop) {
                            return;
                        }
                        earliest = &self->m_events.back();
                    }
                    evt = std::move(*earliest);
                    self->m_events.pop_back();
                }
                if(evt.task) {
                    try {
                        (*evt.task)();
                    }
                    catch(std::runtime_error & ex) {
                        self->m_log->error(ex.what());
                    }
                    catch(...) {
                        self->m_log->error("Unknown exception!");
                    }
                }
            }
        }

    public:
        scheduler()
        {
            m_log = yato::actors::logger_factory::create("scheduler");
            m_thread = std::thread(thread_function, this);
        }

        ~scheduler() 
        {
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                m_soft_stop = true;
            }
            m_condition.notify_one();
            m_thread.join();
        }

        /**
         * Stop scheduler without waiting till the rest of events
         */
        void stop()
        {
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                m_force_stop = true;
            }
            m_condition.notify_one();
        }

        /**
         * Enqueue a task which should be executed at specific time point
         */
        template <typename Fn_, typename... Args_>
        auto enqueue(const time_point_type & when, Fn_ && function, Args_ && ... args)
            -> std::future<YATO_RESULT_OF_T(Fn_, Args_...)>
        {
            auto task = std::packaged_task<YATO_RESULT_OF_T(Fn_, Args_...)()>(std::bind(std::forward<Fn_>(function), std::forward<Args_>(args)...));
            auto result = task.get_future();

            event evt;
            evt.time = when;
            evt.task = make_functor_ptr(std::move(task));
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                auto pos = std::upper_bound(m_events.begin(), m_events.end(), evt, [](const event& e1, const event& e2) { return e1.time > e2.time; });
                m_events.insert(pos, std::move(evt));
            }
            m_condition.notify_one();

            return result;
        }
    };

}// namespace actors

}// namespace yato

#undef YATO_RESULT_OF_T

#endif

