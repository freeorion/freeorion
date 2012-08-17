/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/**************************************************************************************************/

#if defined(ADOBE_STD_SERIALIZATION)

#include <GG/adobe/any_regular.hpp>

#include <cassert>

#include <GG/adobe/algorithm/sorted.hpp>
#include <GG/adobe/algorithm/lower_bound.hpp>
#include <GG/adobe/array.hpp>
#include <GG/adobe/dictionary.hpp>
#include <GG/adobe/name.hpp>
#include <GG/adobe/string.hpp>
#include <GG/adobe/table_index.hpp>

#include <ostream>

/**************************************************************************************************/

namespace {

using namespace adobe;

/**************************************************************************************************/

} // namespace

/**************************************************************************************************/

namespace adobe {

/**************************************************************************************************/

namespace implementation {

/**************************************************************************************************/

struct serializable_t
{
    virtual ~serializable_t() { };
    virtual void operator () (std::ostream& out, const any_regular_t& x) const = 0;
};

/**************************************************************************************************/

template <typename T>
struct serializable : serializable_t {
    void operator () (std::ostream& out, const any_regular_t& x) const
    { out << format(x.cast<T>()); }
};

/**************************************************************************************************/

template <typename T, typename Any = void>
struct make_serializable { static const serializable<T> value; };

template <typename T, typename Any>
const serializable<T> make_serializable<T, Any>::value = serializable<T>();

/**************************************************************************************************/

/*
    WARNING (sparent) : This table is in a static sorted order.
    
    bool
    closed_hash_map:version_1:adobe<name_t:version_1:adobe,any_regular_t:version_1:adobe>
    double
    empty_t:version_1:adobe
    name_t:version_1:adobe
    string_t:version_1:adobe
    vector:version_1:adobe<any_regular_t:version_1:adobe>
*/

typedef const aggregate_pair<aggregate_type_info_t, const serializable_t&> serializable_lookup_t;

serializable_lookup_t serializable_table[] = {
    { { make_type_info<bool>::value }, make_serializable<bool>::value },
    { { make_type_info<dictionary_t>::value }, make_serializable<dictionary_t>::value },
    { { make_type_info<double>::value }, make_serializable<double>::value },
    { { make_type_info<empty_t>::value }, make_serializable<empty_t>::value },
    { { make_type_info<name_t>::value }, make_serializable<name_t>::value },
    { { make_type_info<string_t>::value }, make_serializable<string_t>::value },
    { { make_type_info<array_t>::value }, make_serializable<array_t>::value }
};

/**************************************************************************************************/

} // namespace implementation

/**************************************************************************************************/

namespace version_1 {

std::ostream& operator<<(std::ostream& out, const any_regular_t& x)
{
    using namespace implementation;
    
#ifndef NDEBUG
    static bool inited = false;
    
    if (!inited) {
        inited = true;
        assert(is_sorted(serializable_table, &type_info_t::before,
            boost::bind(constructor<type_info_t>(),  boost::bind(&serializable_lookup_t::first, _1)))
            && "FATAL (sparent): Serialization table is not sorted.");
    }
#endif
    
    serializable_lookup_t* i = binary_search(serializable_table, x.type_info(), &type_info_t::before,
        boost::bind(constructor<type_info_t>(), boost::bind(&serializable_lookup_t::first, _1)));
    
    if (i == boost::end(serializable_table)) throw std::logic_error("Type not serializable.");
    
    i->second(out, x);
    
    return out;
}

std::ostream& operator<<(std::ostream& out, const dictionary_t& x)
{
    typedef table_index<const name_t, const dictionary_t::value_type> index_type;

    index_type index(x.begin(), x.end(), &dictionary_t::value_type::first);
    index.sort();

    out << begin_bag("[0]");
    
    for (index_type::const_iterator first(index.begin()), last(index.end()); first != last; ++first)
    {
        out << begin_sequence;
            out  << format(first->first);
            out  << format(first->second);
        out << end_sequence;
    }
    
    out << end_bag;
    
    return out;
}

} // namespace version_1

/**************************************************************************************************/

} // namespace adobe

/**************************************************************************************************/

#endif
