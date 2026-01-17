#include "../util/Enum.h"

#include <boost/preprocessor/seq/elem.hpp>
#include <boost/preprocessor/seq/size.hpp>
#include <boost/preprocessor/tuple/elem.hpp>
#include <boost/preprocessor/tuple/size.hpp>

#include <tuple>

namespace {
using namespace std::literals;

FO_DEF_ENUM(OneT, int8_t, ((a, "a", 0, 0))((v, "v", 1, 5)))
static_assert(OneT::v > OneT::a);

FO_DEF_ENUM_TOSTRING(OneT, 1, ((a, "a", 0, 0))((v, "v", 1, 5)))
static_assert(to_string(OneT::v) == "v");

static_assert([]() {
    using values_array_t = std::array<std::pair<OneT, std::string_view>, 2>;
    constexpr values_array_t vals{{
        FO_DEF_ENUM_ITERATE_VALUE(_, OneT, (a, "a", 0, 0))
        FO_DEF_ENUM_ITERATE_VALUE(_, OneT, (v, "v"))
    }};
    return vals[0].first == OneT::a;
}());

FO_DEF_ENUM_ITERATE(OneT, 1, ((a, "a", 0, 0))((v, "v", 1, 5)))
static_assert(OneTValues().size() == 2);

FO_DEF_ENUM_FROM_STRING(OneT, 1)
static_assert(OneTFromString("v") == OneT::v);

FO_DEF_ENUM_IMPL_IMPL(TwoT, int8_t, 1, ((b, "b", 1, 1))((c, "c", 1, 0)))
static_assert(TwoTFromString(to_string(TwoT::b)) == TwoT::b);

using TestTuple = std::tuple<TwoT, std::string_view, int, int>;
static_assert((TestTuple FO_DEF_ENUM_TX_VALUE_TUPLE(TwoT::b, 1, 6))  ==  TestTuple{TwoT::b, "TwoT::b"sv, 1, 6});
static_assert((TestTuple FO_DEF_ENUM_TX_VALUE_TUPLE(TwoT::b, 0, 1))  ==  TestTuple{TwoT::b, "TwoT::b"sv, 0, 1});

static_assert( BOOST_PP_TUPLE_SIZE((TwoT::b, 1, 6)) == 3u );

static_assert((TestTuple FO_DEF_ENUM_TX_VALUE((TwoT::b, 6)))  ==  TestTuple{TwoT::b, "TwoT::b"sv, 1, 6}); // identified two elements in input, indicated with flag 1 and passed along value 6
static_assert((TestTuple FO_DEF_ENUM_TX_VALUE((TwoT::b)))     ==  TestTuple{TwoT::b, "TwoT::b"sv, 0, 0}); // identified on element in input, indicated with flag 0 and default value 0

#define input_seq_0 ((TwoT::c))((TwoT::b, 5))((TwoT::b, 0))
#define tx_seq_0 FO_DEF_ENUM_TX_VALUES(input_seq_0)
static_assert( BOOST_PP_SEQ_SIZE(tx_seq_0) == 3u );
#define tx_seq_0_elem0 BOOST_PP_SEQ_ELEM(0, tx_seq_0)
static_assert( BOOST_PP_TUPLE_SIZE(tx_seq_0_elem0) == 4u );
static_assert( BOOST_PP_TUPLE_ELEM(0, tx_seq_0_elem0) == TwoT::c);
#define tx_seq_0_elem1 BOOST_PP_SEQ_ELEM(1, tx_seq_0)
static_assert( BOOST_PP_TUPLE_SIZE(tx_seq_0_elem1) == 4u );
static_assert( BOOST_PP_TUPLE_ELEM(0, tx_seq_0_elem1) == TwoT::b);


#define TEST_SEQ2_RAW ((NEG_ONE, -1))((ZERO))((ONE))((TWO))((THREE))
static_assert( BOOST_PP_SEQ_SIZE(TEST_SEQ2_RAW) == 5u );
static_assert( BOOST_PP_TUPLE_SIZE(BOOST_PP_SEQ_ELEM(0, TEST_SEQ2_RAW)) == 2u );
static_assert( BOOST_PP_TUPLE_SIZE(BOOST_PP_SEQ_ELEM(1, TEST_SEQ2_RAW)) == 1u );

#define TEST_SEQ2_RAW_SEQ_ELEM0 BOOST_PP_SEQ_ELEM(0, TEST_SEQ2_RAW)

inline constexpr std::pair<std::string_view, int> neg_one {
    BOOST_PP_STRINGIZE(BOOST_PP_TUPLE_ELEM(0, TEST_SEQ2_RAW_SEQ_ELEM0)) ,
    BOOST_PP_TUPLE_ELEM(1, TEST_SEQ2_RAW_SEQ_ELEM0)
};
static_assert( neg_one == std::pair{"NEG_ONE"sv, -1});


}
