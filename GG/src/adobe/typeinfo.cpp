/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/**************************************************************************************************/

#include <GG/adobe/typeinfo.hpp>

#include <iterator>

#include <GG/adobe/string.hpp>
#include <GG/adobe/cstring.hpp>

/**************************************************************************************************/

namespace adobe {

/**************************************************************************************************/

bad_cast::bad_cast() : what_m("bad_cast") { }

bad_cast::bad_cast(const bad_cast& error) : what_m(error.what_m) { }

/*
REVISIT (sparent) : This is only for debugging, but a reliable way to map a type to a name would
be a useful addition.
*/

bad_cast::bad_cast(const std::type_info& from, const std::type_info& to) :
    what_m("bad_cast: ")
{
    what_m << from.name() << " -> " << to.name();
}

bad_cast::bad_cast(const type_info_t& from, const type_info_t& to) :
    what_m("bad_cast: ")
{
    std::back_insert_iterator<std::string> out(what_m);
    out = serialize(from, out);
    out = copy(boost::as_literal(" -> "), out);
    out = serialize(to, out);
}

bad_cast& bad_cast::operator=(const bad_cast& error)
{ what_m = error.what_m; return *this; }

bad_cast::~bad_cast() throw()
{ }

const char* bad_cast::what() const throw()
{ return what_m.c_str(); }

/**************************************************************************************************/

#ifndef NDEBUG

namespace version_1 {

std::ostream& operator<<(std::ostream& stream, const type_info_t& x)
{
    std::ostream_iterator<char> out(stream);
    serialize(x, out);
    return stream;
}

} // namespace version_1

#endif

/**************************************************************************************************/

namespace implementation {

/**************************************************************************************************/

bool type_instance_t::requires_std_rtti() const
{
    if (type_info_m) return true;
    
    for (const type_instance_t* const* xp = &parameter_m[0]; *xp; ++xp) {
        if ((*xp)->requires_std_rtti()) return true;
    }
    
    return false;
}

bool operator==(const type_instance_t& x, const type_instance_t& y)
{
    /*
        NOTE (sparent@adobe.com) : Because we frequently check type info's when we know they should
        be equal and the identity will be equal unless they have been passed through a DLL boundary,
        this is a valid optimization.
    */
    
    if (&x == &y) return true;

    if (x.type_info_m) {
        if (y.type_info_m) return *x.type_info_m == *y.type_info_m;
        return false;
    }
    if (y.type_info_m) return false;
    
    if (adobe::strcmp(x.name_m, y.name_m) != 0) return false;

    const type_instance_t* const* xp = &x.parameter_m[0];
    const type_instance_t* const* yp = &y.parameter_m[0];
    
    while (*xp && *yp) {
        if (**xp != **yp) return false;
        ++xp;
        ++yp;
    }
    return *xp == *yp; // at least one is zero - both zero implies equality.
}

bool before(const type_instance_t& x, const type_instance_t& y)
{
    if (x.type_info_m) {
        if (y.type_info_m) return x.type_info_m->before(*y.type_info_m) != 0;
        return false; // All local types sort after named types
    }
    if (y.type_info_m) return true;
    
    int c = adobe::strcmp(x.name_m, y.name_m);
    if (c != 0) return c < 0 ? true : false;
    
    const type_instance_t* const* xp = &x.parameter_m[0];
    const type_instance_t* const* yp = &y.parameter_m[0];
    
    /*
        REVISIT (sparent) : The two compares in this loop are necessary for a lexicographical
        compare. An alternative would be a single three - way compare. That might end up being
        less work overall. If too much time is spent in this code then it should be considered.
    */
    
    while (*xp && *yp) {
        if (before(**xp, **yp)) return true;
        if (before(**yp, **xp)) return false;
        ++xp;
        ++yp;
    }
    return *yp != 0;
}

/**************************************************************************************************/

} // namespace implementation

/**************************************************************************************************/

} // namespace adobe

/**************************************************************************************************/
