/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#ifdef ADOBE_STD_SERIALIZATION

/*************************************************************************************************/

#ifndef ADOBE_MANIP_HPP
#define ADOBE_MANIP_HPP

/*************************************************************************************************/

#include <GG/adobe/config.hpp>

#include <iostream>

/*************************************************************************************************/

namespace adobe {


/*************************************************************************************************/

/*!
\addtogroup manipulator
@{
*/

class manipulator_base
{
public:
    manipulator_base() :
        error_m(std::ios_base::goodbit)
        { }

protected:

    template <typename StreamType>
    std::ios_base::iostate handle_error(StreamType& strm) const
        {
        std::ios_base::iostate err(error_m);

        try { throw; }

        catch (std::bad_alloc&)
            {
            set_bad();

            std::ios_base::iostate exception_mask(strm.exceptions());
            
            if (exception_mask & std::ios_base::failbit && !(exception_mask & std::ios_base::badbit))
                strm.setstate(err);
            else if (exception_mask & std::ios_base::badbit)
                {
                try { strm.setstate(err); }
                catch (std::ios_base::failure&) { }
                throw;
                }
            }
        catch (...)
            {
            set_fail();

            std::ios_base::iostate exception_mask(strm.exceptions());

            if ((exception_mask & std::ios_base::badbit) && (err & std::ios_base::badbit))
                strm.setstate(err);
            else if (exception_mask & std::ios_base::failbit)
                {
                try { strm.setstate(err); }
                catch (std::ios_base::failure&) { }
                throw;
                }
            }

        return err;
        }

    void set_fail() const   { error_m |= std::ios_base::failbit; }
    void set_bad() const    { error_m |= std::ios_base::badbit; }

    mutable std::ios_base::iostate  error_m;
};

/*************************************************************************************************/

template <typename ArgumentType, class charT, class traits>
class basic_omanipulator : public manipulator_base
{
public:
    typedef ArgumentType                        argument_type;
    typedef std::basic_ostream<charT, traits>   stream_type;
    typedef stream_type& (*manip_func)(stream_type&, const ArgumentType&);

    basic_omanipulator(manip_func pf, const argument_type& arg) :
        pf_m(pf), arg_m(arg) 
        { }

    void do_manip(stream_type& strm) const
        {
        if (error_m != std::ios_base::goodbit)
            strm.setstate(error_m);
        else
            {
            std::ios_base::iostate err(error_m);
            try
                {
                (*pf_m)(strm, arg_m);
                }
            catch (...)
                {
                err = handle_error(strm);
                }

            if (err) strm.setstate(err);
            }
        }

private:
    manip_func    pf_m;

protected:
    argument_type arg_m;
};

/*************************************************************************************************/

template <typename ArgumentType1, typename ArgumentType2, class charT, class traits>
class basic_omanipulator2 : public manipulator_base
{
public:
    typedef ArgumentType1                       argument_type_1;
    typedef ArgumentType2                       argument_type_2;
    typedef std::basic_ostream<charT, traits>   stream_type;
    typedef stream_type& (*manip_func)(stream_type&, const ArgumentType1&, const ArgumentType2&);

    basic_omanipulator2(manip_func pf, const ArgumentType1& arg1, const ArgumentType2& arg2) :
        pf_m(pf), arg1_m(arg1) , arg2_m(arg2)
        { }

    void do_manip(stream_type& strm) const
        {
        if (error_m != std::ios_base::goodbit)
            strm.setstate(error_m);
        else
            {
            std::ios_base::iostate err(error_m);
            try
                {
                (*pf_m)(strm, arg1_m, arg2_m);
                }
            catch (...)
                {
                err = handle_error(strm);
                }

            if (err) strm.setstate(err);
            }
        }

private:
    manip_func                      pf_m;

protected:
    argument_type_1                 arg1_m;
    argument_type_2                 arg2_m;
};

/*************************************************************************************************/

template <class ArgumentType, class charT, class traits>
std::basic_ostream<charT, traits>& operator << (std::basic_ostream<charT, traits>& os,
                                                const adobe::basic_omanipulator<ArgumentType, charT, traits>& manip)
    {
    if (os.good())
        manip.do_manip(os);

    return os;
    }

/*************************************************************************************************/

template <class ArgumentType1, class ArgumentType2, class charT, class traits>
std::basic_ostream<charT, traits>& operator << (std::basic_ostream<charT, traits>& os,
                                                const adobe::basic_omanipulator2<ArgumentType1, ArgumentType2, charT, traits>& manip)
    {
    if (os.good())
        manip.do_manip(os);

    return os;
    }

/*************************************************************************************************/

template <class charT, class traits>
class basic_bounded_width : public basic_omanipulator<unsigned int, charT, traits>
{
    typedef basic_omanipulator<unsigned int, charT, traits>     inherited_t;

public:
    typedef typename inherited_t::stream_type                   stream_type;
    typedef typename inherited_t::argument_type                 argument_type;

    basic_bounded_width(argument_type min, argument_type max) :
        basic_omanipulator<argument_type, charT, traits>(basic_bounded_width::fct, min),
        min_m(min), max_m(max)
        { }

    inherited_t& operator() (argument_type i)
        {
        inherited_t::arg_m = std::min(max_m, std::max(i, min_m));
        return *this;
        }

private:
    static stream_type& fct(stream_type& strm, const argument_type& i)
        {
        strm.width(i);
        return strm;
        }

    argument_type   min_m;
    argument_type   max_m;
};

typedef basic_bounded_width<char, std::char_traits<char> >          bounded_width;
typedef basic_bounded_width<wchar_t, std::char_traits<wchar_t> >    wbounded_width;

//! @}

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/

#endif

/*************************************************************************************************/

#endif

/*************************************************************************************************/
