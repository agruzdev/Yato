/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2019 Alexey Gruzdev
 */

#include "gtest/gtest.h"

#include <memory>
#include <yato/instance_of.h>

namespace
{
    class Base
    {
        int x;
    public:
        Base(int x)
            : x(x)
        { }

        virtual ~Base()
        { }

        int get_x() const
        {
            return x;
        }

        void set_x(int _x)
        {
            x = _x;
        }
    };

    int foo(const yato::instance_of<Base> ptr) {
        return ptr->get_x() + 1;
    }

    int bar(yato::instance_of<Base> ptr) {
        ptr->set_x(1 + ptr.ref().get_x());
        return ptr.get()->get_x();
    }


    class Derived : public Base
    {
    public:
        Derived(int x)
            : Base(x)
        { }
    };

    void zoo(yato::instance_of<Derived> /*ptr*/)
    { }

    class AnotherBase
    {
    public:
        virtual ~AnotherBase() 
        { }
    };

    class NotDerived : public AnotherBase
    {};
}

TEST(Yato_InstanceOf, instance_of)
{
    
    Base b{ 1 };

    EXPECT_TRUE(foo(&b) == 2);
    EXPECT_TRUE(foo(b) == 2);

    auto d = std::make_unique<Derived>(10);
    EXPECT_TRUE(foo(d.get()) == 11);
    EXPECT_TRUE(foo(*d.get()) == 11);

    EXPECT_TRUE(d->get_x() == 10);
    EXPECT_TRUE(bar(d.get()) == 11);
    EXPECT_TRUE(d->get_x() == 11);

    EXPECT_NO_THROW(zoo(d.get()));
    EXPECT_THROW(zoo(&b), yato::runtime_error);
    Base* bp = d.get();
    EXPECT_NO_THROW(zoo(bp));

    NotDerived nd;
    EXPECT_THROW(foo(nd), yato::runtime_error);
}
