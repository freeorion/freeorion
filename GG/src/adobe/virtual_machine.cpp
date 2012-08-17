/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#include <boost/config.hpp>

#include <cmath>
#include <numeric>
#include <typeinfo>
#include <vector>

#include <boost/iterator/transform_iterator.hpp>

#include <GG/adobe/algorithm/minmax.hpp>
#include <GG/adobe/any_regular.hpp>
#include <GG/adobe/array.hpp>
#include <GG/adobe/cmath.hpp>
#include <GG/adobe/dictionary.hpp>
#include <GG/adobe/empty.hpp>
#include <GG/adobe/functional.hpp>
#include <GG/adobe/implementation/expression_filter.hpp>
#include <GG/adobe/implementation/token.hpp>
#include <GG/adobe/localization.hpp>
#include <GG/adobe/name.hpp>
#include <GG/adobe/once.hpp>
#include <GG/adobe/static_table.hpp>
#include <GG/adobe/string.hpp>
#include <GG/adobe/vector.hpp>
#include <GG/adobe/virtual_machine.hpp>

#ifndef NDEBUG
#include <iostream>
#endif

/*************************************************************************************************/

ADOBE_ONCE_DECLARATION(adobe_virtual_machine)

ADOBE_ONCE_DECLARATION(adobe_virtual_machine_get_type_name)

/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/

template<typename ValueType>
struct static_table_traits<adobe::type_info_t, ValueType>
{
    typedef bool                            result_type;
    typedef adobe::type_info_t              key_type;
    typedef ValueType                       value_type;
    typedef std::pair<key_type, value_type> entry_type;

    result_type operator()(const entry_type& x, const entry_type& y) const
    {
        return (*this)(x, y.first);
    }

    // revisit: MM. For debugging purposes, VC 8 requires the definition of
    // this (unnecessary overload) in debug versions.
    result_type operator()(const key_type& x, const entry_type& y) const 
    {
        return !(operator()(y, x)) && !equal(x, y.first);
    }

    result_type operator()(const entry_type& x, const key_type& y) const
    {
        return x.first.before(y) != 0;
    }

    result_type equal(const key_type& x, const key_type& y) const
    {
        return x == y;
    }
};

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/

namespace {

/*************************************************************************************************/

typedef void (adobe::virtual_machine_t::implementation_t::* operator_t)();
typedef boost::function<adobe::any_regular_t (const adobe::array_t&)>         array_function_t;
typedef boost::function<adobe::any_regular_t (const adobe::dictionary_t&)>    dictionary_function_t;

typedef adobe::vector<adobe::any_regular_t>                    stack_type; // REVISIT (sparent) : GCC 3.1 the symbol stack_t conflicts with a symbol in signal.h

#if !defined(ADOBE_NO_DOCUMENTATION)
typedef adobe::static_table<adobe::name_t, operator_t, 22>              operator_table_t;
typedef adobe::static_table<adobe::name_t, array_function_t, 7>         array_function_table_t;
typedef adobe::static_table<adobe::name_t, dictionary_function_t, 1>    dictionary_function_table_t;
typedef adobe::static_table<adobe::type_info_t, adobe::name_t, 7>       type_table_t;
#endif // !defined(ADOBE_NO_DOCUMENTATION)

/*************************************************************************************************/

template <typename Result>
struct make
{
    typedef Result result_type;

    template <typename T>
    Result operator () (const T& x) { return Result(x); }
};

/*************************************************************************************************/

static type_table_t* type_table_g;

/*************************************************************************************************/

void get_type_name_init()
{
    static type_table_t type_table_s =
    {{
        type_table_t::entry_type(adobe::type_info<double>(),               adobe::static_name_t("number")),
        type_table_t::entry_type(adobe::type_info<bool>(),                 adobe::static_name_t("boolean")),
        type_table_t::entry_type(adobe::type_info<adobe::empty_t>(),       adobe::static_name_t("empty")),
        type_table_t::entry_type(adobe::type_info<adobe::string_t>(),      adobe::static_name_t("string")),
        type_table_t::entry_type(adobe::type_info<adobe::array_t>(),       adobe::static_name_t("array")),
        type_table_t::entry_type(adobe::type_info<adobe::dictionary_t>(),  adobe::static_name_t("dictionary")),
        type_table_t::entry_type(adobe::type_info<adobe::name_t>(),        adobe::static_name_t("name"))
    }};

    type_table_s.sort();

    type_table_g = &type_table_s;
}

/*************************************************************************************************/

adobe::name_t get_type_name(const adobe::any_regular_t& val)
{
    ADOBE_ONCE_INSTANCE(adobe_virtual_machine_get_type_name);

    adobe::name_t result;

    (*type_table_g)(val.type_info(), result);

    if (!result) result = adobe::static_name_t("unknown");

    return result;
}

/*************************************************************************************************/

#if 0
#pragma mark -
#endif

/*************************************************************************************************/

adobe::any_regular_t xml_escape_function(const adobe::array_t& parameters)
{
    if (parameters.size() != 1 ||
        parameters[0].type_info() != adobe::type_info<adobe::string_t>())
        throw std::runtime_error("xml_escape: parameter error");

    return adobe::any_regular_t(adobe::entity_escape(parameters[0].cast<adobe::string_t>()));
}

/*************************************************************************************************/

adobe::any_regular_t xml_unescape_function(const adobe::array_t& parameters)
{
    if (parameters.size() != 1 ||
        parameters[0].type_info() != adobe::type_info<adobe::string_t>())
        throw std::runtime_error("xml_unescape: parameter error");

    return adobe::any_regular_t(adobe::entity_unescape(parameters[0].cast<adobe::string_t>()));
}

/*************************************************************************************************/

adobe::any_regular_t localize_function(const adobe::array_t& parameters)
{
    if (parameters.size() != 1)
        throw std::runtime_error("localize: parameter error");

    return adobe::any_regular_t(adobe::localization_ready() ?
                              adobe::localization_invoke(parameters.front().cast<std::string>()) :
                              parameters.front().cast<std::string>());
}

/*************************************************************************************************/

adobe::any_regular_t round_function(const adobe::array_t& parameters)
{
    if (parameters.size() == 0)
        throw std::runtime_error("round: parameter error");

    return adobe::any_regular_t(adobe::round(parameters.front().cast<double>()));
}

/*************************************************************************************************/

adobe::any_regular_t min_function(const adobe::array_t& parameters)
{
    if (parameters.size() == 0)
        throw std::runtime_error("min: parameter error");

    return *adobe::min_element(parameters, boost::bind(std::less<double>(),
        boost::bind(adobe::any_regular_t::transform<double>(), _1),
        boost::bind(adobe::any_regular_t::transform<double>(), _2)));
}

/*************************************************************************************************/

adobe::any_regular_t max_function(const adobe::array_t& parameters)
{
    if (parameters.size() == 0)
        throw std::runtime_error("max: parameter error");

    return *adobe::max_element(parameters, boost::bind(std::less<double>(),
        boost::bind(adobe::any_regular_t::transform<double>(), _1),
        boost::bind(adobe::any_regular_t::transform<double>(), _2)));
}

/*************************************************************************************************/

adobe::any_regular_t typeof_function(const adobe::array_t& parameters)
{
    if (parameters.size() == 0)
        throw std::runtime_error("typeof: parameter error");

    return adobe::any_regular_t(get_type_name(parameters.front()));
}

/*************************************************************************************************/

#if 0
#pragma mark -
#endif

/*************************************************************************************************/

adobe::any_regular_t scale_function(const adobe::dictionary_t& parameters)
{
    double  m(1.0);
    double  x(0.0);
    double  b(0.0);

    get_value(parameters, adobe::static_name_t("m"), m);
    get_value(parameters, adobe::static_name_t("x"), x);
    get_value(parameters, adobe::static_name_t("b"), b);

    return adobe::any_regular_t(m * x + b);
}

/*************************************************************************************************/

#if 0
#pragma mark -
#endif

/*************************************************************************************************/

void throw_function_not_defined(adobe::name_t function_name)
{
    throw std::logic_error(adobe::make_string("Function \'", function_name.c_str(), "\' not defined."));
}

/*************************************************************************************************/

} // namespace

/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/

class virtual_machine_t::implementation_t
{
 public:
    implementation_t();

    void evaluate(const array_t& expression);
    
    const any_regular_t& back() const;
    any_regular_t& back();
    void pop_back();

    variable_lookup_t               variable_lookup_m;
    array_function_lookup_t         array_function_lookup_m;
    dictionary_function_lookup_t    dictionary_function_lookup_m;

 private:
    stack_type              value_stack_m;
    
    static operator_t find_operator(adobe::name_t oper);

 public:
    // logical and operator function functions

    template <template<class T> class Operator, class OperandType>
    void unary_operator();

    template <template<class T> class Operator, class OperandType>
    void binary_operator();

    void logical_operator(bool do_and);
    void logical_and_operator();
    void logical_or_operator();
    void index_operator();
    void ifelse_operator();
    void variable_operator();
    void function_operator();
    void array_operator();
    void dictionary_operator();

    static operator_table_t*            operator_table_g;
    static array_function_table_t*      array_function_table_g;
    static dictionary_function_table_t* dictionary_function_table_g;
};

/*************************************************************************************************/

operator_table_t*               virtual_machine_t::implementation_t::operator_table_g;
array_function_table_t*         virtual_machine_t::implementation_t::array_function_table_g;
dictionary_function_table_t*    virtual_machine_t::implementation_t::dictionary_function_table_g;

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/

#if 0
#pragma mark -
#endif

/*************************************************************************************************/

namespace {

/*************************************************************************************************/

void virtual_machine_init_once()
{
    typedef operator_table_t::entry_type op_entry_type;
    typedef adobe::virtual_machine_t::implementation_t implementation_t;

    static operator_table_t operator_table_s =
    {{
        op_entry_type(adobe::not_k,             &implementation_t::unary_operator<std::logical_not, bool>),
        op_entry_type(adobe::unary_negate_k,    &implementation_t::unary_operator<std::negate, double>),
        op_entry_type(adobe::add_k,             &implementation_t::binary_operator<std::plus, double>),
        op_entry_type(adobe::subtract_k,        &implementation_t::binary_operator<std::minus, double>),
        op_entry_type(adobe::multiply_k,        &implementation_t::binary_operator<std::multiplies, double>),
        op_entry_type(adobe::modulus_k,         &implementation_t::binary_operator<std::modulus, long>),
        op_entry_type(adobe::divide_k,          &implementation_t::binary_operator<std::divides, double>),
        op_entry_type(adobe::less_k,            &implementation_t::binary_operator<std::less, double>),
        op_entry_type(adobe::greater_k,         &implementation_t::binary_operator<std::greater, double>),
        op_entry_type(adobe::less_equal_k,      &implementation_t::binary_operator<std::less_equal, double>),
        op_entry_type(adobe::greater_equal_k,   &implementation_t::binary_operator<std::greater_equal, double>),
        op_entry_type(adobe::equal_k,           &implementation_t::binary_operator<std::equal_to, adobe::any_regular_t>),
        op_entry_type(adobe::not_equal_k,       &implementation_t::binary_operator<std::not_equal_to, adobe::any_regular_t>),
        op_entry_type(adobe::ifelse_k,          &implementation_t::ifelse_operator),
        op_entry_type(adobe::dot_index_k,       &implementation_t::index_operator),
        op_entry_type(adobe::bracket_index_k,   &implementation_t::index_operator),
        op_entry_type(adobe::function_k,        &implementation_t::function_operator),
        op_entry_type(adobe::array_k,           &implementation_t::array_operator),
        op_entry_type(adobe::dictionary_k,      &implementation_t::dictionary_operator),
        op_entry_type(adobe::variable_k,        &implementation_t::variable_operator),
        op_entry_type(adobe::and_k,             &implementation_t::logical_and_operator),
        op_entry_type(adobe::or_k,              &implementation_t::logical_or_operator)
    }};

    static array_function_table_t array_function_table_s =
    {{
        array_function_table_t::entry_type(adobe::static_name_t("typeof"),       &typeof_function),
        array_function_table_t::entry_type(adobe::static_name_t("min"),          &min_function),
        array_function_table_t::entry_type(adobe::static_name_t("max"),          &max_function),
        array_function_table_t::entry_type(adobe::static_name_t("round"),        &round_function),
        array_function_table_t::entry_type(adobe::static_name_t("localize"),     &localize_function),
        array_function_table_t::entry_type(adobe::static_name_t("xml_escape"),   &xml_escape_function),
        array_function_table_t::entry_type(adobe::static_name_t("xml_unescape"), &xml_unescape_function)
    }};

    static dictionary_function_table_t dictionary_function_table_s =
    {{
        dictionary_function_table_t::entry_type(adobe::static_name_t("scale"),  &scale_function)
    }};

    operator_table_s.sort();
    array_function_table_s.sort();
    dictionary_function_table_s.sort();

    adobe::virtual_machine_t::implementation_t::operator_table_g            = &operator_table_s;
    adobe::virtual_machine_t::implementation_t::array_function_table_g      = &array_function_table_s;
    adobe::virtual_machine_t::implementation_t::dictionary_function_table_g = &dictionary_function_table_s;
}

/*************************************************************************************************/

} // namespace

/*************************************************************************************************/

ADOBE_ONCE_DEFINITION(adobe_virtual_machine, virtual_machine_init_once)

ADOBE_ONCE_DEFINITION(adobe_virtual_machine_get_type_name, get_type_name_init)

/*************************************************************************************************/

#if 0
#pragma mark -
#endif

/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/
    
virtual_machine_t::implementation_t::implementation_t()
{
    ADOBE_ONCE_INSTANCE(adobe_virtual_machine);
}

/*************************************************************************************************/
    
void virtual_machine_t::implementation_t::evaluate(const array_t& expression)
{
    for(expression_t::const_iterator iter(expression.begin()); iter != expression.end(); ++iter)
    {
        if (iter->type_info() == type_info<adobe::name_t>() && iter->cast<adobe::name_t>().c_str()[0] == '.')
        {
            if (iter->cast<adobe::name_t>() != parenthesized_expression_k &&
                iter->cast<adobe::name_t>() != name_k)
            {
                ((*this).*(find_operator(iter->cast<adobe::name_t>())))();
            }
        }
        else
        {
            value_stack_m.push_back(*iter);
        }
    }
}
    
/*************************************************************************************************/
    
const any_regular_t& virtual_machine_t::implementation_t::back() const
{
    return value_stack_m.back();
}
    
/*************************************************************************************************/
    
any_regular_t& virtual_machine_t::implementation_t::back()
{
    return value_stack_m.back();
}
    
/*************************************************************************************************/
    
void virtual_machine_t::implementation_t::pop_back()
{
    value_stack_m.pop_back();
}

/*************************************************************************************************/

operator_t virtual_machine_t::implementation_t::find_operator(adobe::name_t oper)
{
    ADOBE_ONCE_INSTANCE(adobe_virtual_machine);

    return (*operator_table_g)(oper);
}

/*************************************************************************************************/

#if 0
#pragma mark -
#endif

/*************************************************************************************************/

template <template<class T> class Operator, class OperandType>
void virtual_machine_t::implementation_t::binary_operator()
{
    typedef OperandType                     operand_t;
    typedef Operator<operand_t>             operator_class;
    
    stack_type::iterator iter(value_stack_m.end()); // REVISIT (sparent) : GCC 3.1 requires :: qualifier
    
    adobe::any_regular_t& operand1 = *(iter - 2);
    adobe::any_regular_t& operand2 = *(iter - 1);
    
    operand1.assign(operator_class()(operand1.template cast<operand_t>(), operand2.template cast<operand_t>()));
        
    pop_back();
}

/*************************************************************************************************/

template <template<class T> class Operator, class OperandType>
void virtual_machine_t::implementation_t::unary_operator()
{
    typedef OperandType         operand_t;
    typedef Operator<operand_t> operator_class;

    stack_type::iterator iter(value_stack_m.end());
    
    adobe::any_regular_t& operand1 = *(iter - 1);
    
    operand1.assign(operator_class()(operand1.template cast<operand_t>()));
}

/*************************************************************************************************/

void virtual_machine_t::implementation_t::logical_operator(bool do_and)
{
    adobe::array_t operand_exp (back().cast<adobe::array_t>());
    pop_back();
    
    any_regular_t operand1 = back();

    if (operand1.cast<bool>() == do_and)
    {
        pop_back();
        evaluate(operand_exp);
        
        any_regular_t& operand2(value_stack_m.back());

        if (operand2.type_info() != type_info<bool>()) throw std::bad_cast();
    }
} 

/*************************************************************************************************/

void virtual_machine_t::implementation_t::logical_and_operator()
{
    logical_operator(true);
}

/*************************************************************************************************/

void virtual_machine_t::implementation_t::logical_or_operator()
{
    logical_operator(false);
}

/*************************************************************************************************/

void virtual_machine_t::implementation_t::index_operator()
{
    stack_type::iterator iter(value_stack_m.end());
    
    any_regular_t& operand1(*(iter - 2));
    any_regular_t& operand2(*(iter - 1));
    
    adobe::any_regular_t result;
    
    if (operand2.type_info() == type_info<adobe::name_t>())
    {
        result = get_value(operand1.cast<adobe::dictionary_t>(), operand2.cast<adobe::name_t>());
    }
    else
    {
        const array_t&  array = operand1.cast<array_t>();
                std::size_t     index = operand2.cast<std::size_t>();
        
        if (!(index < array.size())) throw std::runtime_error("index: array index out of range");
        
        result = array[index];
    }
    
    operand1 = result;
    
    pop_back();
}

/*************************************************************************************************/

void virtual_machine_t::implementation_t::ifelse_operator()
{
    adobe::array_t else_exp (back().cast<adobe::array_t>());
    pop_back();
    adobe::array_t then_exp (back().cast<adobe::array_t>());
    pop_back();
    
    bool            predicate(back().cast<bool>());
    pop_back();
            
    evaluate(predicate ? then_exp : else_exp);
}

/*************************************************************************************************/

void virtual_machine_t::implementation_t::variable_operator()
{
    adobe::name_t variable(back().cast<adobe::name_t>());

    pop_back();
    
    if (!variable_lookup_m)
        throw std::logic_error("No variable lookup installed.");

    value_stack_m.push_back(variable_lookup_m(variable));
}

/*************************************************************************************************/

#if 0

// This is good code - it was just overkill for this problem - we ended up generated a fair
// amount of code for this.

template <typename Iterator, typename UnaryFunction>
typename std::pair<boost::transform_iterator<UnaryFunction, Iterator>,
    boost::transform_iterator<UnaryFunction, Iterator> >
        make_transform_range(const Iterator& first, const Iterator& last, UnaryFunction f)
{
    typedef typename boost::transform_iterator<UnaryFunction, Iterator> transform_type;
    
    return std::make_pair(transform_type(first, f), transform_type(last, f));
}

template <class InputRange, class T, class BinaryOperation>
T accumulate(const InputRange& range, T init, BinaryOperation binary_op)
{
    return std::accumulate(boost::begin(range), boost::end(range), init, binary_op);
}
#endif

/*************************************************************************************************/

void virtual_machine_t::implementation_t::array_operator()
{
    stack_type::difference_type count = back().cast<stack_type::difference_type>();
    pop_back();
    
    adobe::array_t  result;

    for (stack_type::iterator first(value_stack_m.end() - count), last(value_stack_m.end());
        first != last; ++first)
    {
        result.push_back(::adobe::move(*first));
    }
            
    value_stack_m.resize(value_stack_m.size() - count);
    value_stack_m.push_back(any_regular_t(::adobe::move(result)));
}

/*************************************************************************************************/

void virtual_machine_t::implementation_t::dictionary_operator()
{
    stack_type::difference_type count = 2 * back().cast<stack_type::difference_type>();
    pop_back();
    
    adobe::dictionary_t result;

    stack_type::iterator first(value_stack_m.end() - count), last(value_stack_m.end());
    
    while (first != last)
    {
        name_t name = first->cast<adobe::name_t>();
        ++first;
        result.insert(make_pair(name, ::adobe::move(*first)));
        ++first;
    }
        
    value_stack_m.resize(value_stack_m.size() - count);
    value_stack_m.push_back(any_regular_t(::adobe::move(result)));
}

/*************************************************************************************************/

void virtual_machine_t::implementation_t::function_operator()
{
    ADOBE_ONCE_INSTANCE(adobe_virtual_machine);

    // pop the function name
    adobe::name_t   function_name(back().cast<adobe::name_t>());
    pop_back();
    
    if (back().type_info() == type_info<adobe::array_t>())
    {
        // handle unnamed parameter functions
        array_function_t    array_func;
        adobe::array_t      arguments(back().cast<adobe::array_t>());
        
        // handle function lookup
        
        if ((*array_function_table_g)(function_name, array_func))
            value_stack_m.back() = array_func(arguments);
        else if (array_function_lookup_m)
            value_stack_m.back() = array_function_lookup_m(function_name, arguments);
        else
            throw_function_not_defined(function_name);
    }
    else
    {
        // handle named parameter functions
        dictionary_function_t   dictionary_func;
        adobe::dictionary_t     arguments(back().cast<adobe::dictionary_t>());

        if ((*dictionary_function_table_g)(function_name, dictionary_func))
            value_stack_m.back() = dictionary_func(arguments);
        else if (dictionary_function_lookup_m)
            value_stack_m.back() = dictionary_function_lookup_m(function_name, arguments);
        else
            throw_function_not_defined(function_name);
    }
}

/*************************************************************************************************/

#if 0
#pragma mark -
#endif

/*************************************************************************************************/
#if !defined(ADOBE_NO_DOCUMENTATION)

virtual_machine_t::virtual_machine_t() :
    object_m(new implementation_t)
{
}

virtual_machine_t::virtual_machine_t(const virtual_machine_t& rhs) :
    object_m(new implementation_t(*rhs.object_m))
{
}

virtual_machine_t& virtual_machine_t::operator = (const virtual_machine_t& rhs)
{
    *object_m = *(rhs.object_m);

    return *this;
}

virtual_machine_t::~virtual_machine_t()
{
    delete object_m;
}
#endif // !defined(ADOBE_NO_DOCUMENTATION)

/*************************************************************************************************/
    
void virtual_machine_t::set_variable_lookup(const variable_lookup_t& lookup)
{
    object_m->variable_lookup_m = lookup;
}
    
/*************************************************************************************************/
    
void virtual_machine_t::set_array_function_lookup(const array_function_lookup_t& function)
{
    object_m->array_function_lookup_m = function;
}

/*************************************************************************************************/

void virtual_machine_t::set_dictionary_function_lookup(const dictionary_function_lookup_t& function)
{
    object_m->dictionary_function_lookup_m = function;
}
    
/*************************************************************************************************/
    
void virtual_machine_t::evaluate(const expression_t& expression)
{
    object_m->evaluate(expression);
}
    
/*************************************************************************************************/
    
const any_regular_t& virtual_machine_t::back() const
{
    return object_m->back();
}
    
/*************************************************************************************************/
    
any_regular_t& virtual_machine_t::back()
{
    return object_m->back();
}

/*************************************************************************************************/
    
void virtual_machine_t::pop_back()
{
    object_m->pop_back();
}

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/
