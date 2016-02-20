#include "gtest/gtest.h"

#include <memory>
#include <yato/invoke.h>

TEST(Yato_Invoke, invoke)
{
    class Foo 
    {
    public:
        bool foo(int x) {
            return (x == 42);
        }

        void zoo() const {
        }
    };

    class Bar
    {
    public:
        bool foo(int x) {
            return (x != 42);
        }
    };

    Foo f;
    Bar b;
    yato::invoke(f, &Foo::foo, 1);
    //yato::invoke(b, &Foo::foo, 1); //compile time error
    yato::invoke(&b, &Bar::foo, 2);

    yato::invoke(Foo(), &Foo::foo, 2);

    auto p = std::make_unique<Foo>();
    yato::invoke(p, &Foo::foo, 42);

    yato::invoke(std::make_shared<Bar>(), &Bar::foo, 1);

    yato::invoke(f, &Foo::zoo);
    const Foo f2{};
    yato::invoke(f2, &Foo::zoo);

    EXPECT_TRUE(true);
}

namespace
{
    class I
    {
    public:
        virtual ~I() {};

        virtual void foo() = 0;
    };

    class A : public I
    {
    public:
        void foo() override { }
    };
}

TEST(Yato_Invoke, invoke_2)
{
    yato::invoke(A(), &I::foo);
    yato::invoke(A(), &A::foo);

    EXPECT_TRUE(true);
}
