#include <GG/AdamParser.h>

#include <GG/adobe/adam_evaluate.hpp>
#include <GG/adobe/adam_parser.hpp>
#include <GG/adobe/array.hpp>
#include <GG/adobe/dictionary.hpp>
#include <GG/adobe/future/widgets/headers/virtual_machine_extension.hpp>

#include <fstream>
#include <iostream>

#define BOOST_TEST_NO_MAIN
#include <boost/test/unit_test.hpp>

#include "TestingUtils.h"


const char* g_input_file = 0;

bool instrument_positions = false;

namespace GG {

    struct StoreAddCellParams
    {
        StoreAddCellParams(adobe::array_t& array, const std::string& str) :
            m_array(array),
            m_str(str)
            {}

        void operator()(adobe::adam_callback_suite_t::cell_type_t type,
                        adobe::name_t cell_name,
                        const adobe::line_position_t& position,
                        const adobe::array_t& expr_or_init,
                        const std::string& brief,
                        const std::string& detailed)
        {
            std::string type_str;
            switch (type)
            {
            case adobe::adam_callback_suite_t::input_k: type_str = "input_k";
            case adobe::adam_callback_suite_t::output_k: type_str = "output_k";
            case adobe::adam_callback_suite_t::constant_k: type_str = "constant_k";
            case adobe::adam_callback_suite_t::logic_k: type_str = "logic_k";
            case adobe::adam_callback_suite_t::invariant_k: type_str = "invariant_k";
            }
            push_back(m_array, type_str);
            push_back(m_array, cell_name);
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
            push_back(m_array, expr_or_init);
            push_back(m_array, brief);
            push_back(m_array, detailed);
        }

        adobe::array_t& m_array;
        const std::string& m_str;
    };

    struct StoreAddRelationParams
    {
        StoreAddRelationParams(adobe::array_t& array, const std::string& str) :
            m_array(array),
            m_str(str)
            {}

        void operator()(const adobe::line_position_t& position,
                        const adobe::array_t& conditional,
                        const adobe::adam_callback_suite_t::relation_t* first,
                        const adobe::adam_callback_suite_t::relation_t* last,
                        const std::string& brief,
                        const std::string& detailed)
        {
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
            push_back(m_array, conditional);
            while (first != last) {
                push_back(m_array, first->name_m);
                if (instrument_positions) {
                    std::cerr << first->position_m.stream_name() << ":"
                              << first->position_m.line_number_m << ":"
                              << first->position_m.line_start_m << ":"
                              << first->position_m.position_m << ":";
                    if (position.line_start_m) {
                        std::cerr << " \"" << std::string(m_str.begin() + first->position_m.line_start_m,
                                                          m_str.begin() + first->position_m.position_m)
                                  << "\"";
                    }
                    std::cerr << "\n";
                } else {
                    push_back(m_array, first->position_m.stream_name());
                    push_back(m_array, first->position_m.line_number_m);
                    push_back(m_array, std::size_t(first->position_m.line_start_m));
                    push_back(m_array, std::size_t(first->position_m.position_m));
                }
                push_back(m_array, first->expression_m);
                push_back(m_array, first->detailed_m);
                push_back(m_array, first->brief_m);
                ++first;
            }
            push_back(m_array, brief);
            push_back(m_array, detailed);
        }

        adobe::array_t& m_array;
        const std::string& m_str;
    };

    struct StoreAddInterfaceParams
    {
        StoreAddInterfaceParams(adobe::array_t& array, const std::string& str) :
            m_array(array),
            m_str(str)
            {}

        void operator()(adobe::name_t cell_name,
                        bool linked,
                        const adobe::line_position_t& position1,
                        const adobe::array_t& initializer,
                        const adobe::line_position_t& position2,
                        const adobe::array_t& expression,
                        const std::string& brief,
                        const std::string& detailed)
        {
            push_back(m_array, cell_name);
            push_back(m_array, linked);
            if (instrument_positions) {
                std::cerr << position1.stream_name() << ":"
                          << position1.line_number_m << ":"
                          << position1.line_start_m << ":"
                          << position1.position_m << ":";
                if (position1.line_start_m) {
                    std::cerr << " \"" << std::string(m_str.begin() + position1.line_start_m,
                                                      m_str.begin() + position1.position_m)
                          << "\"";
                }
                std::cerr << "\n";
            } else {
                push_back(m_array, position1.stream_name());
                push_back(m_array, position1.line_number_m);
                push_back(m_array, std::size_t(position1.line_start_m));
                push_back(m_array, std::size_t(position1.position_m));
            }
            push_back(m_array, initializer);
            if (instrument_positions) {
                std::cerr << position2.stream_name() << ":"
                          << position2.line_number_m << ":"
                          << position2.line_start_m << ":"
                          << position2.position_m << ":";
                if (position2.line_start_m) {
                    std::cerr << " \"" << std::string(m_str.begin() + position2.line_start_m,
                                                      m_str.begin() + position2.position_m)
                              << "\"";
                }
                std::cerr << "\n";
            } else {
                push_back(m_array, position2.stream_name());
                push_back(m_array, position2.line_number_m);
                push_back(m_array, std::size_t(position2.line_start_m));
                push_back(m_array, std::size_t(position2.position_m));
            }
            push_back(m_array, expression);
            push_back(m_array, brief);
            push_back(m_array, detailed);
        }

        adobe::array_t& m_array;
        const std::string& m_str;
    };

}

BOOST_AUTO_TEST_CASE( adam_parser )
{
    std::string file_contents = read_file(g_input_file);

    adobe::array_t new_parse;
    adobe::array_t old_parse;

    adobe::adam_callback_suite_t old_parse_callbacks;
    old_parse_callbacks.add_cell_proc_m = GG::StoreAddCellParams(old_parse, file_contents);
    old_parse_callbacks.add_relation_proc_m = GG::StoreAddRelationParams(old_parse, file_contents);
    old_parse_callbacks.add_interface_proc_m = GG::StoreAddInterfaceParams(old_parse, file_contents);

    adobe::adam_callback_suite_t new_parse_callbacks;
    new_parse_callbacks.add_cell_proc_m = GG::StoreAddCellParams(new_parse, file_contents);
    new_parse_callbacks.add_relation_proc_m = GG::StoreAddRelationParams(new_parse, file_contents);
    new_parse_callbacks.add_interface_proc_m = GG::StoreAddInterfaceParams(new_parse, file_contents);

    std::cout << "sheet:\"\n" << file_contents << "\n\"\n"
              << "filename: " << g_input_file << '\n';
    bool original_parse_failed = false;
    try {
        std::stringstream ss(file_contents);
        adobe::parse(ss, adobe::line_position_t(g_input_file), old_parse_callbacks);
    } catch (const adobe::stream_error_t& e) {
        original_parse_failed = true;
    }
    if (original_parse_failed)
        std::cout << "original: <parse failure>\n";
    else
        std::cout << "original: " << old_parse << "\n";
    bool new_parse_failed = !GG::Parse(file_contents, g_input_file, new_parse_callbacks);
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
init_unit_test_suite( int, char* argv[] )   {
#endif

#if !defined(BOOST_TEST_DYN_LINK)
    g_input_file = argv[1];
#endif

#ifdef BOOST_TEST_MODULE
    using namespace ::boost::unit_test;
    assign_op( framework::master_test_suite().p_name.value, BOOST_TEST_STRINGIZE( BOOST_TEST_MODULE ).trim( "\"" ), 0 );
    
#endif

#ifdef BOOST_TEST_ALTERNATIVE_INIT_API
    return true;
#else
    return 0;
#endif
}

#if defined(BOOST_TEST_DYN_LINK)
int BOOST_TEST_CALL_DECL
main( int argc, char* argv[] )
{
    g_input_file = argv[1];
    return ::boost::unit_test::unit_test_main( &init_unit_test, argc, argv );
}
#endif

