## Yato ##

Yato - Yet another toolkit

A small repository where I'm gatherting useful snippets and abstractions for c++ development


##Licence##
The MIT License (MIT)

Copyright (c) 2016 Alexey Gruzdev

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions: The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

## Description ##

### "Annotations" ###
There are a couple of experiments inspired by [Cpp Core Guidelines](https://github.com/isocpp/CppCoreGuidelines/blob/master/CppCoreGuidelines.md)

* **yato::not_null** - pointer wrapper preventing from passing null pointer
* **yato::instance_of** - pointer wrapper checking the dynamic type of passed pointer 

### Ranges ###
**yato::range** - aggregator of a couple of iterators simplifing passing two iterators to functions, returning iteratos from functions, hepling to hold iterators of one container together. Can be ued in ranged *for* expressions. Together with **yato::numeric_iterator** the range can represent a sequence of integer numbers without actual storing them

### Containers ###
Yato implemets a number of general purpose containers: 

* **yato::array_nd** - multidimensional static array with interface and behaviour similar to *std::array*; The layout and size of the **yato::array_nd** are equal to native multidimensional array ( T[][]... )
* **yato::vector_nd** - multidimensional dynamic array similar to *std::vector*; It supports dynamic adding/removing of elements and provides similar interface to std::vector
* **yato::array_view** /** yato::array_view_nd** - non-owning containers which can be attached to any source of data and treat it similar to one-/multi-dimensional array

### Reflection ###
WIP