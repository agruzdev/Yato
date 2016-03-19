## Yato ##

Yato - Yet another toolkit

A small repository where I'm gatherting useful snippets and abstractions for c++ development


##Licence##
The MIT License (MIT)

Copyright (c) 2016 Alexey Gruzdev

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions: The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

## Build/Install ##
The library consists of only headers, so it doesn't require build. The repository provides sources for some tests sources that can be configured and built with the help of cmake file. In order to build the tests you will need to add directory of googletest to the cmake ([GoogleTest repository](https://github.com/google/googletest))

Currently the library cant be successfully used for the following compilers:

* MSVC 2015
* MSVC 2013 (with some limitations)
* MinGW 5.3
* Clang 3.7 (tested with mingw linker)
* GCC 5.3
* Google Android NDK, GCC 4.9

(Android CrystaX NDK support will be added in future)

## Description ##

### Type support ###
Yato library provides a set of additional type traits

* **yato::is_shared_ptr/yato::is_unique_ptr** - chech if type is a smart pointer
* **yato::is_same** - extended version of std::is_same checking that any number of types are same
* **yato::one_of** - checks that type is one of the number of types
* **yato::function_trait** - gets return type and all argument types of a function or function member

Also there are additional tools for working with types. The library provides literals for fixed size types (_u8, _u16, etc.) if user defined literals are supported by compiler. Additional casts are provided:

* **yato::narrow_cast** - casts one arithmetic type to another. If correct value was lost due to narrowing then it throws exception in debug build
* **yato::pointer_cast** - performs reinterpret casting for pointers. Saves CV-cvalifiers and doens't allow to cast intergral value to pointer

Functional traits

* **yato::is_callable** - checks if type is a callable type, i.e. it is pointer to function or it has operator() (including closure type)
* **yato::callable_trait** - deduces return type and all arguments types for any callable type
* **yato::make_function** - converts any callable type to std::function with corresponding return and arguments types

### "Annotations" ###
There are a couple of experiments inspired by [Cpp Core Guidelines](https://github.com/isocpp/CppCoreGuidelines/blob/master/CppCoreGuidelines.md)

* **yato::not_null** - pointer wrapper preventing from passing null pointer
* **yato::instance_of** - pointer wrapper checking the dynamic type of passed pointer 

### Ranges ###
**yato::range** - aggregator of a couple of iterators simplifing passing two iterators to functions, returning iteratos from functions, hepling to hold iterators of one container together. Can be ued in ranged *for* expressions. Together with **yato::numeric_iterator** the range can represent a sequence of integer numbers without actual storing them

Range provides the following functional style operations:

* **map** - returns a range of **yato::transform_iterator** lazily applying a function to the each element of the range
* **filter** - returns a range of **yato::filter_iterator** lazily selecting only those elements of the range which satify some predicate
* **zip** - returns a range of **yato::zip_iterator** joining the range with other ranges into a range of tuples
* **foldLeft** - accumulates all elements of the range applying some binary operation from the left to the right
* **foldRight** - accumulates all elements of the range applying some binary operation from the right ot the left

### Containers ###
Yato implemets a number of general purpose containers: 

* **yato::array_nd** - multidimensional static array with interface and behaviour similar to *std::array*; The layout and size of the **yato::array_nd** are equal to native multidimensional array ( *T[ ][ ]...* )
* **yato::vector_nd** - multidimensional dynamic array similar to *std::vector*; It supports dynamic adding/removing of elements and provides similar interface to std::vector
* **yato::array_view** /** yato::array_view_nd** - non-owning containers which can be attached to any source of data and treat it similar to one-/multi-dimensional array

### Reflection ###
WIP

Current implementation allows to reflect classes and data fields
Reflection information allows to get a list of data fields and its types in compile time, iterate over all data fields and get pointers to fields in run-time

### Tuple algorighms ###
Yato library provides few compile time algorithms on tuples

* **tuple_transform** - applies unary function to all tuple elements (or binary function to elements of two tuples) and returns tuple with result values
* **tuple_for_each** - calls unary function for the each tuple element 
* **tuple_all_of** - checks if unary predicate returns true for all elements of the tuple (or binary predicate for all elements of two tuples)
* **tuple_any_of** - checks if unary predicate returns true for at least one element of the tuple (or binary predicate for at least one element of two tuples) 

### Iterators ###
* **yato::numeric_iterator** enumerates sequental integer values allowing to iterate over an integer sequence witout storing it 
* **yato::zip_iterator** is an analogue of [boost::zip_iterator](http://www.boost.org/doc/libs/1_60_0/libs/iterator/doc/zip_iterator.html) but implemented in terms of modern C++ in order to get rid of any boost dependency
* **yato::transform_iterator** is an analogue of [boost::transform_iterator](http://www.boost.org/doc/libs/1_60_0/libs/iterator/doc/transform_iterator.html) but implemented in terms of modern C++ in order to get rid of any boost dependency
* **yato::filter_iterator** is an analogue of [boost::filter_iterator](http://www.boost.org/doc/libs/master/libs/iterator/doc/filter_iterator.html) but implemented in terms of modern C++ in order to get rid of any boost dependency