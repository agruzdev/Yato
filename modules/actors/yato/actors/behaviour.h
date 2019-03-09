/**
* YATO library
*
* Apache License, Version 2.0
* Copyright (c) 2016-2019 Alexey Gruzdev
*/

#ifndef _YATO_ACTOR_BEHAVIOUR_H_
#define _YATO_ACTOR_BEHAVIOUR_H_

#include <yato/any.h>

namespace yato
{
namespace actors
{

    /**
     * Behaviour interface
     */
    class message_consumer
    {
    public:
        virtual ~message_consumer() = default;

        /**
         * Process a message
         */
        virtual void receive(yato::any && message) = 0;
    };


    namespace details
    {
        template <typename ConsumerTy_>
        class receive_wrapper
            : public message_consumer
        {
            ConsumerTy_ m_consumer;

        public:
            receive_wrapper(ConsumerTy_ && receive)
                : m_consumer(std::move(receive))
            { }

            ~receive_wrapper() = default;

            receive_wrapper(const receive_wrapper&) = delete;
            receive_wrapper(receive_wrapper&&) = default;

            receive_wrapper& operator=(const receive_wrapper&) = delete;
            receive_wrapper& operator=(receive_wrapper&&) = default;

            void receive(yato::any && message) override {
                m_consumer(std::move(message));
            }
        };
    }

    /**
     * Create behaviour from a functor
     * ConsumerTy_ should have public method void operator()(yato::any &&);
     */
    template <typename ConsumerTy_>
    std::unique_ptr<message_consumer> make_behaviour(ConsumerTy_ && receiver) {
        return std::make_unique<details::receive_wrapper<yato::remove_cvref_t<ConsumerTy_>>>(std::forward<ConsumerTy_>(receiver));
    }


}// namespace actors

}// namespace yato

#endif //_YATO_ACTOR_BEHAVIOUR_H_
