/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://opensource.adobe.com/licenses.html)
*/

/******************************************************************************/

#ifndef ADOBE_FUNCTION_PACK_HPP
#define ADOBE_FUNCTION_PACK_HPP

#include <GG/adobe/config.hpp>

#include <stdexcept>
#include <sstream>

#include <boost/bind.hpp>
#include <boost/static_assert.hpp>
#include <boost/type_traits.hpp>
#include <boost/utility/enable_if.hpp>

#include <GG/adobe/any_regular.hpp>
#include <GG/adobe/array.hpp>
#include <GG/adobe/closed_hash.hpp>
#include <GG/adobe/dictionary.hpp>
#include <GG/adobe/empty.hpp>
#include <GG/adobe/function_traits.hpp>
#include <GG/adobe/string.hpp>
#include <GG/adobe/virtual_machine.hpp>

/******************************************************************************/

namespace adobe {

/******************************************************************************/
#ifndef ADOBE_NO_DOCUMENTATION
namespace implementation {

/******************************************************************************/
/*!
    undecorate is used to determine the regular type stored in any_regular_t for a particular
    argument type to a function. The only undecorating that needs to happen, really, is
    reference removal. Constness removal is OK, as the constness will be added back by any_regular_t
    if the any_regular_t itself is const.
*/
template <typename T>
struct undecorate
{ typedef T type; };

template <typename T>
struct undecorate<const T>
{ typedef T type; };

template <typename T>
struct undecorate<T&>
{ typedef T type; };

template <typename T>
struct undecorate<const T&>
{ typedef T type; };

/******************************************************************************/

inline any_regular_t& find_arg(dictionary_t& named_argument_set, name_t arg_name)
{
    dictionary_t::iterator arg(named_argument_set.find(arg_name));

    if (arg == named_argument_set.end())
        throw std::runtime_error(make_string("No value for named argument '", arg_name.c_str(), "'"));

    return arg->second;
}

inline const any_regular_t& find_arg(const dictionary_t& named_argument_set, name_t arg_name)
{
    return find_arg(const_cast<dictionary_t&>(named_argument_set), arg_name);
}

inline any_regular_t& find_arg(array_t& argument_set, std::size_t index)
{
    if (argument_set.size() < (index + 1))
    {
        std::stringstream error;

        error << "No value for unnamed argument " << (index + 1);

        throw std::runtime_error(error.str());
    }

    return argument_set[index];
}

inline const any_regular_t& find_arg(const array_t& argument_set, std::size_t index)
{
    return find_arg(const_cast<array_t&>(argument_set), index);
}

/******************************************************************************/

#define ADOBE_FUNCTION_UNNAMED_ARG(param_index) \
find_arg(argument_set, (param_index - 1)).cast<typename undecorate<typename traits_type::arg##param_index##_type>::type>()

#define ADOBE_FUNCTION_NAMED_ARG(param_index) \
find_arg(named_argument_set, name##param_index##_m).cast<typename undecorate<typename traits_type::arg##param_index##_type>::type>()

/******************************************************************************/

template <typename T>
struct novoid
{ typedef T type; };

template <>
struct novoid<void>
{ typedef adobe::empty_t type; };

/******************************************************************************/

template <typename F>
inline typename boost::disable_if<boost::is_same<typename F::result_type,
                                                 void>,
                                  typename F::result_type>::type
invoke_novoid(const F& f)
{
    return f();
}

template <typename F>
inline typename boost::enable_if<boost::is_same<typename F::result_type,
                                                void>,
                                 adobe::empty_t>::type
invoke_novoid(const F& f)
{
    f();

    return adobe::empty_t();
}

/******************************************************************************/

template <typename F, std::size_t Arity>
struct function_pack_helper;

/******************************************************************************/

template <typename F>
struct function_pack_helper<F, 0>
{
    typedef function_traits<F>                        traits_type;
    typedef typename traits_type::result_type         traits_result_type;
    typedef typename novoid<traits_result_type>::type result_type;

    explicit function_pack_helper(const F& f) :
        f_m(f)
    { }

    template <typename T>
    result_type operator()(const T&) const
    {
        return invoke_novoid(boost::bind(f_m));
    }

private:
    typename traits_type::function_type f_m;
};

/******************************************************************************/

template <typename F>
struct function_pack_helper<F, 1>
{
    typedef function_traits<F>                        traits_type;
    typedef typename traits_type::result_type         traits_result_type;
    typedef typename novoid<traits_result_type>::type result_type;

    explicit function_pack_helper(const F& f,
                                  name_t   arg1_name = name_t()) :
        f_m(f),
        name1_m(arg1_name)
    { }

    result_type operator()(const array_t& argument_set) const
    {
        return invoke_novoid(boost::bind(f_m, ADOBE_FUNCTION_UNNAMED_ARG(1)));
    }

    result_type operator()(const dictionary_t& named_argument_set) const
    {
        return invoke_novoid(boost::bind(f_m, ADOBE_FUNCTION_NAMED_ARG(1)));
    }

private:
    typename traits_type::function_type f_m;
    name_t name1_m;
};

/******************************************************************************/

template <typename F>
struct function_pack_helper<F, 2>
{
    typedef function_traits<F>                        traits_type;
    typedef typename traits_type::result_type         traits_result_type;
    typedef typename novoid<traits_result_type>::type result_type;

    explicit function_pack_helper(const F& f,
                                  name_t   arg1_name = name_t(), name_t arg2_name = name_t()) :
        f_m(f),
        name1_m(arg1_name), name2_m(arg2_name)
    { }

    result_type operator()(const array_t& argument_set) const
    {
        return invoke_novoid(boost::bind(f_m,
                                         ADOBE_FUNCTION_UNNAMED_ARG(1), ADOBE_FUNCTION_UNNAMED_ARG(2)));
    }

    result_type operator()(const dictionary_t& named_argument_set) const
    {
        return invoke_novoid(boost::bind(f_m,
                                         ADOBE_FUNCTION_NAMED_ARG(1), ADOBE_FUNCTION_NAMED_ARG(2)));
    }

private:
    typename traits_type::function_type f_m;
    name_t name1_m;
    name_t name2_m;
};

/******************************************************************************/

template <typename F>
struct function_pack_helper<F, 3>
{
    typedef function_traits<F>                        traits_type;
    typedef typename traits_type::result_type         traits_result_type;
    typedef typename novoid<traits_result_type>::type result_type;

    explicit function_pack_helper(const F& f,
                                  name_t   arg1_name = name_t(), name_t arg2_name = name_t(),
                                  name_t   arg3_name = name_t()) :
        f_m(f),
        name1_m(arg1_name), name2_m(arg2_name), name3_m(arg3_name)
    { }

    result_type operator()(const array_t& argument_set) const
    {
        return invoke_novoid(boost::bind(f_m,
                                         ADOBE_FUNCTION_UNNAMED_ARG(1), ADOBE_FUNCTION_UNNAMED_ARG(2),
                                         ADOBE_FUNCTION_UNNAMED_ARG(3)));
    }

    result_type operator()(const dictionary_t& named_argument_set) const
    {
        return invoke_novoid(boost::bind(f_m,
                                         ADOBE_FUNCTION_NAMED_ARG(1), ADOBE_FUNCTION_NAMED_ARG(2),
                                         ADOBE_FUNCTION_NAMED_ARG(3)));
    }

private:
    typename traits_type::function_type f_m;
    name_t name1_m;
    name_t name2_m;
    name_t name3_m;
};

/******************************************************************************/

template <typename F>
struct function_pack_helper<F, 4>
{
    typedef function_traits<F>                        traits_type;
    typedef typename traits_type::result_type         traits_result_type;
    typedef typename novoid<traits_result_type>::type result_type;

    explicit function_pack_helper(const F& f,
                                  name_t   arg1_name = name_t(), name_t arg2_name = name_t(),
                                  name_t   arg3_name = name_t(), name_t arg4_name = name_t()) :
        f_m(f),
        name1_m(arg1_name), name2_m(arg2_name), name3_m(arg3_name), name4_m(arg4_name)
    { }

    result_type operator()(const array_t& argument_set) const
    {
        return invoke_novoid(boost::bind(f_m,
                                         ADOBE_FUNCTION_UNNAMED_ARG(1), ADOBE_FUNCTION_UNNAMED_ARG(2),
                                         ADOBE_FUNCTION_UNNAMED_ARG(3), ADOBE_FUNCTION_UNNAMED_ARG(4)));
    }

    result_type operator()(const dictionary_t& named_argument_set) const
    {
        return invoke_novoid(boost::bind(f_m,
                                         ADOBE_FUNCTION_NAMED_ARG(1), ADOBE_FUNCTION_NAMED_ARG(2),
                                         ADOBE_FUNCTION_NAMED_ARG(3), ADOBE_FUNCTION_NAMED_ARG(4)));
    }

private:
    typename traits_type::function_type f_m;
    name_t name1_m;
    name_t name2_m;
    name_t name3_m;
    name_t name4_m;
};

/******************************************************************************/

template <typename F>
struct function_pack_helper<F, 5>
{
    typedef function_traits<F>                        traits_type;
    typedef typename traits_type::result_type         traits_result_type;
    typedef typename novoid<traits_result_type>::type result_type;

    explicit function_pack_helper(const F& f,
                                  name_t   arg1_name = name_t(), name_t arg2_name = name_t(),
                                  name_t   arg3_name = name_t(), name_t arg4_name = name_t(),
                                  name_t   arg5_name = name_t()) :
        f_m(f),
        name1_m(arg1_name), name2_m(arg2_name), name3_m(arg3_name), name4_m(arg4_name),
        name5_m(arg5_name)
    { }

    result_type operator()(const array_t& argument_set) const
    {
        return invoke_novoid(boost::bind(f_m,
                                         ADOBE_FUNCTION_UNNAMED_ARG(1), ADOBE_FUNCTION_UNNAMED_ARG(2),
                                         ADOBE_FUNCTION_UNNAMED_ARG(3), ADOBE_FUNCTION_UNNAMED_ARG(4),
                                         ADOBE_FUNCTION_UNNAMED_ARG(5)));
    }

    result_type operator()(const dictionary_t& named_argument_set) const
    {
        return invoke_novoid(boost::bind(f_m,
                                         ADOBE_FUNCTION_NAMED_ARG(1), ADOBE_FUNCTION_NAMED_ARG(2),
                                         ADOBE_FUNCTION_NAMED_ARG(3), ADOBE_FUNCTION_NAMED_ARG(4),
                                         ADOBE_FUNCTION_NAMED_ARG(5)));
    }

private:
    typename traits_type::function_type f_m;
    name_t name1_m;
    name_t name2_m;
    name_t name3_m;
    name_t name4_m;
    name_t name5_m;
};

/******************************************************************************/

template <typename F>
struct function_pack_helper<F, 6>
{
    typedef function_traits<F>                        traits_type;
    typedef typename traits_type::result_type         traits_result_type;
    typedef typename novoid<traits_result_type>::type result_type;

    explicit function_pack_helper(const F& f,
                                  name_t   arg1_name = name_t(), name_t arg2_name = name_t(),
                                  name_t   arg3_name = name_t(), name_t arg4_name = name_t(),
                                  name_t   arg5_name = name_t(), name_t arg6_name = name_t()) :
        f_m(f),
        name1_m(arg1_name), name2_m(arg2_name), name3_m(arg3_name), name4_m(arg4_name),
        name5_m(arg5_name), name6_m(arg6_name)
    { }

    result_type operator()(const array_t& argument_set) const
    {
        return invoke_novoid(boost::bind(f_m,
                                         ADOBE_FUNCTION_UNNAMED_ARG(1), ADOBE_FUNCTION_UNNAMED_ARG(2),
                                         ADOBE_FUNCTION_UNNAMED_ARG(3), ADOBE_FUNCTION_UNNAMED_ARG(4),
                                         ADOBE_FUNCTION_UNNAMED_ARG(5), ADOBE_FUNCTION_UNNAMED_ARG(6)));
    }

    result_type operator()(const dictionary_t& named_argument_set) const
    {
        return invoke_novoid(boost::bind(f_m,
                                         ADOBE_FUNCTION_NAMED_ARG(1), ADOBE_FUNCTION_NAMED_ARG(2),
                                         ADOBE_FUNCTION_NAMED_ARG(3), ADOBE_FUNCTION_NAMED_ARG(4),
                                         ADOBE_FUNCTION_NAMED_ARG(5), ADOBE_FUNCTION_NAMED_ARG(6)));
    }

private:
    typename traits_type::function_type f_m;
    name_t name1_m;
    name_t name2_m;
    name_t name3_m;
    name_t name4_m;
    name_t name5_m;
    name_t name6_m;
};

/******************************************************************************/

template <typename F>
struct function_pack_helper<F, 7>
{
    typedef function_traits<F>                        traits_type;
    typedef typename traits_type::result_type         traits_result_type;
    typedef typename novoid<traits_result_type>::type result_type;

    explicit function_pack_helper(const F& f,
                                  name_t   arg1_name = name_t(), name_t arg2_name = name_t(),
                                  name_t   arg3_name = name_t(), name_t arg4_name = name_t(),
                                  name_t   arg5_name = name_t(), name_t arg6_name = name_t(),
                                  name_t   arg7_name = name_t()) :
        f_m(f),
        name1_m(arg1_name), name2_m(arg2_name), name3_m(arg3_name), name4_m(arg4_name),
        name5_m(arg5_name), name6_m(arg6_name), name7_m(arg7_name)
    { }

    result_type operator()(const array_t& argument_set) const
    {
        return invoke_novoid(boost::bind(f_m,
                                         ADOBE_FUNCTION_UNNAMED_ARG(1), ADOBE_FUNCTION_UNNAMED_ARG(2),
                                         ADOBE_FUNCTION_UNNAMED_ARG(3), ADOBE_FUNCTION_UNNAMED_ARG(4),
                                         ADOBE_FUNCTION_UNNAMED_ARG(5), ADOBE_FUNCTION_UNNAMED_ARG(6),
                                         ADOBE_FUNCTION_UNNAMED_ARG(7)));
    }

    result_type operator()(const dictionary_t& named_argument_set) const
    {
        return invoke_novoid(boost::bind(f_m,
                                         ADOBE_FUNCTION_NAMED_ARG(1), ADOBE_FUNCTION_NAMED_ARG(2),
                                         ADOBE_FUNCTION_NAMED_ARG(3), ADOBE_FUNCTION_NAMED_ARG(4),
                                         ADOBE_FUNCTION_NAMED_ARG(5), ADOBE_FUNCTION_NAMED_ARG(6),
                                         ADOBE_FUNCTION_NAMED_ARG(7)));
    }

private:
    typename traits_type::function_type f_m;
    name_t name1_m;
    name_t name2_m;
    name_t name3_m;
    name_t name4_m;
    name_t name5_m;
    name_t name6_m;
    name_t name7_m;
};

/******************************************************************************/

#undef ADOBE_FUNCTION_UNNAMED_ARG
#undef ADOBE_FUNCTION_NAMED_ARG

/******************************************************************************/

template <typename T>
any_regular_t wrap_regular(const T& x)
{ return any_regular_t(x); }

template <>
inline any_regular_t wrap_regular<any_regular_t>(const any_regular_t& x)
{ return x; }

/******************************************************************************/

} // namespace implementation
// ADOBE_NO_DOCUMENTATION
#endif
/******************************************************************************/

template <typename F>
inline implementation::function_pack_helper<F, 0>
named_bind(const F& f)
{
    BOOST_STATIC_ASSERT((function_traits<F>::arity == 0));

    return implementation::function_pack_helper<F, 0>(f);
}

template <typename F>
inline implementation::function_pack_helper<F, 1>
named_bind(const F& f,
           name_t arg1_name)
{
    BOOST_STATIC_ASSERT((function_traits<F>::arity == 1));

    return implementation::function_pack_helper<F, 1>(f,
                                                      arg1_name);
}

template <typename F>
inline implementation::function_pack_helper<F, 2>
named_bind(const F& f,
           name_t arg1_name, name_t arg2_name)
{
    BOOST_STATIC_ASSERT((function_traits<F>::arity == 2));

    return implementation::function_pack_helper<F, 2>(f,
                                                      arg1_name,
                                                      arg2_name);
}

template <typename F>
inline implementation::function_pack_helper<F, 3>
named_bind(const F& f,
           name_t arg1_name, name_t arg2_name, name_t arg3_name)
{
    BOOST_STATIC_ASSERT((function_traits<F>::arity == 3));

    return implementation::function_pack_helper<F, 3>(f,
                                                      arg1_name,
                                                      arg2_name,
                                                      arg3_name);
}

template <typename F>
inline implementation::function_pack_helper<F, 4>
named_bind(const F& f,
           name_t arg1_name, name_t arg2_name, name_t arg3_name, name_t arg4_name)
{
    BOOST_STATIC_ASSERT((function_traits<F>::arity == 4));

    return implementation::function_pack_helper<F, 4>(f,
                                                      arg1_name,
                                                      arg2_name,
                                                      arg3_name,
                                                      arg4_name);
}

template <typename F>
inline implementation::function_pack_helper<F, 5>
named_bind(const F& f,
           name_t arg1_name, name_t arg2_name, name_t arg3_name, name_t arg4_name,
           name_t arg5_name)
{
    BOOST_STATIC_ASSERT((function_traits<F>::arity == 5));

    return implementation::function_pack_helper<F, 5>(f,
                                                      arg1_name,
                                                      arg2_name,
                                                      arg3_name,
                                                      arg4_name,
                                                      arg5_name);
}

template <typename F>
inline implementation::function_pack_helper<F, 6>
named_bind(const F& f,
           name_t arg1_name, name_t arg2_name, name_t arg3_name, name_t arg4_name,
           name_t arg5_name, name_t arg6_name)
{
    BOOST_STATIC_ASSERT((function_traits<F>::arity == 6));

    return implementation::function_pack_helper<F, 6>(f,
                                                      arg1_name,
                                                      arg2_name,
                                                      arg3_name,
                                                      arg4_name,
                                                      arg5_name,
                                                      arg6_name);
}

template <typename F>
inline implementation::function_pack_helper<F, 7>
named_bind(const F& f,
           name_t arg1_name, name_t arg2_name, name_t arg3_name, name_t arg4_name,
           name_t arg5_name, name_t arg6_name, name_t arg7_name)
{
    BOOST_STATIC_ASSERT((function_traits<F>::arity == 7));

    return implementation::function_pack_helper<F, 7>(f,
                                                      arg1_name,
                                                      arg2_name,
                                                      arg3_name,
                                                      arg4_name,
                                                      arg5_name,
                                                      arg6_name,
                                                      arg7_name);
}

/******************************************************************************/
/*!
    \brief Container class to unify a collecton of functions under the same function signature.
    \ingroup apl_libraries

    This class will collect registered functions by one of two methods: as functions with named or
    unnamed arguments. If the former, the function is then invoked with an array_t containing
    arguments in the order in which they should be passed to the original function. If the latter,
    the function is invoked with an dictionary_t containing arguments bound to keys that match
    those required by the function.

    This collection is also useful when tying into the array- and dictionary-based function lookup
    routines for the virtual_machine_t class. There is a related routine used to attach this
    class to the latter.

    \note
    The array_t and dictionary_t are passed as const into the lookup routine. As such, non-const
    parameters will not work.

    \note
    As array_t and dictionary_t are containers of types that model Regular, the parameters you
    pass must model the Regular concept. This differs from the argument types of the functions
    you register into the pack: they need not be regular (e.g. const references are OK).

    \note
    The unnamed and named function collections are kept separately from one another. Thus while
    it may be more confusing suring usage, it is possible to reference to different functions
    (one unnamed, the other named) with the same name_t identifier.

    \note
    Don't forget that a member function has a minimum arity of 1 (the 'this' of the member function
    routine).
*/
struct function_pack_t
{
#ifndef ADOBE_NO_DOCUMENTATION
private:
    typedef boost::function<any_regular_t (const dictionary_t&)> dictionary_function_t;
    typedef boost::function<any_regular_t (const array_t&)>      array_function_t;
#endif

public:
    /*!
        This routine is used to register unnamed functions of any arity to the function pack.
    */
    template <typename F>
    void register_function(name_t name, const F& f)
    {
        // We can't check arity here because unnamed argument functions
        // pass through this, which could be of any arity.

        attach_unnamed(name,
                       implementation::function_pack_helper<F, function_traits<F>::arity>(f));
    }

    /*!
        This routine is specifically used to register named functions of arity 0 to the function pack.
    */
    template <typename F>
    void register_named0_function(name_t name, const F& f)
    {
        attach_named(name, named_bind(f));
    }

    /*!
        This routine is used to register named functions of arity 1 to the function pack.

        \note
        The arity of the original function is asserted to be 1 at compile time.
    */
    template <typename F>
    void register_function(name_t name, const F& f, name_t arg1_name)
    {
        attach_named(name,
                     named_bind(f, arg1_name));
    }

    /*!
        This routine is used to register named functions of arity 2 to the function pack.

        \note
        The arity of the original function is asserted to be 2 at compile time.
    */
    template <typename F>
    void register_function(name_t name, const F& f, name_t arg1_name, name_t arg2_name)
    {
        attach_named(name, named_bind(f,
                                      arg1_name, arg2_name));
    }

    /*!
        This routine is used to register named functions of arity 3 to the function pack.

        \note
        The arity of the original function is asserted to be 3 at compile time.
    */
    template <typename F>
    void register_function(name_t name, const F& f, name_t arg1_name, name_t arg2_name, name_t arg3_name)
    {
        attach_named(name, named_bind(f,
                                      arg1_name, arg2_name, arg3_name));
    }

    /*!
        This routine is used to register named functions of arity 4 to the function pack.

        \note
        The arity of the original function is asserted to be 4 at compile time.
    */
    template <typename F>
    void register_function(name_t name, const F& f,
                           name_t arg1_name, name_t arg2_name, name_t arg3_name, name_t arg4_name)
    {
        attach_named(name, named_bind(f,
                                      arg1_name, arg2_name, arg3_name, arg4_name));
    }

    /*!
        This routine is used to register named functions of arity 5 to the function pack.

        \note
        The arity of the original function is asserted to be 5 at compile time.
    */
    template <typename F>
    void register_function(name_t name, const F& f,
                           name_t arg1_name, name_t arg2_name, name_t arg3_name, name_t arg4_name,
                           name_t arg5_name)
    {
        BOOST_STATIC_ASSERT((function_traits<F>::arity == 5));

        attach_named(name, named_bind(f,
                                      arg1_name, arg2_name, arg3_name, arg4_name,
                                      arg5_name));
    }

    /*!
        This routine is used to register named functions of arity 6 to the function pack.

        \note
        The arity of the original function is asserted to be 6 at compile time.
    */
    template <typename F>
    void register_function(name_t name, const F& f,
                           name_t arg1_name, name_t arg2_name, name_t arg3_name, name_t arg4_name,
                           name_t arg5_name, name_t arg6_name)
    {
        attach_named(name, named_bind(f,
                                      arg1_name, arg2_name, arg3_name, arg4_name,
                                      arg5_name, arg6_name));
    }

    /*!
        This routine is used to register named functions of arity 7 to the function pack.

        \note
        The arity of the original function is asserted to be 7 at compile time.
    */
    template <typename F>
    void register_function(name_t name, const F& f,
                           name_t arg1_name, name_t arg2_name, name_t arg3_name, name_t arg4_name,
                           name_t arg5_name, name_t arg6_name, name_t arg7_name)
    {
        attach_named(name, named_bind(f,
                                      arg1_name, arg2_name, arg3_name, arg4_name,
                                      arg5_name, arg6_name, arg7_name));
    }

    /*!
        Routine for invoking one of the registered functions with a const unnamed argument set.
    */
    any_regular_t operator()(name_t function, const array_t& argument_set) const
    {
        return find_unnamed(function)->second(argument_set);
    }

    /*!
        Routine for invoking one of the registered functions with a const named argument set.
    */
    any_regular_t operator()(name_t function, const dictionary_t& named_argument_set) const
    {
        return find_named(function)->second(named_argument_set);
    }

    /*!
        Routine for invoking one of the registered functions with a nonconst unnamed argument set.
    */
    any_regular_t operator()(name_t function, array_t& argument_set) const
    {
        return find_unnamed(function)->second(argument_set);
    }

    /*!
        Routine for invoking one of the registered functions with a nonconst named argument set.
    */
    any_regular_t operator()(name_t function, dictionary_t& named_argument_set) const
    {
        return find_named(function)->second(named_argument_set);
    }

    /*!
        Routine for invoking one of the registered functions with a const unnamed argument set.
        The name of the function to invoke is extracted out of the array at the 0th index
    */
    any_regular_t operator()(const array_t& argument_set) const
    {
        adobe::name_t function;

        if (!argument_set.empty())
            argument_set[0].cast<adobe::name_t>(function);

        return (*this)(function, argument_set);
    }

    /*!
        Routine for invoking one of the registered functions with a const named argument set.
        The name of the function to invoke is extracted out of the dictionary under the <code>command</code> named argument
    */
    any_regular_t operator()(const dictionary_t& named_argument_set) const
    {
        adobe::name_t function;

        get_value(named_argument_set, adobe::static_name_t("command"), function);

        return (*this)(function, named_argument_set);
    }

    /*!
        Routine for invoking one of the registered functions with a nonconst unnamed argument set.
        The name of the function to invoke is extracted out of the array at the 0th index
    */
    any_regular_t operator()(array_t& argument_set) const
    {
        adobe::name_t function;

        if (!argument_set.empty())
            argument_set[0].cast<adobe::name_t>(function);

        return (*this)(function, argument_set);
    }

    /*!
        Routine for invoking one of the registered functions with a nonconst named argument set.
        The name of the function to invoke is extracted out of the dictionary under the <code>command</code> named argument
    */
    any_regular_t operator()(dictionary_t& named_argument_set) const
    {
        adobe::name_t function;

        get_value(named_argument_set, adobe::static_name_t("command"), function);

        return (*this)(function, named_argument_set);
    }

    void register_unnamed(name_t name, const array_function_t& f)
    {
        array_function_map_m.insert(array_function_map_t::value_type(name, f));
    }

    void register_named(name_t name, const dictionary_function_t& f)
    {
        dictionary_function_map_m.insert(dictionary_function_map_t::value_type(name, f));
    }

#ifndef ADOBE_NO_DOCUMENTATION
private:
    typedef closed_hash_map<name_t, dictionary_function_t>       dictionary_function_map_t;
    typedef closed_hash_map<name_t, array_function_t>            array_function_map_t;

    dictionary_function_map_t::const_iterator find_named(name_t function) const
    {
        if (function == name_t())
            throw std::runtime_error("find_named: no function name specified");

        dictionary_function_map_t::const_iterator found(dictionary_function_map_m.find(function));

        if (found == dictionary_function_map_m.end())
            throw std::runtime_error(make_string("find_named: function '",
                                                 function.c_str(),
                                                 "' not found"));

        return found;
    }

    array_function_map_t::const_iterator find_unnamed(name_t function) const
    {
        if (function == name_t())
            throw std::runtime_error("find_named: no function name specified");

        array_function_map_t::const_iterator found(array_function_map_m.find(function));

        if (found == array_function_map_m.end())
            throw std::runtime_error(make_string("find_unnamed: function '",
                                                 function.c_str(),
                                                 "' not found"));

        return found;
    }

    template <typename T>
    void attach_unnamed(name_t name, T helper)
    {
        typename T::result_type (T::*proc)(const array_t&) const = &T::operator();

        array_function_map_m.insert(
            array_function_map_t::value_type(name,
                boost::bind(&implementation::wrap_regular<typename T::result_type>,
                            boost::bind(proc, helper, _1))));
    }

    template <typename T>
    void attach_named(name_t name, T helper)
    {
        typename T::result_type (T::*proc)(const dictionary_t&) const = &T::operator();

        dictionary_function_map_m.insert(
            dictionary_function_map_t::value_type(name,
                boost::bind(&implementation::wrap_regular<typename T::result_type>,
                            boost::bind(proc, helper, _1))));
    }

    dictionary_function_map_t dictionary_function_map_m;
    array_function_map_t      array_function_map_m;
// ADOBE_NO_DOCUMENTATION
#endif
};

/******************************************************************************/
/*!
    \ingroup function_pack
    \relates function_pack_t

    Binds a function pack to an virtual_machine_t as the array- and dictionary-based function
    lookup mechanism.

    \note
    The function pack must have a lifetime that is at least equal to that of the VM
*/
inline void attach(virtual_machine_t& vm, const function_pack_t& pack)
{
    vm.set_array_function_lookup(boost::cref(pack));
    vm.set_dictionary_function_lookup(boost::cref(pack));
}

/******************************************************************************/

} // namespace adobe

/******************************************************************************/

#endif

/******************************************************************************/
