#include <boost/test/unit_test.hpp>

#include <boost/filesystem/path.hpp>

#include "util/OptionsDB.h"
#include "util/i18n.h"
#include "util/XMLDoc.h"

BOOST_AUTO_TEST_SUITE(TestPathOptions);

BOOST_AUTO_TEST_CASE(LatinPath) {
    boost::filesystem::path latin1 = "latin1";

    GetOptionsDB().Add<boost::filesystem::path>("test.latin1.path", UserStringNop("TEST_LATIN1"), std::move(latin1));
    auto exp_latin1 = GetOptionsDB().Get<boost::filesystem::path>("test.latin1.path");
    BOOST_CHECK_EQUAL(boost::filesystem::path{"latin1"}, exp_latin1);

    XMLDoc doc;
    GetOptionsDB().GetXML(doc);
    auto textual_path = doc.root_node.Child("test").Child("latin1").Child("path").Text();
    BOOST_CHECK_EQUAL("latin1", textual_path);

    GetOptionsDB().SetFromXML(doc);

    exp_latin1 = GetOptionsDB().Get<boost::filesystem::path>("test.latin1.path");
    BOOST_CHECK_EQUAL(boost::filesystem::path{"latin1"}, exp_latin1);
}

BOOST_AUTO_TEST_CASE(EscapePath) {
    boost::filesystem::path latin1 = "lat&in1";

    GetOptionsDB().Add<boost::filesystem::path>("test.latampin1.path", UserStringNop("TEST_LATIN1"), std::move(latin1));
    auto exp_latin1 = GetOptionsDB().Get<boost::filesystem::path>("test.latampin1.path");
    BOOST_CHECK_EQUAL(boost::filesystem::path{"lat&in1"}, exp_latin1);

    XMLDoc doc;
    GetOptionsDB().GetXML(doc);
    auto textual_path = doc.root_node.Child("test").Child("latampin1").Child("path").Text();
    BOOST_CHECK_EQUAL("lat&amp;in1", textual_path);

    GetOptionsDB().SetFromXML(doc);

    exp_latin1 = GetOptionsDB().Get<boost::filesystem::path>("test.latampin1.path");
    BOOST_CHECK_EQUAL(boost::filesystem::path{"lat&in1"}, exp_latin1);
}

BOOST_AUTO_TEST_CASE(CyrPath) {
    boost::filesystem::path cyr = "кириллица";

    GetOptionsDB().Add<boost::filesystem::path>("test.cyr.path", UserStringNop("TEST_CYR"), std::move(cyr));
    auto exp_cyr = GetOptionsDB().Get<boost::filesystem::path>("test.cyr.path");
    BOOST_CHECK_EQUAL(boost::filesystem::path{"кириллица"}, exp_cyr);

    XMLDoc doc;
    GetOptionsDB().GetXML(doc);
    auto textual_path = doc.root_node.Child("test").Child("cyr").Child("path").Text();
    BOOST_CHECK_EQUAL("&#208;&#186;&#208;&#184;&#209;&#128;&#208;&#184;&#208;&#187;&#208;&#187;&#208;&#184;&#209;&#134;&#208;&#176;", textual_path);

    GetOptionsDB().SetFromXML(doc);

    exp_cyr = GetOptionsDB().Get<boost::filesystem::path>("test.cyr.path");
    BOOST_CHECK_EQUAL(boost::filesystem::path{"кириллица"}, exp_cyr);
}

BOOST_AUTO_TEST_SUITE_END();

