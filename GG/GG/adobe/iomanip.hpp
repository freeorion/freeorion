/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#ifdef ADOBE_STD_SERIALIZATION

/*************************************************************************************************/

#ifndef ADOBE_IOMANIP_HPP
#define ADOBE_IOMANIP_HPP

/*************************************************************************************************/

#include <GG/adobe/config.hpp>

#include <cassert>
#include <iosfwd>
#include <string>
#include <sstream>
#include <list>
#include <stdexcept>
#include <functional>

#include <GG/adobe/any_regular.hpp>
#include <GG/adobe/iomanip_fwd.hpp>
#include <GG/adobe/manip.hpp>
#include <GG/adobe/name.hpp>

/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/

namespace implementation {

/*************************************************************************************************/

} // namespace implementation

/*************************************************************************************************/

static const aggregate_name_t bag_name_g  = { "bag" };
static const aggregate_name_t seq_name_g  = { "seq" };
static const aggregate_name_t alt_name_g  = { "alt" };
static const aggregate_name_t atom_name_g = { "atom" };

/*************************************************************************************************/

/*!
\defgroup manipulator I/O Manipulators
\ingroup parsing
*/

//!\ingroup manipulator
class format_element_t
{
public:
    explicit format_element_t(name_t             tag = name_t(),
                              const std::string& ident = std::string()) :
        ident_m(ident),
        num_out_m(0),
        tag_m(tag),
        value_m(0)
    { }

    format_element_t(name_t tag, const any_regular_t& value) :
        num_out_m(0),
        tag_m(tag),
        value_m(&value)
    { }

    name_t tag() const
    { return tag_m; }

    const any_regular_t& value() const
    {
        if (!value_m)
            throw std::runtime_error("invalid value");

        return *value_m;
    }

    std::string ident_m;
    std::size_t num_out_m;

private:
    name_t               tag_m;
    const any_regular_t* value_m;
};

/*************************************************************************************************/

//!\ingroup manipulator
class format_base
{
public:
    typedef std::ostream                stream_type;
    typedef format_element_t            stack_value_type;
    typedef std::list<stack_value_type> stack_type;

    virtual ~format_base()
    { }

    virtual void begin_format(stream_type& os) { push_stack(os); }
    virtual void end_format(stream_type& os)   { pop_stack(os); }

    virtual void begin_bag(stream_type& os, const std::string&) { push_stack(os); }
    virtual void end_bag(stream_type& os)                       { pop_stack(os); }

    virtual void begin_sequence(stream_type& os) { push_stack(os); }
    virtual void end_sequence(stream_type& os)   { pop_stack(os); }

    virtual void begin_alternate(stream_type& os) { push_stack(os); }
    virtual void end_alternate(stream_type& os)   { pop_stack(os); }

    virtual void begin_atom(stream_type& os, const any_regular_t&) { push_stack(os); }
    virtual void end_atom(stream_type& os)                         { pop_stack(os); }

    format_base() :
        depth_m(0)
        { }

    virtual std::size_t depth()
        { return depth_m; }

    virtual std::size_t stack_depth()
        { return stack_m.size(); }

protected:

    void push_stack(stream_type&            os,
                    const stack_value_type& element = format_element_t())
    {
        stack_m.push_front(element);
        stack_event(os, true);
    }

    void pop_stack(stream_type& os)
    {
        assert(stack_m.empty() == false);
        stack_event(os, false);
        stack_m.pop_front();
    }

    const stack_value_type& stack_top() const
    { return stack_n(0); }

    stack_value_type& stack_top()
    { return stack_n(0); }

    const stack_value_type& stack_n(std::size_t n) const
    {
        if (n > stack_m.size())
        {
            std::stringstream buf;
            buf << "stack_n: n(" << static_cast<unsigned int>(n) << ") > size(" << static_cast<unsigned int>(stack_m.size()) << ").";
            throw std::range_error(buf.str());
        }

        return *boost::next(stack_m.begin(), n);
    }

    stack_value_type& stack_n(std::size_t n)
    {
        if (n > stack_m.size())
        {
            std::stringstream buf;
            buf << "stack_n: n(" << static_cast<unsigned int>(n) << ") > size(" << static_cast<unsigned int>(stack_m.size()) << ").";
            throw std::range_error(buf.str());
        }

        return *boost::next(stack_m.begin(), n);
    }

    void up()
    { ++depth_m; }

    void down()
    { depth_m = (std::max)(std::size_t(0), depth_m - 1); }

private:
    virtual void stack_event(stream_type& os, bool is_push) = 0;

    std::size_t depth_m; // note: Visual "depth", NOT the depth of the stack
    stack_type  stack_m;
};

/*************************************************************************************************/

format_base::stream_type& begin_format(format_base::stream_type& os);
format_base::stream_type& end_format(format_base::stream_type& os);

format_base::stream_type& end_bag(format_base::stream_type& os);

format_base::stream_type& begin_sequence(format_base::stream_type& os);
format_base::stream_type& end_sequence(format_base::stream_type& os);

format_base::stream_type& begin_alternate(format_base::stream_type& os);
format_base::stream_type& end_alternate(format_base::stream_type& os);

format_base::stream_type& end_atom(format_base::stream_type& os);

/*************************************************************************************************/

//!\ingroup manipulator
class indents : public basic_omanipulator<std::size_t, char, std::char_traits<char> >
{
    typedef basic_omanipulator<std::size_t, char, std::char_traits<char> >      inherited_t;

public:
    typedef inherited_t::stream_type    stream_type;
    typedef inherited_t::argument_type  argument_type;

    indents(argument_type num) :
        inherited_t(indents::fct, num)
        { }

    inherited_t& operator() (argument_type i)
        { arg_m = i; return *this; }

private:
    static stream_type& fct(stream_type& os, const argument_type& i)
    {
        for (argument_type count(0); count < i; ++count)
            os.put('\t');

        return os;
    }
};

/*************************************************************************************************/

//!\ingroup manipulator
class begin_bag : public basic_omanipulator<std::string, char, std::char_traits<char> >
{
    typedef basic_omanipulator<std::string, char, std::char_traits<char> >      inherited_t;

public:
    typedef inherited_t::stream_type    stream_type;
    typedef inherited_t::argument_type  argument_type;

    begin_bag(const argument_type& index) :
        inherited_t(begin_bag::fct, index)
        { }

    inherited_t& operator() (argument_type /*i*/)
        { return *this; }

private:
    static stream_type& fct(stream_type& os, const argument_type& i)
        {
        format_base* format(get_formatter(os));
        if (format) format->begin_bag(os, i);
        return os;
        }
};

/*************************************************************************************************/

//!\ingroup manipulator
template <typename T>
class begin_atom : public basic_omanipulator<const any_regular_t, char, std::char_traits<char> >
{
    typedef basic_omanipulator<const any_regular_t, char, std::char_traits<char> > inherited_t;

public:
    typedef inherited_t::stream_type   stream_type;
    typedef inherited_t::argument_type argument_type;

    explicit begin_atom(const T& x) :
        inherited_t(begin_atom::fct, any_regular_t(x))
        { }

    inherited_t& operator() (argument_type /*i*/)
    { return *this; }

private:
    static stream_type& fct(stream_type& os, const argument_type& atom)
    {
        format_base* format(get_formatter(os));

        if (format == 0)
            return os << atom.cast<typename promote<T>::type>();

        format->begin_atom(os, atom);

        return os;
    }
};

/*************************************************************************************************/

//!\ingroup manipulator
template <typename T, class charT, class traits>
std::basic_ostream<charT, traits>& operator << (std::basic_ostream<charT, traits>& os,
                                                const begin_atom<T>&               manip)
{
    if (os.good())
        manip.do_manip(os);

    return os;
}

/*************************************************************************************************/

//!\ingroup manipulator
template <typename NewFormat>
void callback(std::ios_base::event ev, std::ios_base& strm, int idx)
    {
    if (ev == std::ios_base::erase_event)
        {
        try
            {
            delete static_cast<NewFormat*>(strm.pword(idx));

            strm.pword(idx) = 0;
            }
        catch (...)
            { }
        }
    else if (ev == std::ios_base::copyfmt_event)
        {
        NewFormat* old(static_cast<NewFormat*>(strm.pword(idx)));

        if (old != 0)
            {
            try
                {
                strm.pword(idx) = new NewFormat(*old);
                }
            catch (std::bad_alloc&)
                { }
            }
        }
    }

/*************************************************************************************************/

//!\ingroup manipulator
template <typename OldFormat, typename NewFormat>
void replace_pword(std::ios_base& iob, int idx)
    {
    iob.register_callback(callback<NewFormat>, idx);

    NewFormat* new_format(new NewFormat());
    OldFormat* old_format(static_cast<OldFormat*>(iob.pword(idx)));
    iob.pword(idx) = new_format;
    delete old_format;
    }

/*************************************************************************************************/

//!\ingroup manipulator
template <typename OldFormat, typename NewFormat, typename T>
void replace_pword(std::ios_base& iob, int idx, const T& x)
    {
    iob.register_callback(callback<NewFormat>, idx);

    NewFormat* new_format(new NewFormat(x));
    OldFormat* old_format(static_cast<OldFormat*>(iob.pword(idx)));
    iob.pword(idx) = new_format;
    delete old_format;
    }

/*************************************************************************************************/

//!\ingroup manipulator
template <class T>
std::ostream& fmt(std::ostream& os, const T& t)
{
    os  << begin_atom<T>(t) << end_atom;

    return os;
}

/*************************************************************************************************/

//!\ingroup manipulator
template <class T>
class basic_format : public basic_omanipulator<T, char, std::char_traits<char> >
{
    typedef basic_omanipulator<T, char, std::char_traits<char> >        inherited_t;

public:
    typedef typename inherited_t::stream_type   stream_type;
    typedef typename inherited_t::argument_type argument_type;

    basic_format(const argument_type& t) :
        inherited_t(fmt<T>, t)
        { }

    inherited_t& operator() (const argument_type& /*i*/)
        { return *this; }
};

/*************************************************************************************************/

//!\ingroup manipulator
template <class T>
inline basic_format<T> format(const T& t)
{
    return basic_format<T>(t);
}

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/

#endif

/*************************************************************************************************/

#endif

/*************************************************************************************************/
