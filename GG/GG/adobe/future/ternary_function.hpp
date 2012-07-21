/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#ifndef ADOBE_TERNARY_FUNCTION_HPP
#define ADOBE_TERNARY_FUNCTION_HPP

#include <GG/adobe/config.hpp>

/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/

template <typename T, typename U, typename V, typename R>
struct ternary_function
{
    typedef T   first_argument_type;
    typedef U   second_argument_type;
    typedef V   third_argument_type;
    typedef R   result_type;
};

/*************************************************************************************************/

template <typename T, typename U, typename V, typename R>
class pointer_to_ternary_function : public ternary_function<T, U, V, R>
{
public:
    typedef ternary_function<T, U, V, R>            _super;
    typedef typename _super::first_argument_type    first_argument_type;
    typedef typename _super::second_argument_type   second_argument_type;
    typedef typename _super::third_argument_type    third_argument_type;
    typedef typename _super::result_type            result_type;

    explicit pointer_to_ternary_function(result_type (*f)(first_argument_type, second_argument_type, third_argument_type)) :
        f_m(f)
        { }

    result_type operator()(first_argument_type x, second_argument_type y, third_argument_type z) const
    {
        return f_m(x, y, z);
    }

private:
    result_type (*f_m)(first_argument_type, second_argument_type, third_argument_type);
};

/*************************************************************************************************/

template <typename T, typename U, typename V, typename R>
inline pointer_to_ternary_function<T, U, V, R> ptr_fun(R (*f)(T, U, V))
{
    return pointer_to_ternary_function<T, U, V, R>(f);
}

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/

#endif

/*************************************************************************************************/
