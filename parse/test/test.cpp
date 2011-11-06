#include "test.h"

#include "../ConditionParser.h"
#include "../EffectParser.h"
#include "../EnumParser.h"
#include "../Parse.h"
#include "../ReportParseError.h"
#include "../Empire/Empire.h"
#include "../universe/ValueRef.h"

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/filesystem/fstream.hpp>

#include <fstream>


void print_help()
{
    std::cout << "Usage: test lexer|planet_size_enum_parser|planet_type_enum_parser|planet_environment_enum_parser|universe_object_type_enum_parser|star_type_enum_parser|non_ship_part_meter_type_enum_parser|set_non_ship_part_meter_type_enum_parser|set_ship_part_meter_type_enum_parser|empire_affiliation_type_enum_parser|unlockable_item_type_enum_parser|tech_type_enum_parser|ship_slot_type_enum_parser|ship_part_class_enum_parser|combat_fighter_type_enum_parser|capture_result_enum_parser|value_ref_statistic_type_enum_parser|int_value_ref_parser|double_value_ref_parser|string_value_ref_parser|planet_size_value_ref_parser|planet_type_value_ref_parser|planet_environment_value_ref_parser|universe_object_type_value_ref_parser|star_type_value_ref_parser|int_value_ref_evaluation|double_value_ref_evaluation|condition_parser|effect_parser|buildings_parser|specials_parser|species_parser|techs_parser|items_parser|ship_parts_parser|ship_hulls_parser|ship_designs_parser|fleet_plans_parser|monster_fleet_plans_parser|alignments_parser <-f filename>|<test string> --fail" << std::endl;
}

int main(int argc, char* argv[])
{
    if (argc < 3) {
        print_help();
        exit(1);
    }

    const std::string test_str = argv[1];
    test_type test = unknown;
#define CASE(x) if (test_str == #x) test = x
    CASE(lexer);
    CASE(planet_size_enum_parser);
    CASE(planet_type_enum_parser);
    CASE(planet_environment_enum_parser);
    CASE(universe_object_type_enum_parser);
    CASE(star_type_enum_parser);
    CASE(non_ship_part_meter_type_enum_parser);
    CASE(set_non_ship_part_meter_type_enum_parser);
    CASE(set_ship_part_meter_type_enum_parser);
    CASE(empire_affiliation_type_enum_parser);
    CASE(unlockable_item_type_enum_parser);
    CASE(tech_type_enum_parser);
    CASE(ship_slot_type_enum_parser);
    CASE(ship_part_class_enum_parser);
    CASE(combat_fighter_type_enum_parser);
    CASE(capture_result_enum_parser);
    CASE(value_ref_statistic_type_enum_parser);
    CASE(int_value_ref_parser);
    CASE(double_value_ref_parser);
    CASE(string_value_ref_parser);
    CASE(planet_size_value_ref_parser);
    CASE(planet_type_value_ref_parser);
    CASE(planet_environment_value_ref_parser);
    CASE(universe_object_type_value_ref_parser);
    CASE(star_type_value_ref_parser);
    CASE(int_value_ref_evaluation);
    CASE(double_value_ref_evaluation);
    CASE(condition_parser);
    CASE(effect_parser);
    CASE(buildings_parser);
    CASE(specials_parser);
    CASE(species_parser);
    CASE(techs_parser);
    CASE(items_parser);
    CASE(ship_parts_parser);
    CASE(ship_hulls_parser);
    CASE(ship_designs_parser);
    CASE(fleet_plans_parser);
    CASE(monster_fleet_plans_parser);
    CASE(alignments_parser);
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

    if (!fail && buildings_parser <= test && test <= alignments_parser) {
        assert(std::string(argv[2]) == "-f");
        bool success = false;
        try {
            switch (test) {
            case buildings_parser: {
                std::map<std::string, BuildingType*> building_types;
                success = parse::buildings(boost::filesystem::path(argv[3]), building_types);
                break;
            }
            case specials_parser: {
                std::map<std::string, Special*> specials;
                success = parse::specials(boost::filesystem::path(argv[3]), specials);
                break;
            }
            case species_parser: {
                std::map<std::string, Species*> species;
                success = parse::species(boost::filesystem::path(argv[3]), species);
                break;
            }
            case techs_parser: {
                TechManager::TechContainer techs;
                std::map<std::string, TechCategory*> tech_categories;
                std::set<std::string> categories_seen_in_techs;
                success = parse::techs(boost::filesystem::path(argv[3]), techs, tech_categories, categories_seen_in_techs);
                break;
            }
            case items_parser: {
                std::vector<ItemSpec> items;
                success = parse::items(boost::filesystem::path(argv[3]), items);
                break;
            }
            case ship_parts_parser: {
                std::map<std::string, PartType*> parts;
                success = parse::ship_parts(boost::filesystem::path(argv[3]), parts);
                break;
            }
            case ship_hulls_parser: {
                std::map<std::string, HullType*> hulls;
                success = parse::ship_hulls(boost::filesystem::path(argv[3]), hulls);
                break;
            }
            case ship_designs_parser: {
                std::map<std::string, ShipDesign*> designs;
                success = parse::ship_designs(boost::filesystem::path(argv[3]), designs);
                break;
            }
            case fleet_plans_parser: {
                std::vector<FleetPlan*> fleet_plans;
                success = parse::fleet_plans(boost::filesystem::path(argv[3]), fleet_plans);
                break;
            }
            case monster_fleet_plans_parser: {
                std::vector<MonsterFleetPlan*> monster_fleet_plans;
                success = parse::monster_fleet_plans(boost::filesystem::path(argv[3]), monster_fleet_plans);
                break;
            }
            case alignments_parser: {
                std::vector<Alignment> alignments;
                std::vector<boost::shared_ptr<const Effect::EffectsGroup> > effects_groups;
                success = parse::alignments(boost::filesystem::path(argv[3]), alignments, effects_groups);
                break;
            }
            default:
                break;
            }

            if (success) {
                std::cout <<  "Successful parse." << std::endl;
            } else {
                std::cout <<  "Failed parse of \"" << argv[3] << "\"." << std::endl;
                ++failures;
            }
        } catch (const boost::spirit::qi::expectation_failure<parse::token_iterator>&) {
            std::cout <<  "Failed parse of \"" << argv[3] << "\" (qi::expectation_failure<> exception)." << std::endl;
            ++failures;
        }
    } else {
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
        case planet_size_enum_parser: boost::spirit::qi::on_error<boost::spirit::qi::fail>(parse::enum_parser<PlanetSize>(), parse::report_error(_1, _2, _3, _4)); break;
        case planet_type_enum_parser: boost::spirit::qi::on_error<boost::spirit::qi::fail>(parse::enum_parser<PlanetType>(), parse::report_error(_1, _2, _3, _4)); break;
        case planet_environment_enum_parser: boost::spirit::qi::on_error<boost::spirit::qi::fail>(parse::enum_parser<PlanetEnvironment>(), parse::report_error(_1, _2, _3, _4)); break;
        case universe_object_type_enum_parser: boost::spirit::qi::on_error<boost::spirit::qi::fail>(parse::enum_parser<UniverseObjectType>(), parse::report_error(_1, _2, _3, _4)); break;
        case star_type_enum_parser: boost::spirit::qi::on_error<boost::spirit::qi::fail>(parse::enum_parser<StarType>(), parse::report_error(_1, _2, _3, _4)); break;
        case non_ship_part_meter_type_enum_parser: boost::spirit::qi::on_error<boost::spirit::qi::fail>(parse::non_ship_part_meter_type_enum(), parse::report_error(_1, _2, _3, _4)); break;
        case set_non_ship_part_meter_type_enum_parser: boost::spirit::qi::on_error<boost::spirit::qi::fail>(parse::set_non_ship_part_meter_type_enum(), parse::report_error(_1, _2, _3, _4)); break;
        case set_ship_part_meter_type_enum_parser: boost::spirit::qi::on_error<boost::spirit::qi::fail>(parse::set_ship_part_meter_type_enum(), parse::report_error(_1, _2, _3, _4)); break;
        case empire_affiliation_type_enum_parser: boost::spirit::qi::on_error<boost::spirit::qi::fail>(parse::enum_parser<UnlockableItemType>(), parse::report_error(_1, _2, _3, _4)); break;
        case unlockable_item_type_enum_parser: boost::spirit::qi::on_error<boost::spirit::qi::fail>(parse::enum_parser<UnlockableItemType>(), parse::report_error(_1, _2, _3, _4)); break;
        case tech_type_enum_parser: boost::spirit::qi::on_error<boost::spirit::qi::fail>(parse::enum_parser<TechType>(), parse::report_error(_1, _2, _3, _4)); break;
        case ship_slot_type_enum_parser: boost::spirit::qi::on_error<boost::spirit::qi::fail>(parse::enum_parser<ShipSlotType>(), parse::report_error(_1, _2, _3, _4)); break;
        case ship_part_class_enum_parser: boost::spirit::qi::on_error<boost::spirit::qi::fail>(parse::enum_parser<ShipPartClass>(), parse::report_error(_1, _2, _3, _4)); break;
        case combat_fighter_type_enum_parser: boost::spirit::qi::on_error<boost::spirit::qi::fail>(parse::enum_parser<CombatFighterType>(), parse::report_error(_1, _2, _3, _4)); break;
        case capture_result_enum_parser: boost::spirit::qi::on_error<boost::spirit::qi::fail>(parse::enum_parser<CaptureResult>(), parse::report_error(_1, _2, _3, _4)); break;
        case value_ref_statistic_type_enum_parser: boost::spirit::qi::on_error<boost::spirit::qi::fail>(parse::enum_parser<ValueRef::StatisticType>(), parse::report_error(_1, _2, _3, _4)); break;
        case int_value_ref_parser: boost::spirit::qi::on_error<boost::spirit::qi::fail>(parse::value_ref_parser<int>(), parse::report_error(_1, _2, _3, _4)); break;
        case double_value_ref_parser: boost::spirit::qi::on_error<boost::spirit::qi::fail>(parse::value_ref_parser<double>(), parse::report_error(_1, _2, _3, _4)); break;
        case string_value_ref_parser: boost::spirit::qi::on_error<boost::spirit::qi::fail>(parse::value_ref_parser<std::string>(), parse::report_error(_1, _2, _3, _4)); break;
        case planet_size_value_ref_parser: boost::spirit::qi::on_error<boost::spirit::qi::fail>(parse::value_ref_parser<PlanetSize>(), parse::report_error(_1, _2, _3, _4)); break;
        case planet_type_value_ref_parser: boost::spirit::qi::on_error<boost::spirit::qi::fail>(parse::value_ref_parser<PlanetType>(), parse::report_error(_1, _2, _3, _4)); break;
        case planet_environment_value_ref_parser: boost::spirit::qi::on_error<boost::spirit::qi::fail>(parse::value_ref_parser<PlanetEnvironment>(), parse::report_error(_1, _2, _3, _4)); break;
        case universe_object_type_value_ref_parser: boost::spirit::qi::on_error<boost::spirit::qi::fail>(parse::value_ref_parser<UniverseObjectType>(), parse::report_error(_1, _2, _3, _4)); break;
        case star_type_value_ref_parser: boost::spirit::qi::on_error<boost::spirit::qi::fail>(parse::value_ref_parser<StarType>(), parse::report_error(_1, _2, _3, _4)); break;
        case int_value_ref_evaluation: boost::spirit::qi::on_error<boost::spirit::qi::fail>(parse::value_ref_parser<int>(), parse::report_error(_1, _2, _3, _4)); break;
        case double_value_ref_evaluation: boost::spirit::qi::on_error<boost::spirit::qi::fail>(parse::value_ref_parser<double>(), parse::report_error(_1, _2, _3, _4)); break;
        case condition_parser: boost::spirit::qi::on_error<boost::spirit::qi::fail>(parse::condition_parser(), parse::report_error(_1, _2, _3, _4)); break;
        case effect_parser: boost::spirit::qi::on_error<boost::spirit::qi::fail>(parse::effect_parser(), parse::report_error(_1, _2, _3, _4)); break;
        default: break;
        }

        for (std::size_t i = 0; i < strings.size(); ++i) {
            const std::string& string = strings[i];
            if (string.empty())
                continue;

            ++iterations;

            const std::size_t equals_position =
                test == int_value_ref_evaluation || test == double_value_ref_evaluation ?
                string.find('=') :
                std::string::npos;

            parse::text_iterator first(string.begin());
            const parse::text_iterator last(equals_position == std::string::npos ? string.end() : string.begin() + equals_position);

            bool success = false;

            parse::detail::s_text_it = &first;
            parse::detail::s_begin = first;
            parse::detail::s_end = last;
            parse::detail::s_filename = argc == 4 ? argv[3] : "command-line";
            parse::token_iterator it = l.begin(first, last);
            const parse::token_iterator end_it = l.end();

            boost::spirit::qi::in_state_type in_state;

            boost::filesystem::path file_parser_path;
            if (buildings_parser <= test && test <= alignments_parser) {
                file_parser_path = "tmp";
                boost::filesystem::ofstream ofs(file_parser_path);
                ofs << string;
            }

            try {
                switch (test) {
                case lexer: {
                    success = boost::spirit::qi::phrase_parse(it, end_it, lexer_rules.lexer, in_state("WS")[l.self]);
                    break;
                }
                case planet_size_enum_parser: {
                    success = boost::spirit::qi::phrase_parse(it, end_it, parse::enum_parser<PlanetSize>(), in_state("WS")[l.self]);
                    break;
                }
                case planet_type_enum_parser: {
                    success = boost::spirit::qi::phrase_parse(it, end_it, parse::enum_parser<PlanetType>(), in_state("WS")[l.self]);
                    break;
                }
                case planet_environment_enum_parser: {
                    success = boost::spirit::qi::phrase_parse(it, end_it, parse::enum_parser<PlanetEnvironment>(), in_state("WS")[l.self]);
                    break;
                }
                case universe_object_type_enum_parser: {
                    success = boost::spirit::qi::phrase_parse(it, end_it, parse::enum_parser<UniverseObjectType>(), in_state("WS")[l.self]);
                    break;
                }
                case star_type_enum_parser: {
                    success = boost::spirit::qi::phrase_parse(it, end_it, parse::enum_parser<StarType>(), in_state("WS")[l.self]);
                    break;
                }
                case non_ship_part_meter_type_enum_parser: {
                    success = boost::spirit::qi::phrase_parse(it, end_it, parse::non_ship_part_meter_type_enum(), in_state("WS")[l.self]);
                    break;
                }
                case set_non_ship_part_meter_type_enum_parser: {
                    success = boost::spirit::qi::phrase_parse(it, end_it, parse::set_non_ship_part_meter_type_enum(), in_state("WS")[l.self]);
                    break;
                }
                case set_ship_part_meter_type_enum_parser: {
                    success = boost::spirit::qi::phrase_parse(it, end_it, parse::set_ship_part_meter_type_enum(), in_state("WS")[l.self]);
                    break;
                }
                case empire_affiliation_type_enum_parser: {
                    success = boost::spirit::qi::phrase_parse(it, end_it, parse::enum_parser<EmpireAffiliationType>(), in_state("WS")[l.self]);
                    break;
                }
                case unlockable_item_type_enum_parser: {
                    success = boost::spirit::qi::phrase_parse(it, end_it, parse::enum_parser<UnlockableItemType>(), in_state("WS")[l.self]);
                    break;
                }
                case tech_type_enum_parser: {
                    success = boost::spirit::qi::phrase_parse(it, end_it, parse::enum_parser<TechType>(), in_state("WS")[l.self]);
                    break;
                }
                case ship_slot_type_enum_parser: {
                    success = boost::spirit::qi::phrase_parse(it, end_it, parse::enum_parser<ShipSlotType>(), in_state("WS")[l.self]);
                    break;
                }
                case ship_part_class_enum_parser: {
                    success = boost::spirit::qi::phrase_parse(it, end_it, parse::enum_parser<ShipPartClass>(), in_state("WS")[l.self]);
                    break;
                }
                case combat_fighter_type_enum_parser: {
                    success = boost::spirit::qi::phrase_parse(it, end_it, parse::enum_parser<CombatFighterType>(), in_state("WS")[l.self]);
                    break;
                }
                case capture_result_enum_parser: {
                    success = boost::spirit::qi::phrase_parse(it, end_it, parse::enum_parser<CaptureResult>(), in_state("WS")[l.self]);
                    break;
                }
                case value_ref_statistic_type_enum_parser: {
                    success = boost::spirit::qi::phrase_parse(it, end_it, parse::enum_parser<ValueRef::StatisticType>(), in_state("WS")[l.self]);
                    break;
                }
                case int_value_ref_parser: {
                    success = boost::spirit::qi::phrase_parse(it, end_it, parse::value_ref_parser<int>(), in_state("WS")[l.self]);
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
                case universe_object_type_value_ref_parser: {
                    success = boost::spirit::qi::phrase_parse(it, end_it, parse::value_ref_parser<UniverseObjectType>(), in_state("WS")[l.self]);
                    break;
                }
                case star_type_value_ref_parser: {
                    success = boost::spirit::qi::phrase_parse(it, end_it, parse::value_ref_parser<StarType>(), in_state("WS")[l.self]);
                    break;
                }
                case int_value_ref_evaluation: {
                    ValueRef::ValueRefBase<int>* result = 0;
                    success = boost::spirit::qi::phrase_parse(it, end_it, parse::value_ref_parser<int>(), in_state("WS")[l.self], result);
                    assert(equals_position != std::string::npos);
                    const int expected_result = boost::lexical_cast<int>(string.substr(equals_position + 1, -1));
                    if (result->Eval() == expected_result) {
                        std::cout <<  "Correct evaluation." << std::endl;
                    } else {
                        std::cout <<  "Incorrect evaluation of \"" << string << "\".  Got " << result->Eval() << "." << std::endl;
                        ++failures;
                    }
                    break;
                }
                case double_value_ref_evaluation: {
                    ValueRef::ValueRefBase<double>* result = 0;
                    success = boost::spirit::qi::phrase_parse(it, end_it, parse::value_ref_parser<double>(), in_state("WS")[l.self], result);
                    assert(equals_position != std::string::npos);
                    const double expected_result = boost::lexical_cast<double>(string.substr(equals_position + 1, -1));
                    if (result->Eval() == expected_result) {
                        std::cout <<  "Correct evaluation." << std::endl;
                    } else {
                        std::cout <<  "Incorrect evaluation of \"" << string << "\".  Got " << result->Eval() << "." << std::endl;
                        ++failures;
                    }
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
                case buildings_parser: {
                    std::map<std::string, BuildingType*> building_types;
                    success = parse::buildings(file_parser_path, building_types);
                    break;
                }
                case specials_parser: {
                    std::map<std::string, Special*> specials;
                    success = parse::specials(file_parser_path, specials);
                    break;
                }
                case species_parser: {
                    std::map<std::string, Species*> species;
                    success = parse::species(file_parser_path, species);
                    break;
                }
                case techs_parser: {
                    TechManager::TechContainer techs;
                    std::map<std::string, TechCategory*> tech_categories;
                    std::set<std::string> categories_seen_in_techs;
                    success = parse::techs(file_parser_path, techs, tech_categories, categories_seen_in_techs);
                    break;
                }
                case items_parser: {
                    std::vector<ItemSpec> items;
                    success = parse::items(file_parser_path, items);
                    break;
                }
                case ship_parts_parser: {
                    std::map<std::string, PartType*> parts;
                    success = parse::ship_parts(file_parser_path, parts);
                    break;
                }
                case ship_hulls_parser: {
                    std::map<std::string, HullType*> hulls;
                    success = parse::ship_hulls(file_parser_path, hulls);
                    break;
                }
                case ship_designs_parser: {
                    std::map<std::string, ShipDesign*> designs;
                    success = parse::ship_designs(file_parser_path, designs);
                    break;
                }
                case fleet_plans_parser: {
                    std::vector<FleetPlan*> fleet_plans;
                    success = parse::fleet_plans(file_parser_path, fleet_plans);
                    break;
                }
                case monster_fleet_plans_parser: {
                    std::vector<MonsterFleetPlan*> monster_fleet_plans;
                    success = parse::monster_fleet_plans(file_parser_path, monster_fleet_plans);
                    break;
                }
                case alignments_parser: {
                    std::vector<Alignment> alignments;
                    std::vector<boost::shared_ptr<const Effect::EffectsGroup> > effects_groups;
                    success = parse::alignments(file_parser_path, alignments, effects_groups);
                    break;
                }
                default:
                    break;
                }

                if (success && it == end_it) {
                    if (test != int_value_ref_evaluation && test != double_value_ref_evaluation) {
                        if (fail)
                            std::cout <<  "Successful parse of \"" << string << "\" (that's bad -- it should fail)." << std::endl;
                        else
                            std::cout <<  "Successful parse." << std::endl;
                    }
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
