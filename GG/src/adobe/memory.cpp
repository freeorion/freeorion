/*
    Copyright 2008 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#include <cstddef>
#include <new>

#include <GG/adobe/memory.hpp>

namespace {

void* new_s(std::size_t n)
{
    return ::operator new(n, std::nothrow);
}

void delete_s(void* p)
{
    return ::operator delete(p, std::nothrow);
}
    
} // namespace

namespace adobe {
namespace version_1 {

const new_delete_t local_new_delete_g = { new_s, delete_s };

} // namespace version_1
} // namespace adobe
