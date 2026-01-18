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


/** types of diplomatic empire affiliations to another empire*/
FO_ENUM(
    (TestEnum),
    ((INVALID, -1))((ZERO, 0))((ONE, 1))
    ((TWO))((THREE))((FOUR))((FIVE))((SIX))
    ((SEVEN))((EIGHT))((NINE))((TEN))
    ((ELEVEN))((TWELVE))((THIRTEEN))((FOURTEEN))
    ((FIFTEEN))((SIXTEEN))((SEVENTEEN))((EIGHTEEN))
    ((NINETEEN))((TWENTY))((NUM_TEST))
)

static_assert(TestEnum{-1} == TestEnum::INVALID);
static_assert(TestEnum{} == TestEnum::ZERO);
static_assert(TestEnum{12} == TestEnum::TWELVE);
static_assert(TestEnum{20} == TestEnum::TWENTY);

static_assert(TestEnumFromString("ZERO", TestEnum::INVALID) == TestEnum{});
static_assert(TestEnumFromString("TWO", TestEnum::INVALID) == TestEnum::TWO);
static_assert(TestEnumFromString("TWENTY", TestEnum::NINE) == TestEnum::TWENTY);

static_assert(TestEnumFromString("", TestEnum::INVALID) == TestEnum::INVALID);
static_assert(TestEnumFromString("ZERO", TestEnum::INVALID) == TestEnum{});
static_assert(TestEnumFromString("TWO", TestEnum::INVALID) == TestEnum::TWO);
static_assert(TestEnumFromString("TWENTY", TestEnum::NINE) == TestEnum::TWENTY);

static_assert(to_string(TestEnum::INVALID) == "INVALID");
static_assert(to_string(TestEnum::ONE) == "ONE");
static_assert(to_string(TestEnum::NINETEEN) == "NINETEEN");

static_assert(TestEnumImpl::vals.size());

constexpr auto test_enum_vals = TestEnumValues();
static_assert(std::find_if(test_enum_vals.begin(), test_enum_vals.end(),
                           [](const auto& val_str) { return val_str.first == TestEnum::NINE; })->second == "NINE");

constexpr auto test_arr = []() {
    std::array<std::string_view, static_cast<std::size_t>(TestEnum::NUM_TEST)> arr{};
    for (std::size_t idx = 0u; idx < arr.size(); ++idx)
        arr[idx] = test_enum_vals[idx].second;
    return arr;
}();
static_assert(test_arr[4] == "THREE");

static_assert(test_enum_vals.front() == std::pair{TestEnum::INVALID, std::string_view{"INVALID"}});
static_assert(test_enum_vals[3].second == "TWO");
static_assert(std::is_enum_v<TestEnum>);
static_assert(!std::is_arithmetic_v<TestEnum>);
static_assert(!std::is_signed_v<TestEnum>);
static_assert(std::is_signed_v<std::underlying_type_t<TestEnum>>);
static_assert(!std::is_unsigned_v<TestEnum>);

struct Encloses {
    FO_ENUM_BIG(
        (Encloses, Enclosed),
        ((INVALID, -1))
        ((ZERO, 0))
        ((ONE, 1))
        ((TWO_HUNDRED, 200))
        ((THOUSAND, 1000))
    )
};

}

