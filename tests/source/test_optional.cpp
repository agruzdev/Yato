#include "gtest/gtest.h"

#include <string>

#include <yato/optional.h>


namespace
{
    yato::optional<std::string> create(bool b)
    {
        if (b) {
            return yato::make_optional<std::string>("Something");
        }
        else {
            return yato::nullopt_t{};
        }
    }
}

TEST(Yato_Optional, common)
{
    EXPECT_EQ("Empty",     create(false).get_or("Empty"));
    EXPECT_EQ("Something", create(true).get_or("Empty"));
}


TEST(Yato_Optional, common_2)
{
    yato::optional<int> opt1(42);
    EXPECT_EQ(42, opt1.get_or(0));

    yato::optional<int> opt2(41);
    opt1 = opt2;
    EXPECT_EQ(41, opt1.get_or(0));

    opt2 = yato::optional<int>(40);
    opt1.swap(opt2);
    EXPECT_EQ(40, opt1.get_or(0));

    yato::optional<short> x(static_cast<short>(39));
    yato::optional<int> opt4(x);
    EXPECT_EQ(39, opt4.get_or(0));

    yato::optional<char> y(static_cast<char>(38));
    yato::optional<int> opt5(std::move(y));
    EXPECT_EQ(38, opt5.get_or(0));

    //optional<int> opt5(nullptr);

    yato::optional<short> opt6(37);
    opt1 = opt6;
    EXPECT_EQ(37, opt1.get_or(0));
    EXPECT_EQ(37, opt6.get_or(0));
    
    yato::optional<short> opt7(36);
    opt1 = std::move(opt7);
    EXPECT_EQ(36, opt1.get_or(0));

    yato::optional<std::unique_ptr<float>> opt10 = yato::nullopt_t{};
    yato::optional<std::unique_ptr<float>> opt11{std::make_unique<float>(1.0f)};
    
    const auto p1 = std::move(opt11).get_or(nullptr);
    ASSERT_NE(nullptr, p1);
    EXPECT_FLOAT_EQ(1.0f, *p1);
    
    EXPECT_FALSE(static_cast<bool>(opt10));
    EXPECT_TRUE(static_cast<bool>(opt11));
}


namespace
{
    class Foo
    {
        int x_;
    public:
        Foo(int x) : x_(x) {}
        ~Foo() = default;

        Foo(const Foo&) = delete;
        Foo(Foo&&) = delete;

        Foo& operator = (const Foo&) = delete;
        Foo& operator = (Foo&&) = delete;

        int x() const {
            return x_;
        }
    };

    class FooCopy
    {
        int x_;
    public:
        FooCopy(int x) : x_(x) {}
        ~FooCopy() = default;

        FooCopy(const FooCopy&) = default;
        FooCopy(FooCopy&&) = delete;

        FooCopy& operator = (const FooCopy&) = default;
        FooCopy& operator = (FooCopy&&) = delete;
    };

    class FooMove
    {
        int x_;
    public:
        FooMove(int x) : x_(x) {}
        ~FooMove() = default;

        FooMove(const FooMove&) = delete;
        FooMove(FooMove&&) = default;

        FooMove& operator = (const FooMove&) = delete;
        FooMove& operator = (FooMove&&) = default;
    };

    class FooCopyMove
    {
        int x_;
    public:
        FooCopyMove(int x) : x_(x) {}
        ~FooCopyMove() = default;

        FooCopyMove(const FooCopyMove&) = default;
        FooCopyMove(FooCopyMove&&) = default;

        FooCopyMove& operator = (const FooCopyMove&) = default;
        FooCopyMove& operator = (FooCopyMove&&) = default;
    };
}


TEST(Yato_Optional, common_3) 
{
    {
        yato::optional<Foo> opt(yato::in_place_t{}, 1);
        ASSERT_TRUE(static_cast<bool>(opt));

        EXPECT_EQ(1, opt.get().x());

        opt.emplace(7);
        EXPECT_EQ(7, opt.get().x());
    }

    {
        yato::optional<FooCopy> opt(yato::in_place_t{}, 2);
        EXPECT_TRUE(static_cast<bool>(opt));

        const auto opt2(opt);
        opt = opt2;
    }

    {
        auto opt = yato::optional<FooMove>(yato::in_place_t{}, 3);
        EXPECT_TRUE(static_cast<bool>(opt));

        auto opt2(std::move(opt));
        opt = std::move(opt2);
    }

    {
        auto opt = yato::optional<FooCopyMove>(yato::in_place_t{}, 2);
        EXPECT_TRUE(static_cast<bool>(opt));

        const auto opt2(opt);
        opt = opt2;

        auto opt3(std::move(opt));
        opt = std::move(opt3);
    }

}


TEST(Yato_Optional, map)
{
    auto opt = yato::optional<int>(10);
    opt = opt.map([](int x) { return 2 * x; });
    
    EXPECT_NO_THROW(EXPECT_EQ(20, opt.get()));
    
    auto opt2 = opt.map([](int x){ return std::make_unique<float>(x * 2.0f); });
    EXPECT_NO_THROW(ASSERT_NE(nullptr, opt2.get().get()));
    EXPECT_NO_THROW(EXPECT_FLOAT_EQ(40.0f, *opt2.get()));
    
    auto opt3 = std::move(opt2).map([](std::unique_ptr<float> && p){ return *p + 2.0f; });
    EXPECT_FLOAT_EQ(42.0f, opt3.get_or(0.0f));
}
