/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
*/

#ifndef _YATO_SINGLETON_H_
#define _YATO_SINGLETON_H_

#include <memory>

namespace yato
{
    
    /**
     *  Create helper
     */
    template <typename _T>
    struct create_using_new
    {
        static _T* create()
        {
            return new _T;
        }

        static void destroy(_T* ptr)
        {
            delete ptr;
        }
    };

    /**
     *  Singleton abstraction
     *  Simplified approach from Loki library
     */
    template<typename _T, template <typename> class _Creator>
    class singleton_holder
    {
        struct deleter
        {
            void operator()(_T* ptr)
            {
                _Creator<_T>::destroy(ptr);
            }
        };
        using instance_ptr = std::unique_ptr<_T, deleter>;
        static instance_ptr ms_instance;

    public:
        /**
         *  Get singleton instance
         */
        static _T* instance()
        {
            touch();
            return ms_instance.get();
        }

        /**
         *  Get singleton instance
         */
        static _T& instance_ref()
        {
            touch();
            return *ms_instance.get();
        }

        /**
         *  Make sure that the singleton instance is created
         */
        static void touch()
        {
            if (ms_instance.get() == nullptr) {
                ms_instance.reset(_Creator<_T>::create());
            }
        }
    };

    template<typename _T, template <typename> class _Creator>
    typename singleton_holder<_T, _Creator>::instance_ptr singleton_holder<_T, _Creator>::ms_instance;

}

#endif
