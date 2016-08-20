#include "gtest/gtest.h"

#include <yato/storage.h>

namespace
{
    template <size_t _Size>
    class Foo
    {
        char x[_Size];

        Foo(Foo && other) = delete;
        Foo& operator=(const Foo&) = delete;
        Foo& operator=(Foo&&) = delete;
    public:
        Foo(char c)
        {
            x[0] = c;
        }

        Foo(const Foo & other)
        {
            x[0] = other.x[0];
        }

        ~Foo()
        {
        }

        char foo() const
        {
            return x[0];
        }
    };
}

TEST(Yato_Storage, base)
{
    using A = Foo<1>;
    static_assert(sizeof(yato::storage<A>) == sizeof(A), "yato::storage fail");
    static_assert(!yato::storage<A>::allocates_memory, "yato::storage fail");

    using B = Foo<8>;
    static_assert(sizeof(yato::storage<B>) == sizeof(B), "yato::storage fail");
    static_assert(!yato::storage<B>::allocates_memory, "yato::storage fail");

    using C = Foo<1024>;
    static_assert(sizeof(yato::storage<C>) == sizeof(void*), "yato::storage fail");
    static_assert(yato::storage<C>::allocates_memory, "yato::storage fail");
}

namespace
{
    template <size_t _ObjSize>
    void TestFunction_ctor()
    {
        using A = Foo<_ObjSize>;
        yato::storage<A> a(A{'a'});
        EXPECT_EQ('a', a.get()->foo());
        const yato::storage<A> b(a);
        EXPECT_EQ('a', b.get()->foo());

        yato::storage<A> c(std::move(a));
    }

    template <size_t _ObjSize>
    void TestFunction_swap()
    {
        using A = Foo<_ObjSize>;

        yato::storage<A> a(A{ 'a' });
        yato::storage<A> b(A{ 'b' });

        EXPECT_EQ('a', a->foo());
        EXPECT_EQ('b', b->foo());

        a.swap(b);

        EXPECT_EQ('b', a->foo());
        EXPECT_EQ('a', b->foo());

        using std::swap;
        swap(a, b);

        EXPECT_EQ('a', a->foo());
        EXPECT_EQ('b', b->foo());
    }

    template <size_t _ObjSize>
    void TestFunction_assign()
    {
        using A = Foo<_ObjSize>;

        yato::storage<A> a(A{ 'a' });
        yato::storage<A> b(A{ 'b' });

        EXPECT_EQ('a', a->foo());
        EXPECT_EQ('b', b->foo());

        a = b;
        EXPECT_EQ('b', a->foo());

        yato::storage<A> c(A{ 'c' });
        EXPECT_EQ('c', c->foo());
        c = std::move(a);
        EXPECT_EQ('b', c->foo());
    }
}

TEST(Yato_Storage, ctor)
{
    TestFunction_ctor<4>();
    TestFunction_ctor<128>();
}

TEST(Yato_Storage, swap)
{
    TestFunction_swap<4>();
    TestFunction_swap<128>();
}

TEST(Yato_Storage, assign)
{
    TestFunction_assign<4>();
    TestFunction_assign<128>();
}
