/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://opensource.adobe.com/licenses.html)
*/

/**************************************************************************************************/

#ifndef ADOBE_FUNCTION_TRAITS_HPP
#define ADOBE_FUNCTION_TRAITS_HPP

#include <GG/adobe/config.hpp>

#include <boost/function.hpp>

/**************************************************************************************************/

namespace adobe {

/**************************************************************************************************/
/*!
    \defgroup function_traits Function Traits
    \ingroup apl_libraries

    \brief Utility classes for gaining information about types that model ConvertibleToFunction.
*/
/**************************************************************************************************/

template <typename T>
struct function_traits;

/**************************************************************************************************/

template <typename R>
struct function_traits<R()>
{
    enum { arity = 0 };

    typedef R result_type;

    typedef R(function_type)();
    typedef R(&param_type)();
};

template <typename R,
          typename A1>
struct function_traits<R(A1)> :
    public function_traits<R()>
{
    enum { arity = 1 };

    typedef A1 first_argument_type;
    typedef A1 arg1_type;

    typedef R(function_type)(A1);
    typedef R(&param_type)(A1);
};

template <typename R,
          typename A1, typename A2>
struct function_traits<R(A1, A2)> :
    public function_traits<R(A1)>
{
    enum { arity = 2 };

    typedef A1 second_argument_type;
    typedef A2 arg2_type;

    typedef R(function_type)(A1, A2);
    typedef R(&param_type)(A1, A2);
};

template <typename R,
          typename A1, typename A2, typename A3>
struct function_traits<R(A1, A2, A3)> :
    public function_traits<R(A1, A2)>
{
    enum { arity = 3 };

    typedef A3 third_argument_type;
    typedef A3 arg3_type;

    typedef R(function_type)(A1, A2, A3);
    typedef R(&param_type)(A1, A2, A3);
};

template <typename R,
          typename A1, typename A2, typename A3, typename A4>
struct function_traits<R(A1, A2, A3, A4)> :
    public function_traits<R(A1, A2, A3)>
{
    enum { arity = 4 };

    typedef A4 fourth_argument_type;
    typedef A4 arg4_type;

    typedef R(function_type)(A1, A2, A3, A4);
    typedef R(&param_type)(A1, A2, A3, A4);
};

template <typename R,
          typename A1, typename A2, typename A3, typename A4, typename A5>
struct function_traits<R(A1, A2, A3, A4, A5)> :
    public function_traits<R(A1, A2, A3, A4)>
{
    enum { arity = 5 };

    typedef A5 fifth_argument_type;
    typedef A5 arg5_type;

    typedef R(function_type)(A1, A2, A3, A4, A5);
    typedef R(&param_type)(A1, A2, A3, A4, A5);
};

template <typename R,
          typename A1, typename A2, typename A3, typename A4, typename A5,
          typename A6>
struct function_traits<R(A1, A2, A3, A4, A5, A6)> :
    public function_traits<R(A1, A2, A3, A4, A5)>
{
    enum { arity = 6 };

    typedef A6 sixth_argument_type;
    typedef A6 arg6_type;

    typedef R(function_type)(A1, A2, A3, A4, A5, A6);
    typedef R(&param_type)(A1, A2, A3, A4, A5, A6);
};

template <typename R,
          typename A1, typename A2, typename A3, typename A4, typename A5,
          typename A6, typename A7>
struct function_traits<R(A1, A2, A3, A4, A5, A6, A7)> :
    public function_traits<R(A1, A2, A3, A4, A5, A6)>
{
    enum { arity = 7 };

    typedef A7 seventh_argument_type;
    typedef A7 arg7_type;

    typedef R(function_type)(A1, A2, A3, A4, A5, A6, A7);
    typedef R(&param_type)(A1, A2, A3, A4, A5, A6, A7);
};

/**************************************************************************************************/

template <typename R>
struct function_traits<R(*)()> :
    public function_traits<R()>
{
    typedef R(*function_type)();
    typedef R(*param_type)();
};

template <typename R,
          typename A1>
struct function_traits<R(*)(A1)> :
    public function_traits<R(A1)>
{
    typedef R(*function_type)(A1);
    typedef R(*param_type)(A1);
};

template <typename R,
          typename A1, typename A2>
struct function_traits<R(*)(A1, A2)> :
    public function_traits<R(A1, A2)>
{
    typedef R(*function_type)(A1, A2);
    typedef R(*param_type)(A1, A2);
};

template <typename R,
          typename A1, typename A2, typename A3>
struct function_traits<R(*)(A1, A2, A3)> :
    public function_traits<R(A1, A2, A3)>
{
    typedef R(*function_type)(A1, A2, A3);
    typedef R(*param_type)(A1, A2, A3);
};

template <typename R,
          typename A1, typename A2, typename A3, typename A4>
struct function_traits<R(*)(A1, A2, A3, A4)> :
    public function_traits<R(A1, A2, A3, A4)>
{
    typedef R(*function_type)(A1, A2, A3, A4);
    typedef R(*param_type)(A1, A2, A3, A4);
};

template <typename R,
          typename A1, typename A2, typename A3, typename A4, typename A5>
struct function_traits<R(*)(A1, A2, A3, A4, A5)> :
    public function_traits<R(A1, A2, A3, A4, A5)>
{
    typedef R(*function_type)(A1, A2, A3, A4, A5);
    typedef R(*param_type)(A1, A2, A3, A4, A5);
};

template <typename R,
          typename A1, typename A2, typename A3, typename A4, typename A5,
          typename A6>
struct function_traits<R(*)(A1, A2, A3, A4, A5, A6)> :
    public function_traits<R(A1, A2, A3, A4, A5, A6)>
{
    typedef R(*function_type)(A1, A2, A3, A4, A5, A6);
    typedef R(*param_type)(A1, A2, A3, A4, A5, A6);
};

template <typename R,
          typename A1, typename A2, typename A3, typename A4, typename A5,
          typename A6, typename A7>
struct function_traits<R(*)(A1, A2, A3, A4, A5, A6, A7)> :
    public function_traits<R(A1, A2, A3, A4, A5, A6, A7)>
{
    typedef R(*function_type)(A1, A2, A3, A4, A5, A6, A7);
    typedef R(*param_type)(A1, A2, A3, A4, A5, A6, A7);
};

/**************************************************************************************************/

template <typename R>
struct function_traits<R(&)()> :
    public function_traits<R()>
{
    typedef R(&function_type)();
    typedef R(&param_type)();
};

template <typename R,
          typename A1>
struct function_traits<R(&)(A1)> :
    public function_traits<R(A1)>
{
    typedef R(&function_type)(A1);
    typedef R(&param_type)(A1);
};

template <typename R,
          typename A1, typename A2>
struct function_traits<R(&)(A1, A2)> :
    public function_traits<R(A1, A2)>
{
    typedef R(&function_type)(A1, A2);
    typedef R(&param_type)(A1, A2);
};

template <typename R,
          typename A1, typename A2, typename A3>
struct function_traits<R(&)(A1, A2, A3)> :
    public function_traits<R(A1, A2, A3)>
{
    typedef R(&function_type)(A1, A2, A3);
    typedef R(&param_type)(A1, A2, A3);
};

template <typename R,
          typename A1, typename A2, typename A3, typename A4>
struct function_traits<R(&)(A1, A2, A3, A4)> :
    public function_traits<R(A1, A2, A3, A4)>
{
    typedef R(&function_type)(A1, A2, A3, A4);
    typedef R(&param_type)(A1, A2, A3, A4);
};

template <typename R,
          typename A1, typename A2, typename A3, typename A4, typename A5>
struct function_traits<R(&)(A1, A2, A3, A4, A5)> :
    public function_traits<R(A1, A2, A3, A4, A5)>
{
    typedef R(&function_type)(A1, A2, A3, A4, A5);
    typedef R(&param_type)(A1, A2, A3, A4, A5);
};

template <typename R,
          typename A1, typename A2, typename A3, typename A4, typename A5,
          typename A6>
struct function_traits<R(&)(A1, A2, A3, A4, A5, A6)> :
    public function_traits<R(A1, A2, A3, A4, A5, A6)>
{
    typedef R(&function_type)(A1, A2, A3, A4, A5, A6);
    typedef R(&param_type)(A1, A2, A3, A4, A5, A6);
};

template <typename R,
          typename A1, typename A2, typename A3, typename A4, typename A5,
          typename A6, typename A7>
struct function_traits<R(&)(A1, A2, A3, A4, A5, A6, A7)> :
    public function_traits<R(A1, A2, A3, A4, A5, A6, A7)>
{
    typedef R(&function_type)(A1, A2, A3, A4, A5, A6, A7);
    typedef R(&param_type)(A1, A2, A3, A4, A5, A6, A7);
};

/**************************************************************************************************/

template <typename R,
          typename A1>
struct function_traits<R(A1::*)()>
{
    enum { arity = 1 };

    typedef R result_type;

    typedef R(A1::*function_type)();
    typedef R(A1::*param_type)();

    typedef A1* arg1_type;
    typedef A1* first_argument_type;
};

template <typename R,
          typename A1, typename A2>
struct function_traits<R(A1::*)(A2)> :
    public function_traits<R(A1::*)()>
{
    enum { arity = 2 };

    typedef R(A1::*function_type)(A2);
    typedef R(A1::*param_type)(A2);

    typedef A2 arg2_type;
    typedef A2 second_argument_type;
};

template <typename R,
          typename A1, typename A2, typename A3>
struct function_traits<R(A1::*)(A2, A3)> :
    public function_traits<R(A1::*)(A2)>
{
    enum { arity = 3 };

    typedef R(A1::*function_type)(A2, A3);
    typedef R(A1::*param_type)(A2, A3);

    typedef A3 arg3_type;
    typedef A3 third_argument_type;
};

template <typename R,
          typename A1, typename A2, typename A3, typename A4>
struct function_traits<R(A1::*)(A2, A3, A4)> :
    public function_traits<R(A1::*)(A2, A3)>
{
    enum { arity = 4 };

    typedef R(A1::*function_type)(A2, A3, A4);
    typedef R(A1::*param_type)(A2, A3, A4);

    typedef A4 arg4_type;
    typedef A4 fourth_argument_type;
};

template <typename R,
          typename A1, typename A2, typename A3, typename A4, typename A5>
struct function_traits<R(A1::*)(A2, A3, A4, A5)> :
    public function_traits<R(A1::*)(A2, A3, A4)>
{
    enum { arity = 5 };

    typedef R(A1::*function_type)(A2, A3, A4, A5);
    typedef R(A1::*param_type)(A2, A3, A4, A5);

    typedef A5 arg5_type;
    typedef A5 fifth_argument_type;
};

template <typename R,
          typename A1, typename A2, typename A3, typename A4, typename A5,
          typename A6>
struct function_traits<R(A1::*)(A2, A3, A4, A5, A6)> :
    public function_traits<R(A1::*)(A2, A3, A4, A5)>
{
    enum { arity = 6 };

    typedef R(A1::*function_type)(A2, A3, A4, A5, A6);
    typedef R(A1::*param_type)(A2, A3, A4, A5, A6);

    typedef A6 arg6_type;
    typedef A6 sixth_argument_type;
};

template <typename R,
          typename A1, typename A2, typename A3, typename A4, typename A5,
          typename A6, typename A7>
struct function_traits<R(A1::*)(A2, A3, A4, A5, A6, A7)> :
    public function_traits<R(A1::*)(A2, A3, A4, A5, A6)>
{
    enum { arity = 7 };

    typedef R(A1::*function_type)(A2, A3, A4, A5, A6, A7);
    typedef R(A1::*param_type)(A2, A3, A4, A5, A6, A7);

    typedef A7 arg7_type;
    typedef A7 seventh_argument_type;
};

/**************************************************************************************************/

template <typename R,
          typename A1>
struct function_traits<R(A1::*)() const> :
    public function_traits<R(A1::*)()>
{
    typedef R(A1::*function_type)() const;
    typedef R(A1::*param_type)() const;
};

template <typename R,
          typename A1, typename A2>
struct function_traits<R(A1::*)(A2) const> :
    public function_traits<R(A1::*)(A2)>
{
    typedef R(A1::*function_type)(A2) const;
    typedef R(A1::*param_type)(A2) const;
};

template <typename R,
          typename A1, typename A2, typename A3>
struct function_traits<R(A1::*)(A2, A3) const> :
    public function_traits<R(A1::*)(A2, A3)>
{
    typedef R(A1::*function_type)(A2, A3) const;
    typedef R(A1::*param_type)(A2, A3) const;
};

template <typename R,
          typename A1, typename A2, typename A3, typename A4>
struct function_traits<R(A1::*)(A2, A3, A4) const> :
    public function_traits<R(A1::*)(A2, A3, A4)>
{
    typedef R(A1::*function_type)(A2, A3, A4) const;
    typedef R(A1::*param_type)(A2, A3, A4) const;
};

template <typename R,
          typename A1, typename A2, typename A3, typename A4, typename A5>
struct function_traits<R(A1::*)(A2, A3, A4, A5) const> :
    public function_traits<R(A1::*)(A2, A3, A4, A5)>
{
    typedef R(A1::*function_type)(A2, A3, A4, A5) const;
    typedef R(A1::*param_type)(A2, A3, A4, A5) const;
};

template <typename R,
          typename A1, typename A2, typename A3, typename A4, typename A5,
          typename A6>
struct function_traits<R(A1::*)(A2, A3, A4, A5, A6) const> :
    public function_traits<R(A1::*)(A2, A3, A4, A5, A6)>
{
    typedef R(A1::*function_type)(A2, A3, A4, A5, A6) const;
    typedef R(A1::*param_type)(A2, A3, A4, A5, A6) const;
};

template <typename R,
          typename A1, typename A2, typename A3, typename A4, typename A5,
          typename A6, typename A7>
struct function_traits<R(A1::*)(A2, A3, A4, A5, A6, A7) const> :
    public function_traits<R(A1::*)(A2, A3, A4, A5, A6, A7)>
{
    typedef R(A1::*function_type)(A2, A3, A4, A5, A6, A7) const;
    typedef R(A1::*param_type)(A2, A3, A4, A5, A6, A7) const;
};

/**************************************************************************************************/

template <typename F>
struct function_traits<boost::function<F> > :
    public function_traits<F>
{
    typedef boost::function<F>        function_type;
    typedef const boost::function<F>& param_type;
};

/**************************************************************************************************/

} // namespace adobe

/**************************************************************************************************/

#endif

/**************************************************************************************************/
