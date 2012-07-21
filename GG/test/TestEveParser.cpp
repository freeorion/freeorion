#include <GG/EveParser.h>

#include <GG/adobe/eve_parser.hpp>
#include <GG/adobe/array.hpp>
#include <GG/adobe/dictionary.hpp>

#include <fstream>
#include <iostream>

#define BOOST_TEST_DYN_LINK

#include <boost/test/unit_test.hpp>

#include "TestingUtils.h"


const char* g_input_file = 0;

bool instrument_positions = false;

namespace GG {

    struct StoreAddViewParams
    {
        StoreAddViewParams(adobe::array_t& array, const std::string& str) :
            m_array(array),
            m_str(str)
            {}

        boost::any operator()(const boost::any& parent,
                              const adobe::line_position_t& parse_location,
                              adobe::name_t name,
                              const adobe::array_t& parameters,
                              const std::string& brief,
                              const std::string& detailed)
        {
            // Note that we are forced to ignore parent

            if (instrument_positions) {
                std::cerr << parse_location.stream_name() << ":"
                          << parse_location.line_number_m << ":"
                          << parse_location.line_start_m << ":"
                          << parse_location.position_m << ":";
                if (parse_location.line_start_m) {
                    std::cerr << " \"" << std::string(m_str.begin() + parse_location.line_start_m,
                                                      m_str.begin() + parse_location.position_m)
                              << "\"";
                }
                std::cerr << "\n";
            } else {
                push_back(m_array, parse_location.stream_name());
                push_back(m_array, parse_location.line_number_m);
                push_back(m_array, std::size_t(parse_location.line_start_m));
                push_back(m_array, std::size_t(parse_location.position_m));
            }
            push_back(m_array, name);
            push_back(m_array, parameters);
            push_back(m_array, brief);
            push_back(m_array, detailed);

            return boost::any();
        }

        adobe::array_t& m_array;
        const std::string& m_str;
    };

    struct StoreAddCellParams
    {
        StoreAddCellParams(adobe::array_t& array, const std::string& str) :
            m_array(array),
            m_str(str)
            {}

        void operator()(adobe::eve_callback_suite_t::cell_type_t type,
                        adobe::name_t name,
                        const adobe::line_position_t& position,
                        const adobe::array_t& initializer,
                        const std::string& brief,
                        const std::string& detailed)
        {
            std::string type_str;
            switch (type)
            {
            case adobe::eve_callback_suite_t::constant_k: type_str = "constant_k";
            case adobe::eve_callback_suite_t::interface_k: type_str = "interface_k";
            }
            push_back(m_array, type_str);
            push_back(m_array, name);
            if (instrument_positions) {
                std::cerr << position.stream_name() << ":"
                          << position.line_number_m << ":"
                          << position.line_start_m << ":"
                          << position.position_m << ":";
                if (position.line_start_m) {
                    std::cerr << " \"" << std::string(m_str.begin() + position.line_start_m,
                                                      m_str.begin() + position.position_m)
                              << "\"";
                }
                std::cerr << "\n";
            } else {
                push_back(m_array, position.stream_name());
                push_back(m_array, position.line_number_m);
                push_back(m_array, std::size_t(position.line_start_m));
                push_back(m_array, std::size_t(position.position_m));
            }
            push_back(m_array, initializer);
            push_back(m_array, brief);
            push_back(m_array, detailed);
        }

        adobe::array_t& m_array;
        const std::string& m_str;
    };

}

BOOST_AUTO_TEST_CASE( eve_parser )
{
    std::string file_contents = read_file(g_input_file);

    adobe::array_t new_parse;
    adobe::array_t old_parse;

    adobe::eve_callback_suite_t old_parse_callbacks;
    old_parse_callbacks.add_view_proc_m = GG::StoreAddViewParams(old_parse, file_contents);
    old_parse_callbacks.add_cell_proc_m = GG::StoreAddCellParams(old_parse, file_contents);

    adobe::eve_callback_suite_t new_parse_callbacks;
    new_parse_callbacks.add_view_proc_m = GG::StoreAddViewParams(new_parse, file_contents);
    new_parse_callbacks.add_cell_proc_m = GG::StoreAddCellParams(new_parse, file_contents);

    std::cout << "layout:\"\n" << file_contents << "\n\"\n"
              << "filename: " << g_input_file << '\n';
    bool original_parse_failed = false;
    try {
        std::stringstream ss(file_contents);
        adobe::parse(ss, adobe::line_position_t(g_input_file), boost::any(), old_parse_callbacks);
    } catch (const adobe::stream_error_t& e) {
        original_parse_failed = true;
    }
    if (original_parse_failed)
        std::cout << "original: <parse failure>\n";
    else
        std::cout << "original: " << old_parse << "\n";
    bool new_parse_failed = !GG::Parse(file_contents, g_input_file, boost::any(), new_parse_callbacks);
    if (new_parse_failed)
        std::cout << "new:      <parse failure>\n";
    else
        std::cout << "new:      " << new_parse << "\n";
    bool pass =
        original_parse_failed && new_parse_failed ||
        new_parse == old_parse;
    std::cout << (pass ? "PASS" : "FAIL") << "\n";

    if (!pass) {
        std::cout << "original (verbose):\n";
        verbose_dump(old_parse);
        std::cout << "new (verbose):\n";
        verbose_dump(new_parse);
    }

    std::cout << "\n";

    BOOST_CHECK(pass);
}

// Most of this is boilerplate cut-and-pasted from Boost.Test.  We need to
// select which test(s) to do, so we can't use it here unmodified.

#ifdef BOOST_TEST_ALTERNATIVE_INIT_API
bool init_unit_test()                   {
#else
::boost::unit_test::test_suite*
init_unit_test_suite( int, char* [] )   {
#endif

#ifdef BOOST_TEST_MODULE
    using namespace ::boost::unit_test;
    assign_op( framework::master_test_suite().p_name.value, BOOST_TEST_STRINGIZE( BOOST_TEST_MODULE ).trim( "\"" ), 0 );
    
#endif

#ifdef BOOST_TEST_ALTERNATIVE_INIT_API
    return true;
}
#else
    return 0;
}
#endif

int BOOST_TEST_CALL_DECL
main( int argc, char* argv[] )
{
    g_input_file = argv[1];
    return ::boost::unit_test::unit_test_main( &init_unit_test, argc, argv );
}
