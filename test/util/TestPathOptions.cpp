#include <boost/test/unit_test.hpp>


#include "util/OptionsDB.h"
#include "util/Directories.h"
#include "util/i18n.h"
#include "util/XMLDoc.h"

BOOST_AUTO_TEST_SUITE(TestPathOptions);

BOOST_AUTO_TEST_CASE(LatinPath) {
    const std::filesystem::path latin1 = FilenameToPath("latin1");

    GetOptionsDB().Add<std::filesystem::path>("test.latin1.path", UserStringNop("TEST_LATIN1"), latin1);
    auto exp_latin1 = GetOptionsDB().Get<std::filesystem::path>("test.latin1.path");
    BOOST_CHECK_EQUAL(latin1, exp_latin1);

    XMLDoc doc;
    GetOptionsDB().GetXML(doc);
    auto textual_path = doc.root_node.Child("test").Child("latin1").Child("path").Text();
    BOOST_CHECK_EQUAL("latin1", textual_path);

    GetOptionsDB().SetFromXML(doc);

    exp_latin1 = GetOptionsDB().Get<std::filesystem::path>("test.latin1.path");
    BOOST_CHECK_EQUAL(latin1, exp_latin1);
}

BOOST_AUTO_TEST_CASE(EscapePath) {
    const std::filesystem::path latin1 = FilenameToPath("lat&in1");

    GetOptionsDB().Add<std::filesystem::path>("test.latampin1.path", UserStringNop("TEST_LATIN1"), latin1);
    auto exp_latin1 = GetOptionsDB().Get<std::filesystem::path>("test.latampin1.path");
    BOOST_CHECK_EQUAL(latin1, exp_latin1);

    XMLDoc doc;
    GetOptionsDB().GetXML(doc);
    auto textual_path = doc.root_node.Child("test").Child("latampin1").Child("path").Text();
    BOOST_CHECK_EQUAL("lat&amp;in1", textual_path);

    GetOptionsDB().SetFromXML(doc);

    exp_latin1 = GetOptionsDB().Get<std::filesystem::path>("test.latampin1.path");
    BOOST_CHECK_EQUAL(FilenameToPath("lat&in1"), exp_latin1);
}

BOOST_AUTO_TEST_CASE(CyrPath) {
    static constexpr std::string_view cyr_chars = "кириллица";
    const std::filesystem::path cyr = FilenameToPath(cyr_chars);

    GetOptionsDB().Add<std::filesystem::path>("test.cyr.path", UserStringNop("TEST_CYR"), FilenameToPath(cyr_chars));
    auto exp_cyr = GetOptionsDB().Get<std::filesystem::path>("test.cyr.path");
    BOOST_CHECK_EQUAL(cyr, exp_cyr);

    XMLDoc doc;
    GetOptionsDB().GetXML(doc);
    auto textual_path = doc.root_node.Child("test").Child("cyr").Child("path").Text();
    BOOST_CHECK_EQUAL(cyr_chars, textual_path);

    GetOptionsDB().SetFromXML(doc);

    exp_cyr = GetOptionsDB().Get<std::filesystem::path>("test.cyr.path");
    BOOST_CHECK_EQUAL(cyr, exp_cyr);
}

BOOST_AUTO_TEST_SUITE_END();