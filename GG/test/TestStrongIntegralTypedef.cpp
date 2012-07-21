#include <GG/PtRect.h>

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE StrongIntegralTypedef

#include <boost/test/unit_test.hpp>

int i = 7;
double d = 3.1;
GG::X x(2);
GG::X_d xd(5.1);

int temp_int;
int temp_int_1;
int temp_int_2;
double temp_double;
double temp_double_1;
double temp_double_2;

#define ARITHMETIC_ITERATION(op)                                   \
    BOOST_CHECK_EQUAL(Value(i op x), i op Value(x));               \
    BOOST_CHECK_EQUAL(Value(x op i), Value(x) op i);               \
    BOOST_CHECK_EQUAL(Value(d op x), d op Value(x));               \
    BOOST_CHECK_EQUAL(Value(x op d), Value(x) op d);               \
    BOOST_CHECK_EQUAL(Value(x op x), Value(x) op Value(x));        \
    BOOST_CHECK_EQUAL(Value(x op xd), Value(x) op Value(xd));      \
    BOOST_CHECK_EQUAL(Value(xd op x), Value(xd) op Value(x));      \
                                                                   \
    BOOST_CHECK_EQUAL(Value(i op xd), i op Value(xd));             \
    BOOST_CHECK_EQUAL(Value(xd op i), Value(xd) op i);             \
    BOOST_CHECK_EQUAL(Value(d op xd), d op Value(xd));             \
    BOOST_CHECK_EQUAL(Value(xd op d), Value(xd) op d);             \
    BOOST_CHECK_EQUAL(Value(xd op xd), Value(xd) op Value(xd))

#define RESET()                                 \
    i = 7;                                      \
    d = 3.1;                                    \
    x = GG::X(2);                               \
    xd = GG::X_d(5.1)

#define ASSIGN_ARITHMETIC_ITERATION(op)                                 \
    temp_int_1 = Value(x op i);                                         \
    RESET();                                                            \
    temp_int = Value(x);                                                \
    temp_int_2 = temp_int op i;                                         \
    RESET();                                                            \
    BOOST_CHECK_EQUAL(temp_int_1, temp_int_2);                          \
                                                                        \
    temp_double_1 = Value(x op d);                                      \
    RESET();                                                            \
    temp_int = Value(x);                                                \
    temp_double_2 = temp_int op d;                                      \
    RESET();                                                            \
    BOOST_CHECK_EQUAL(temp_double_1, temp_double_2);                    \
                                                                        \
    temp_int_1 = Value(x op x);                                         \
    RESET();                                                            \
    temp_int = Value(x);                                                \
    temp_int_2 = temp_int op temp_int;                                  \
    RESET();                                                            \
    BOOST_CHECK_EQUAL(temp_int_1, temp_int_2);                          \
                                                                        \
    temp_double_1 = Value(x op xd);                                     \
    RESET();                                                            \
    temp_int = Value(x);                                                \
    temp_double = Value(xd);                                            \
    temp_double_2 = temp_int op temp_double;                            \
    RESET();                                                            \
    BOOST_CHECK_EQUAL(temp_double_1, temp_double_2);                    \
                                                                        \
    temp_double_1 = Value(xd op x);                                     \
    RESET();                                                            \
    temp_double = Value(xd);                                            \
    temp_int = Value(x);                                                \
    temp_double_2 = temp_double op temp_int;                            \
    RESET();                                                            \
    BOOST_CHECK_EQUAL(temp_double_1, temp_double_2);                    \
                                                                        \
    temp_double_1 = Value(xd op i);                                     \
    RESET();                                                            \
    temp_double = Value(xd);                                            \
    temp_double_2 = temp_double op i;                                   \
    RESET();                                                            \
    BOOST_CHECK_EQUAL(temp_double_1, temp_double_2);                    \
                                                                        \
    temp_double_1 = Value(xd op d);                                     \
    RESET();                                                            \
    temp_double = Value(xd);                                            \
    temp_double_2 = temp_double op d;                                   \
    RESET();                                                            \
    BOOST_CHECK_EQUAL(temp_double_1, temp_double_2);                    \
                                                                        \
    temp_double_1 = Value(xd op xd);                                    \
    RESET();                                                            \
    temp_double = Value(xd);                                            \
    temp_double_2 = temp_double op temp_double;                         \
    RESET();                                                            \
    BOOST_CHECK_EQUAL(temp_double_1, temp_double_2)

#define COMPARISON_ITERATION(op)                                 \
    BOOST_CHECK_EQUAL(i op x, i op Value(x));                    \
    BOOST_CHECK_EQUAL(x op i, Value(x) op i);                    \
    BOOST_CHECK_EQUAL(d op x, d op Value(x));                    \
    BOOST_CHECK_EQUAL(x op d, Value(x) op d);                    \
    BOOST_CHECK_EQUAL(x op x, Value(x) op Value(x));             \
    BOOST_CHECK_EQUAL(x op xd, Value(x) op Value(xd));           \
    BOOST_CHECK_EQUAL(xd op x, Value(xd) op Value(x));           \
                                                                 \
    BOOST_CHECK_EQUAL(i op xd, i op Value(xd));                  \
    BOOST_CHECK_EQUAL(xd op i, Value(xd) op i);                  \
    BOOST_CHECK_EQUAL(d op xd, d op Value(xd));                  \
    BOOST_CHECK_EQUAL(xd op d, Value(xd) op d);                  \
    BOOST_CHECK_EQUAL(xd op xd, Value(xd) op Value(xd))

BOOST_AUTO_TEST_CASE( arithmetic )
{
    ARITHMETIC_ITERATION(+);
    ARITHMETIC_ITERATION(-);
    ARITHMETIC_ITERATION(*);

    BOOST_CHECK_EQUAL(Value(x / i), Value(x) / i);
    BOOST_CHECK_EQUAL(Value(x / d), Value(x) / d);
    BOOST_CHECK_EQUAL(Value(x / x), Value(x) / Value(x));
    BOOST_CHECK_EQUAL(Value(x / xd), Value(x) / Value(xd));
    BOOST_CHECK_EQUAL(Value(xd / x), Value(xd) / Value(x));
    BOOST_CHECK_EQUAL(Value(xd / i), Value(xd) / i);
    BOOST_CHECK_EQUAL(Value(xd / d), Value(xd) / d);
    BOOST_CHECK_EQUAL(Value(xd / xd), Value(xd) / Value(xd));

    BOOST_CHECK_EQUAL(Value(x % i), Value(x) % i);
    BOOST_CHECK_EQUAL(Value(x % x), Value(x) % Value(x));

    ASSIGN_ARITHMETIC_ITERATION(+);
    ASSIGN_ARITHMETIC_ITERATION(-);
    ASSIGN_ARITHMETIC_ITERATION(*);
    ASSIGN_ARITHMETIC_ITERATION(/);

    temp_int_1 = Value(x %= i);
    RESET();
    temp_int = Value(x);
    temp_int_2 = temp_int %= i;
    RESET();
    BOOST_CHECK_EQUAL(temp_int_1, temp_int_2);

    temp_int_1 = Value(x %= x);
    RESET();
    temp_int = Value(x);
    temp_int_2 = temp_int %= temp_int;
    RESET();
    BOOST_CHECK_EQUAL(temp_int_1, temp_int_2);
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
    GG::X temp_x_1;
    GG::X_d temp_xd_1;

    temp_x_1 = ++x;
    RESET();
    temp_int_2 = ++(temp_int = Value(x));
    RESET();
    BOOST_CHECK_EQUAL(temp_x_1, temp_int_2);

    temp_xd_1 = ++xd;
    RESET();
    temp_double_2 = ++(temp_double = Value(xd));
    RESET();
    BOOST_CHECK_EQUAL(temp_xd_1, temp_double_2);

    temp_x_1 = x++;
    RESET();
    temp_int_2 = (temp_int = Value(x))++;
    RESET();
    BOOST_CHECK_EQUAL(temp_x_1, temp_int_2);

    temp_xd_1 = xd++;
    RESET();
    temp_double_2 = (temp_double = Value(xd))++;
    RESET();
    BOOST_CHECK_EQUAL(temp_xd_1, temp_double_2);

    temp_x_1 = --x;
    RESET();
    temp_int_2 = --(temp_int = Value(x));
    RESET();
    BOOST_CHECK_EQUAL(temp_x_1, temp_int_2);

    temp_xd_1 = --xd;
    RESET();
    temp_double_2 = --(temp_double = Value(xd));
    RESET();
    BOOST_CHECK_EQUAL(temp_xd_1, temp_double_2);

    temp_x_1 = x--;
    RESET();
    temp_int_2 = (temp_int = Value(x))--;
    RESET();
    BOOST_CHECK_EQUAL(temp_x_1, temp_int_2);

    temp_xd_1 = xd--;
    RESET();
    temp_double_2 = (temp_double = Value(xd))--;
    RESET();
    BOOST_CHECK_EQUAL(temp_xd_1, temp_double_2);

    BOOST_CHECK_EQUAL(!x, !Value(x));
    BOOST_CHECK_EQUAL(!xd, !Value(xd));

    BOOST_CHECK_EQUAL(-x, -Value(x));
    BOOST_CHECK_EQUAL(-xd, -Value(xd));
}
