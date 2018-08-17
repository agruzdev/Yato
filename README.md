## Yato

A small repository where I'm gatherting useful snippets and abstractions for C++ development

If you find any bug, please feel free to submit an issue! It will be very helpful for me

## Licence

The MIT License (MIT)

Copyright (c) 2016-2018 Alexey Gruzdev

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions: The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


## Build/Install

The library consists of only headers, so it doesn't require build. The repository provides sources for some tests sources that can be configured and built with the help of cmake file. In order to build the tests you will need to add directory of googletest to the cmake ([GoogleTest repository](https://github.com/google/googletest))

Currently the library can be successfully built by the following compilers:

* MSVC 2017
* MSVC 2015
* MinGW 7.1
* Clang 6.0
* GCC 6.2
* Android (llvm)

### Android:

Use Clang toolchain with libc++ runtime.
The library is tested on NDK 13

## Description

### Actors system

Yato provides a simple actors system implementation, that is developed to look and feel like [Akka](https://akka.io/). 
It supports basic features of an actor system and optional io model for networking (requires `boost` dependency).

[More...](./actors/Actors.md)


### Type support

Yato library provides a set of additional type traits

* **yato::is_shared_ptr** / **yato::is_unique_ptr** - chech if type is a smart pointer
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

### Type matching

**yato::match** performs compile time matching of variable type based on function overloading mechinism 

### "Annotations"

There are a couple of experiments inspired by [Cpp Core Guidelines](https://github.com/isocpp/CppCoreGuidelines/blob/master/CppCoreGuidelines.md)

* **yato::not_null** - pointer wrapper preventing from passing null pointer
* **yato::instance_of** - pointer wrapper checking the dynamic type of passed pointer 

### Ranges

**yato::range** - aggregator of a couple of iterators simplifing passing two iterators to functions, returning iteratos from functions, hepling to hold iterators of one container together. Can be ued in ranged *for* expressions. Together with **yato::numeric_iterator** the range can represent a sequence of integer numbers without actual storing them

Range provides the following functional style operations:

* **map** - returns a range of **yato::transform_iterator** lazily applying a function to the each element of the range
* **filter** - returns a range of **yato::filter_iterator** lazily selecting only those elements of the range which satify some predicate
* **zip** - returns a range of **yato::zip_iterator** joining the range with other ranges into a range of tuples
* **fold_left** - accumulates all elements of the range applying some binary operation from the left to the right
* **fold_right** - accumulates all elements of the range applying some binary operation from the right ot the left

### Containers

Yato implemets a number of general purpose containers: 

* **yato::array_nd** - multidimensional static array with interface and behaviour similar to *std::array*; The layout and size of the **yato::array_nd** are equal to native multidimensional array ( *T[ ][ ]...* )
* **yato::vector_nd** - multidimensional dynamic array similar to *std::vector*; It supports dynamic adding/removing of elements and provides similar interface to std::vector
* **yato::array_view** / **yato::array_view_nd** - non-owning containers which can be attached to any source of data and treat it similar to one-/multi-dimensional array
* **yato::vector_view** - non-owning one-dimensional container providing an interface similar to *std::vector* for a fixed memory buffer

### Reflection

WIP

Current implementation allows to reflect classes and data fields
Reflection information allows to get a list of data fields and its types in compile time, iterate over all data fields and get pointers to fields in run-time

### Tuple algorighms

Yato library provides few compile time algorithms on tuples

* **tuple_transform** - applies unary function to all tuple elements (or binary function to elements of two tuples) and returns tuple with result values
* **tuple_for_each** - calls unary function for the each tuple element 
* **tuple_all_of** - checks if unary predicate returns true for all elements of the tuple (or binary predicate for all elements of two tuples)
* **tuple_any_of** - checks if unary predicate returns true for at least one element of the tuple (or binary predicate for at least one element of two tuples) 

### Iterators

* **yato::numeric_iterator** enumerates sequental integer values allowing to iterate over an integer sequence witout storing it 
* **yato::zip_iterator** is an analogue of [boost::zip_iterator](http://www.boost.org/doc/libs/1_60_0/libs/iterator/doc/zip_iterator.html) but implemented in terms of modern C++ in order to get rid of any boost dependency
* **yato::transform_iterator** is an analogue of [boost::transform_iterator](http://www.boost.org/doc/libs/1_60_0/libs/iterator/doc/transform_iterator.html) but implemented in terms of modern C++ in order to get rid of any boost dependency
* **yato::filter_iterator** is an analogue of [boost::filter_iterator](http://www.boost.org/doc/libs/master/libs/iterator/doc/filter_iterator.html) but implemented in terms of modern C++ in order to get rid of any boost dependency

### Allocator

* **yato::aligning_allocator** is STL-compatible allocator aloowing to alloc heap memory with any alignment. Alignments which are not power of 2 or bigger than alignment of std::max_align_t will have small memory overhead

### Type safe wrappers

* **yato::any** type safe wrapper for any type. Is similar to std::any or [boost::any](http://www.boost.org/doc/libs/1_61_0/doc/html/any.html), but this implementation supports non-copyable and non-movable types as well
* **yato::any_ptr** type safe wrapper for any pointer (i.e. `void*`)
* **yato::variant** type safe wrapper for a specified alternativies. Similar to std::variant but can't have empty state. If empty state is necessary, then `void` should be in alternatives list

Cast between `variants` with different sets of alternatives can be made with the help of `yato::variant_cast`.

There provided effective matchers for `yato::any` and `yato::variant` allowing to handle stored value in `swicth` style.
If no match is found then default case is called (`yato::match_default_t`) if defined, otherwise the exception `yato::bad_match_error` is thrown.

* **yato::any_match** matches stored value of `yato::any`
* **yato::variant_match** matches stored value of `yato::variant`

### Attributes interface

**yato::attributes_interface** is interface class allowing to add arbitrary attributes to a class instance (as key-value pair). Attributes are completely generic and are passed as **yato::any**. 
Only the concrete implementation is able to take or discard passed attribute. Is useful for passing parameters to very different types in one hierarchy so that base classes don't know about parameters types.

There are few generic implementations

* **yato::attributes_map** accepts any attributes and stores in std::map. Is not thread safe
* **yato::ignores_attributes** ignores any attributes passed to the class.
* **yato::atomic_attributes** accepts a number of previously registered attributes. All opetarions with the attrinutes are atomic and thread safe (except for registration). Supports only arithmetic types or pointers
