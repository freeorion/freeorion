#include <GG/FontFwd.h>

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE StrongSizeTypedef

#include <boost/test/unit_test.hpp>

std::size_t i = 7;
GG::StrSize x(2);

std::size_t temp_size;
std::size_t temp_size_1;
std::size_t temp_size_2;

#define ARITHMETIC_ITERATION(op)                                   \
    BOOST_CHECK_EQUAL(Value(i op x), i op Value(x));               \
    BOOST_CHECK_EQUAL(Value(x op i), Value(x) op i);               \
    BOOST_CHECK_EQUAL(Value(x op x), Value(x) op Value(x))

#define RESET()                                 \
    i = 7;                                      \
    x = GG::StrSize(2)

#define ASSIGN_ARITHMETIC_ITERATION(op)                                 \
    temp_size_1 = Value(x op i);                                        \
    RESET();                                                            \
    temp_size = Value(x);                                               \
    temp_size_2 = temp_size op i;                                       \
    RESET();                                                            \
    BOOST_CHECK_EQUAL(temp_size_1, temp_size_2);                        \
                                                                        \
    temp_size_1 = Value(x op x);                                        \
    RESET();                                                            \
    temp_size = Value(x);                                               \
    temp_size_2 = temp_size op temp_size;                               \
    RESET();                                                            \
    BOOST_CHECK_EQUAL(temp_size_1, temp_size_2)

#define COMPARISON_ITERATION(op)                                 \
    BOOST_CHECK_EQUAL(i op x, i op Value(x));                    \
    BOOST_CHECK_EQUAL(x op i, Value(x) op i);                    \
    BOOST_CHECK_EQUAL(x op x, Value(x) op Value(x))

BOOST_AUTO_TEST_CASE( arithmetic )
{
    ARITHMETIC_ITERATION(+);
    ARITHMETIC_ITERATION(-);
    ARITHMETIC_ITERATION(*);

    BOOST_CHECK_EQUAL(Value(x / i), Value(x) / i);
    BOOST_CHECK_EQUAL(Value(x / x), Value(x) / Value(x));

    BOOST_CHECK_EQUAL(Value(x % i), Value(x) % i);
    BOOST_CHECK_EQUAL(Value(x % x), Value(x) % Value(x));

    ASSIGN_ARITHMETIC_ITERATION(+=);
    ASSIGN_ARITHMETIC_ITERATION(-=);
    ASSIGN_ARITHMETIC_ITERATION(*=);
    ASSIGN_ARITHMETIC_ITERATION(/=);
    ASSIGN_ARITHMETIC_ITERATION(%=);
}

BOOST_AUTO_TEST_CASE( comparison )
{
    COMPARISON_ITERATION(<);
    COMPARISON_ITERATION(>);
    COMPARISON_ITERATION(==);
    COMPARISON_ITERATION(!=);
    COMPARISON_ITERATION(<=);
    COMPARISON_ITERATION(>=);
}

BOOST_AUTO_TEST_CASE( unary )
{
    GG::StrSize temp_x_1;

    temp_x_1 = ++x;
    RESET();
    temp_size_2 = ++(temp_size = Value(x));
    RESET();
    BOOST_CHECK_EQUAL(temp_x_1, temp_size_2);

    temp_x_1 = x++;
    RESET();
    temp_size_2 = (temp_size = Value(x))++;
    RESET();
    BOOST_CHECK_EQUAL(temp_x_1, temp_size_2);

    temp_x_1 = --x;
    RESET();
    temp_size_2 = --(temp_size = Value(x));
    RESET();
    BOOST_CHECK_EQUAL(temp_x_1, temp_size_2);

    temp_x_1 = x--;
    RESET();
    temp_size_2 = (temp_size = Value(x))--;
    RESET();
    BOOST_CHECK_EQUAL(temp_x_1, temp_size_2);

    BOOST_CHECK_EQUAL(!x, !Value(x));

    BOOST_CHECK_EQUAL(-x, -Value(x));
}
