#include "test.h"

#include "parse/ConditionParser.h"
#include "parse/EffectParser.h"
#include "parse/EnumParser.h"
#include "parse/Parse.h"
#include "parse/ReportParseError.h"
#include "Empire/Empire.h"
#include "universe/ValueRef.h"

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/filesystem/fstream.hpp>

#include <fstream>


void send_error_string(const std::string& str)
{ std::cerr << str; }

void print_help()
{
    std::cout << "Usage: test lexer|double_value_ref_parser|string_value_ref_parser|planet_size_value_ref_parser|planet_type_value_ref_parser|planet_environment_value_ref_parser|star_type_value_ref_parser|condition_parser|effect_parser|buildings_parser|specials_parser|species_parser|techs_parser|items_parser|ship_parts_parser|ship_hulls_parser|ship_designs_parser|fleet_plans_parser|monster_fleet_plans_parser|alignments_parser <-f filename>|<test string> --fail" << std::endl;
}

int main(int argc, char* argv[])
{
    parse::report_error_::send_error_string = &send_error_string;

    if (argc < 3) {
        print_help();
        exit(1);
    }

    const std::string test_str = argv[1];
    test_type test = unknown;
#define CASE(x) if (test_str == #x) test = x
    CASE(lexer);
    CASE(double_value_ref_parser);
    CASE(string_value_ref_parser);
    CASE(planet_size_value_ref_parser);
    CASE(planet_type_value_ref_parser);
    CASE(planet_environment_value_ref_parser);
    CASE(star_type_value_ref_parser);
    CASE(condition_parser);
    CASE(effect_parser);
#undef CASE

    if (test == unknown) {
        print_help();
        exit(1);
    }

    std::string str;
    if (4 <= argc && argc <= 5) {
        if (std::string(argv[2]) != "-f") {
            print_help();
            exit(1);
        }
        std::ifstream ifs(argv[3]);
        while (ifs) {
            str += ifs.get();
        }
        // Get rid of terminating FF char.
        if (!str.empty())
            str.resize(str.size() - 1);
    } else {
        str = argv[2];
    }

    const bool fail = argc == 5 && argv[4] == std::string("--fail");

    const parse::lexer& l = parse::lexer::instance();

    parse::init();

    unsigned int failures = 0;
    unsigned int iterations = 0;
    std::vector<std::string> strings;


    boost::algorithm::split(strings,
                            str,
                            boost::algorithm::is_any_of("\n\r"),
                            boost::algorithm::token_compress_on);

    lexer_test_rules lexer_rules;

    boost::spirit::qi::_1_type _1;
    boost::spirit::qi::_2_type _2;
    boost::spirit::qi::_3_type _3;
    boost::spirit::qi::_4_type _4;

    switch (test) {
    case lexer: boost::spirit::qi::on_error<boost::spirit::qi::fail>(lexer_rules.lexer, parse::report_error(_1, _2, _3, _4)); break;
    case double_value_ref_parser: boost::spirit::qi::on_error<boost::spirit::qi::fail>(parse::value_ref_parser<double>(), parse::report_error(_1, _2, _3, _4)); break;
    case string_value_ref_parser: boost::spirit::qi::on_error<boost::spirit::qi::fail>(parse::value_ref_parser<std::string>(), parse::report_error(_1, _2, _3, _4)); break;
    case planet_size_value_ref_parser: boost::spirit::qi::on_error<boost::spirit::qi::fail>(parse::value_ref_parser<PlanetSize>(), parse::report_error(_1, _2, _3, _4)); break;
    case planet_type_value_ref_parser: boost::spirit::qi::on_error<boost::spirit::qi::fail>(parse::value_ref_parser<PlanetType>(), parse::report_error(_1, _2, _3, _4)); break;
    case planet_environment_value_ref_parser: boost::spirit::qi::on_error<boost::spirit::qi::fail>(parse::value_ref_parser<PlanetEnvironment>(), parse::report_error(_1, _2, _3, _4)); break;
    case star_type_value_ref_parser: boost::spirit::qi::on_error<boost::spirit::qi::fail>(parse::value_ref_parser<StarType>(), parse::report_error(_1, _2, _3, _4)); break;
    case condition_parser: boost::spirit::qi::on_error<boost::spirit::qi::fail>(parse::condition_parser(), parse::report_error(_1, _2, _3, _4)); break;
    case effect_parser: boost::spirit::qi::on_error<boost::spirit::qi::fail>(parse::effect_parser(), parse::report_error(_1, _2, _3, _4)); break;
    default: break;
    }

    for (const std::string& string : strings) {
        if (string.empty())
            continue;

        ++iterations;

        parse::text_iterator first(string.begin());
        const parse::text_iterator last(string.end());

        bool success = false;

        parse::detail::s_text_it = &first;
        parse::detail::s_begin = first;
        parse::detail::s_end = last;
        parse::detail::s_filename = argc == 4 ? argv[3] : "command-line";
        parse::token_iterator it = l.begin(first, last);
        const parse::token_iterator end_it = l.end();

        boost::spirit::qi::in_state_type in_state;

        try {
            switch (test) {
            case lexer: {
                success = boost::spirit::qi::phrase_parse(it, end_it, lexer_rules.lexer, in_state("WS")[l.self]);
                break;
            }
            case double_value_ref_parser: {
                success = boost::spirit::qi::phrase_parse(it, end_it, parse::value_ref_parser<double>(), in_state("WS")[l.self]);
                break;
            }
            case string_value_ref_parser: {
                success = boost::spirit::qi::phrase_parse(it, end_it, parse::value_ref_parser<std::string>(), in_state("WS")[l.self]);
                break;
            }
            case planet_size_value_ref_parser: {
                success = boost::spirit::qi::phrase_parse(it, end_it, parse::value_ref_parser<PlanetSize>(), in_state("WS")[l.self]);
                break;
            }
            case planet_type_value_ref_parser: {
                success = boost::spirit::qi::phrase_parse(it, end_it, parse::value_ref_parser<PlanetType>(), in_state("WS")[l.self]);
                break;
            }
            case planet_environment_value_ref_parser: {
                success = boost::spirit::qi::phrase_parse(it, end_it, parse::value_ref_parser<PlanetEnvironment>(), in_state("WS")[l.self]);
                break;
            }
            case star_type_value_ref_parser: {
                success = boost::spirit::qi::phrase_parse(it, end_it, parse::value_ref_parser<StarType>(), in_state("WS")[l.self]);
                break;
            }
            case condition_parser: {
                success = boost::spirit::qi::phrase_parse(it, end_it, parse::condition_parser(), in_state("WS")[l.self]);
                break;
            }
            case effect_parser: {
                success = boost::spirit::qi::phrase_parse(it, end_it, parse::effect_parser(), in_state("WS")[l.self]);
                break;
            }
            default:
                break;
            }

            if (success && it == end_it) {
                if (fail)
                    std::cout <<  "Successful parse of \"" << string << "\" (that's bad -- it should fail)." << std::endl;
                else
                    std::cout <<  "Successful parse." << std::endl;
            } else {
                if (fail)
                    std::cout <<  "Failed parse, as expected." << std::endl;
                else
                    std::cout <<  "Failed parse of \"" << string << "\"." << std::endl;
                ++failures;
            }
        } catch (const boost::spirit::qi::expectation_failure<parse::token_iterator>&) {
            std::cout <<  "Failed parse of \"" << string << "\" (qi::expectation_failure<> exception)." << std::endl;
            ++failures;
        }
    }

    if(test == lexer && !lexer_test_rules::unchecked_tokens.empty())
    {
        std::cout << "There were unhandled tokens: " << std::endl;
        for (const char* token : lexer_test_rules::unchecked_tokens) {
            std::cout << token << std::endl;
            ++failures;
        }
    }

    if (1u < strings.size()) {
        if (fail) {
            if (failures != iterations)
                std::cout << (strings.size() - failures) << " successful parses total (that's bad -- all should fail)." << std::endl;
            else
                std::cout << "All parses failed, as expected." << std::endl;
        } else {
            if (failures)
                std::cout << failures << " failures total." << std::endl;
            else
                std::cout << "All parses successful." << std::endl;
        }
    }

    return fail ? failures != iterations : failures;
}
