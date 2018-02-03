#include "gtest/gtest.h"

#include <yato/any_match.h>
#include <yato/variant_match.h>

namespace
{
    class Foo {};
}

TEST(Yato_AnyMatch, common)
{
    Foo f;
    Foo* fp = &f;
    const Foo* cfp = &f;

    auto xi = yato::any(1);
    auto xf = yato::any(1.0f);
    auto xs = yato::any(std::string("abc"));
    const auto cs = yato::any(std::string("abc"));
    auto xp = yato::any(fp);
    auto xe = yato::any();

    auto matcher = yato::any_match(
        [](int) {
            return 1;
        },
        [](const float) {
            return 2;
        },
        [](Foo*) {
            return 3;
        },
        [](std::string &&) {
            return 4;
        },
        [](yato::match_empty_t) {
            return 0;
        },
        [](yato::match_default_t) {
            return -1;
        }
    );

    int r = -1;
    EXPECT_NO_THROW(r = matcher(xi));
    EXPECT_EQ(1, r);
    EXPECT_NO_THROW(r = matcher(xf));
    EXPECT_EQ(2, r);
    EXPECT_NO_THROW(r = matcher(xp));
    EXPECT_EQ(3, r);
    EXPECT_NO_THROW(r = matcher(xs));
    EXPECT_EQ(4, r);
    EXPECT_NO_THROW(r = matcher(std::move(xs)));
    EXPECT_EQ(4, r);
    EXPECT_NO_THROW(r = matcher(cs));
    EXPECT_EQ(-1, r);
    EXPECT_NO_THROW(r = matcher(xe));
    EXPECT_EQ(0, r);
    EXPECT_NO_THROW(r = matcher(yato::any(cfp)));
    EXPECT_EQ(-1, r);
}

TEST(Yato_AnyMatch, common_2)
{
    Foo f;
    Foo* fp = &f;
    const Foo* cfp = &f;

    auto xi = yato::any(1);
    auto xf = yato::any(1.0f);
    auto xs = yato::any(std::string("abc"));
    auto xp = yato::any(fp);
    auto xe = yato::any();

    auto matcher = yato::any_match(
        [](int) {
            return 1;
        },
        [](const float) {
            return 2;
        },
        [](Foo*) {
            return 3;
        },
        [](std::string &&) {
            return 4;
        },
        [](yato::match_empty_t) {
            return 0;
        },
        [](yato::match_default_t) {
            return -1;
        }
    );

    int r = -1;
    EXPECT_NO_THROW(r = matcher(std::move(xi)));
    EXPECT_EQ(1, r);
    EXPECT_NO_THROW(r = matcher(std::move(xf)));
    EXPECT_EQ(2, r);
    EXPECT_NO_THROW(r = matcher(std::move(xp)));
    EXPECT_EQ(3, r);
    EXPECT_NO_THROW(r = matcher(std::move(xs)));
    EXPECT_EQ(4, r);
    EXPECT_NO_THROW(r = matcher(std::move(xe)));
    EXPECT_EQ(0, r);
    EXPECT_NO_THROW(r = matcher(yato::any(cfp)));
    EXPECT_EQ(-1, r);
}

TEST(Yato_AnyMatch, exception)
{
    auto xi = yato::any(1);
    auto xf = yato::any(1.0f);

    auto matcher = yato::any_match(
        [](int) {
            return 10;
        },
        [](yato::match_empty_t) {
            return 20;
        }
    );

    int r = -1;
    EXPECT_NO_THROW(r = matcher(xi));
    EXPECT_EQ(10, r);
    EXPECT_THROW(r = matcher(xf), yato::bad_match_error);
    EXPECT_EQ(10, r);
}

TEST(Yato_VariantMatch, common)
{

    using var_t = yato::variant<void, int, float, std::string, char>;
    auto vi = var_t(1);
    auto vf = var_t(1.0f);
    auto vs = var_t(std::string("abc"));
    auto ve = var_t();
    auto vc = var_t('c');

    auto matcher = yato::variant_match(
        [](int) {
            return 1;
        },
        [](const float) {
            return 2;
        },
        [](Foo*) {
            return 3;
        },
        [](std::string &&) {
            return 4;
        },
        [](yato::match_empty_t) {
            return 0;
        },
        [](yato::match_default_t) {
            return -1;
        }
    );

    int r = -1;
    EXPECT_NO_THROW(r = matcher(vi));
    EXPECT_EQ(1, r);
    EXPECT_NO_THROW(r = matcher(vf));
    EXPECT_EQ(2, r);
    EXPECT_NO_THROW(r = matcher(vs));
    EXPECT_EQ(4, r);
    EXPECT_NO_THROW(r = matcher(ve));
    EXPECT_EQ(0, r);
    EXPECT_NO_THROW(r = matcher(vc));
    EXPECT_EQ(-1, r);
}


