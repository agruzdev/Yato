#include "gtest/gtest.h"

#include <yato/not_null.h>

//Works for constexpr variables
constexpr int y = 1;
constexpr auto x1 = yato::not_null<const int*>(&y);

class A
{   
public: 
    int x = 1;
};

void foo(yato::not_null<int*> p){
	*p += 1;
    EXPECT_TRUE(2 == *p);
}

void bar(yato::not_null<const A*> p){
    EXPECT_TRUE(1 == p->x);
}

void baz(yato::not_null< std::shared_ptr<float> > p){
    EXPECT_TRUE(2.0f == *p);
}

void zoo(yato::not_null< std::unique_ptr<float> > p){
    EXPECT_TRUE(6.0f == *p);
}

TEST(Yato_NotNull, Test1) 
{
	int a = 1;
    yato::not_null<int*> p1(&a);
    //not_null<int*> p2(nullptr); // compile time error
    
    foo(&a);

    auto pa = new A{};
    bar(pa);
    delete pa;
 
    auto f = std::make_shared<float>(2.0f);
    baz(f);
    baz(std::make_shared<float>(2.0f));
    
    zoo(std::make_unique<float>(6.0f));
    auto u = std::make_unique<float>(6.0f);
    zoo(u);
}



TEST(Yato_NotNull, Test2)
{
	yato::not_null< std::unique_ptr<int> > p1 = std::make_unique<int>(1);
}
