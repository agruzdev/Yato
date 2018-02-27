#include "gtest/gtest.h"

#include <cstdint>
#include <iostream>

#include <yato/config/manual/manual_value.h>
#include <yato/config/manual/manual_array.h>
#include <yato/config/manual/manual_object.h>

inline
std::ostream & operator << (std::ostream & os, const yato::manual_array & arr) {
    for(size_t i = 0; i < arr.size(); ++i) {
        if(const auto pval = arr.get_value(i)) {
            os << pval->get<int>(-1) << " ";
        }
        else if(const auto parr = arr.get_array(i)) {
            os << *dynamic_cast<const yato::manual_array*>(parr) << "\n";
        }
    }
    return os;
}

inline
std::ostream & operator << (std::ostream & os, const yato::manual_object & obj) {
    os << "{\n";
    for(const auto & key : obj.keys()) {
        if(const auto pval = obj.get_value(key)) {
            os << "  " << key << ": " << pval->get<int>(-1) << "\n";
        }
        else if(const auto parr = obj.get_array(key)) {
            os << "  " << key << ": " << *dynamic_cast<const yato::manual_array*>(parr) << "\n";
        }
    }
    os << "}\n";
    return os;
}


TEST(Yato_Config, manual_common)
{
    yato::manual_value v(static_cast<int64_t>(42));
    
    std::cout << v.get<int32_t>(-1) << std::endl;
    std::cout << v.get<float>(-1.0f) << std::endl;
    
    v = 7.0f;
    std::cout << v.get<int32_t>(-1) << std::endl;
    std::cout << v.get<float>(-1.0f) << std::endl;
    //std::cout << v.get<int32_t>() << std::endl; // throws
    

    yato::manual_array arr;
    arr.resize(3);
    arr.put(0, 10);
    arr.put(1, 20);
    arr.put(2, 30);
    std::cout << arr.size() << std::endl;
    std::cout << arr << std::endl;
    
    yato::manual_array arr2(arr);
    arr2.resize(4);
    arr2.put(3, 40);
    
    std::cout << arr2 << std::endl;
    
    yato::manual_array arr2d;
    arr2d.resize(3);
    arr2d.append(arr);
    arr2d.append(arr);
    arr2d.append(arr);
    
    std::cout << arr2d;
    
    yato::manual_object obj;
    obj.put("len", arr.size());
    obj.put("arr", arr);
    
    std::cout << "len: "<< obj.get_value("len")->get<size_t>() << std::endl;
    std::cout << "arr: "<< *dynamic_cast<const yato::manual_array*>(obj.get_array("arr")) << std::endl;
    std::cout << obj;
}

