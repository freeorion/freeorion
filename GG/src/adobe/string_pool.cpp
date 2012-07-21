/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#include <GG/adobe/implementation/string_pool.hpp>

#include <cassert>
#include <cstddef>
#include <list>
#include <set>

#include <GG/adobe/algorithm/for_each.hpp>
#include <GG/adobe/algorithm/copy.hpp>
#include <GG/adobe/functional.hpp>
#include <GG/adobe/once.hpp>
#include <GG/adobe/string.hpp>

/*************************************************************************************************/

namespace detail {

/**************************************************************************************************/

class string_pool_t : boost::noncopyable
{
private:
    enum { default_pool_size_k = 1024 * 4 };

public:
    explicit string_pool_t(std::size_t pool_size = default_pool_size_k) :
        pool_size_m(pool_size), next_m(NULL), end_m(NULL)
        { }

    ~string_pool_t()
    { adobe::for_each(pool_m, adobe::delete_ptr<char*>()); }

    const char* add(const char* ident)
    {
        std::size_t n = std::strlen(ident);
    
        char* result(allocate(n));

        adobe::copy_n(ident, n, result);
        
        return result;
    }

private:
    char* allocate(std::size_t length)
    {
        std::size_t full_length(length + 1);

        if (full_length > std::size_t(end_m - next_m))
        {
            std::size_t pool_size((std::max)(pool_size_m, full_length));

            pool_m.push_back(static_cast<char*>(::operator new(pool_size)));
            next_m = pool_m.back();
            end_m = next_m + pool_size;
        }

        char* result(next_m);

        next_m += length;
        *next_m++ = '\0';

        return result;
    }

    std::size_t         pool_size_m;
    std::list<char*>    pool_m;
    char*               next_m;
    char*               end_m;
};

/*************************************************************************************************/

} // namespace detail

/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/

struct unique_string_pool_t::implementation_t
{
public:
    implementation_t()
    { }

    // Precondition: length only need be non-zero if not copying
    // Precondition: if str is null then length must be zero
    const char* add(const char* str)
    {
        static const char* empty_string_s = "";

        if (!str || !*str) return empty_string_s;

        name_store_t::iterator iter = store_m.find(str);

        if (iter == store_m.end())
        {
            str = pool_m.add(str);
            iter = store_m.insert(str).first;
        }

        return *iter;
    }

private:
    typedef std::set<const char*, str_less_t> name_store_t;

    name_store_t    store_m;
    ::detail::string_pool_t   pool_m;
};

/*************************************************************************************************/

unique_string_pool_t::unique_string_pool_t() :
    object_m(new implementation_t())
    { }

unique_string_pool_t::~unique_string_pool_t()
    { delete object_m; }

const char* unique_string_pool_t::add(const char* str)
    { return object_m->add(str); }

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/
