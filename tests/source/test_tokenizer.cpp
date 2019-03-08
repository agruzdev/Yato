#include "gtest/gtest.h"

#include <cstring>
#include <yato/tokenizer.h>

TEST(Yato_Tokenizer, common)
{
    const std::string str = "The quick brown fox jumps over the lazy dog";
    auto t = yato::ctokenize(str, ' ');
    std::vector<std::string> words;
    while(t.has_next()) {
        auto token = t.next();
        words.emplace_back(token.begin(), token.end());
    }
    ASSERT_EQ(static_cast<size_t>(9), words.size());
    ASSERT_EQ("The", words[0]);
    ASSERT_EQ("quick", words[1]);
    ASSERT_EQ("brown", words[2]);
    ASSERT_EQ("fox", words[3]);
    ASSERT_EQ("jumps", words[4]);
    ASSERT_EQ("over", words[5]);
    ASSERT_EQ("the", words[6]);
    ASSERT_EQ("lazy", words[7]);
    ASSERT_EQ("dog", words[8]);
}

TEST(Yato_Tokenizer, wcommon)
{
    const std::wstring str = L"The quick brown fox jumps over the lazy dog";
    auto t = yato::ctokenize(str, L' ');
    std::vector<std::wstring> words;
    while(t.has_next()) {
        auto token = t.next();
        words.emplace_back(token.begin(), token.end());
    }
    ASSERT_EQ(static_cast<size_t>(9), words.size());
    ASSERT_EQ(L"The", words[0]);
    ASSERT_EQ(L"quick", words[1]);
    ASSERT_EQ(L"brown", words[2]);
    ASSERT_EQ(L"fox", words[3]);
    ASSERT_EQ(L"jumps", words[4]);
    ASSERT_EQ(L"over", words[5]);
    ASSERT_EQ(L"the", words[6]);
    ASSERT_EQ(L"lazy", words[7]);
    ASSERT_EQ(L"dog", words[8]);
}

TEST(Yato_Tokenizer, common_2)
{
    const char* url = "/dir/etc//folder///img.jpeg";
    {
        auto t = yato::ctokenize_n(url, std::strlen(url), '/');
        std::vector<std::string> all_words;
        while(t.has_next()) {
            auto token = t.next();
            all_words.emplace_back(token.begin(), token.end());
        }
        ASSERT_EQ(static_cast<size_t>(8), all_words.size());
        ASSERT_EQ("dir", all_words[1]);
        ASSERT_EQ("etc", all_words[2]);
        ASSERT_EQ("folder", all_words[4]);
        ASSERT_EQ("img.jpeg", all_words[7]);
    }
    {
        auto t = yato::ctokenize_n(url, std::strlen(url), '/', true);
        std::vector<std::string> words;
        while(t.has_next()) {
            auto token = t.next();
            words.emplace_back(token.begin(), token.end());
        }
        ASSERT_EQ(static_cast<size_t>(4), words.size());
        ASSERT_EQ("dir", words[0]);
        ASSERT_EQ("etc", words[1]);
        ASSERT_EQ("folder", words[2]);
        ASSERT_EQ("img.jpeg", words[3]);
    }
    {
        auto t = yato::tokenize_if(url, url + strlen(url), [](char c){return c == '/' || c == '.'; }, true);
        std::vector<std::string> words;
        while(t.has_next()) {
            auto token = t.next();
            words.emplace_back(token.begin(), token.end());
        }
        ASSERT_EQ(static_cast<size_t>(5), words.size());
        ASSERT_EQ("dir", words[0]);
        ASSERT_EQ("etc", words[1]);
        ASSERT_EQ("folder", words[2]);
        ASSERT_EQ("img", words[3]);
        ASSERT_EQ("jpeg", words[4]);
    }
}

TEST(Yato_Tokenizer, nonconst)
{
    std::string str = "The quick brown fox jumps over the lazy dog";
    auto t = yato::tokenize(str, ' ');

    while(t.has_next()) {
        auto token = t.next();
        std::transform(token.begin(), token.end(), token.begin(), [](char c){ return static_cast<char>(::toupper(c)); });
    }
    
    ASSERT_EQ("THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG", str);
}


#if defined(YATO_MSVC_2017) || defined(YATO_CXX17)

TEST(Yato_Tokenizer, common_range)
{
    const std::string str = "The quick brown fox jumps over the lazy dog";
    std::vector<std::string> words;
    for (const auto & token : yato::tokens_crange(str, ' ')) {
        words.emplace_back(token.begin(), token.end());
    }
    ASSERT_EQ(static_cast<size_t>(9), words.size());
    ASSERT_EQ("The", words[0]);
    ASSERT_EQ("quick", words[1]);
    ASSERT_EQ("brown", words[2]);
    ASSERT_EQ("fox", words[3]);
    ASSERT_EQ("jumps", words[4]);
    ASSERT_EQ("over", words[5]);
    ASSERT_EQ("the", words[6]);
    ASSERT_EQ("lazy", words[7]);
    ASSERT_EQ("dog", words[8]);
}

TEST(Yato_Tokenizer, nonconst_range)
{
    std::wstring str = L"The quick brown fox jumps over the lazy dog";

    for (const auto & token : yato::tokens_range(str, L' ')) {
        std::transform(token.begin(), token.end(), token.begin(), [](wchar_t c){ return static_cast<char>(::towupper(c)); });
    }

    std::wstring ref = L"THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG";
    ASSERT_TRUE(std::equal(str.cbegin(), str.cend(), ref.cbegin()));
}

TEST(Yato_Tokenizer, common_2_range)
{
    const char* url = "/dir/etc//folder///img.jpeg";
    {
        std::vector<std::string> all_words;
        for (const auto & token : yato::tokens_crange_n(url, strlen(url), '/')) {
            all_words.emplace_back(token.begin(), token.end());
        }
        ASSERT_EQ(static_cast<size_t>(8), all_words.size());
        ASSERT_EQ("dir", all_words[1]);
        ASSERT_EQ("etc", all_words[2]);
        ASSERT_EQ("folder", all_words[4]);
        ASSERT_EQ("img.jpeg", all_words[7]);
    }
    {
        std::vector<std::string> words;
        for (const auto & token : yato::tokens_crange_n(url, strlen(url), '/', true)) {
            words.emplace_back(token.begin(), token.end());
        }
        ASSERT_EQ(static_cast<size_t>(4), words.size());
        ASSERT_EQ("dir", words[0]);
        ASSERT_EQ("etc", words[1]);
        ASSERT_EQ("folder", words[2]);
        ASSERT_EQ("img.jpeg", words[3]);
    }
    {
        std::vector<std::string> words;
        for (const auto & token : yato::tokens_range_if(url, url + strlen(url), [](char c){return c == '/' || c == '.'; }, true)) {
            words.emplace_back(token.begin(), token.end());
        }
        ASSERT_EQ(static_cast<size_t>(5), words.size());
        ASSERT_EQ("dir", words[0]);
        ASSERT_EQ("etc", words[1]);
        ASSERT_EQ("folder", words[2]);
        ASSERT_EQ("img", words[3]);
        ASSERT_EQ("jpeg", words[4]);
    }
}

#endif //YATO_CXX17


