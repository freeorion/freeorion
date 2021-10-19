#include <boost/test/unit_test.hpp>

#include "ClientAppFixture.h"

BOOST_FIXTURE_TEST_SUITE(TestChecksum, ClientAppFixture)

BOOST_AUTO_TEST_CASE(compare_checksum) {
    auto checksums = CheckSumContent();

    BOOST_WARN_EQUAL(checksums["NamedValueRefManager"], 42021);
    BOOST_WARN_EQUAL(checksums["TechManager"], 9095847);
}

BOOST_AUTO_TEST_SUITE_END()

