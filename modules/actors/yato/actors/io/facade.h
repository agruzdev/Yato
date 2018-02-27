/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2018 Alexey Gruzdev
*/

#ifndef _YATO_ACTORS_IO_FACADE_H_
#define _YATO_ACTORS_IO_FACADE_H_

namespace yato
{
namespace actors
{
    class actor_system;

namespace io
{

    struct facade
    {
    private:
        /**
         * Initializes IO module
         */
        static
        void init(actor_system & sys);

    public:
        facade() = delete;

        // Only actor system can use the facade
        friend class yato::actors::actor_system;
    };

} // namespace io

} // namespace actors

} // namespace yato


#endif // _YATO_ACTORS_IO_FACADE_H_
