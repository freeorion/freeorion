#include "ConditionParserImpl.h"
#include "Double.h"
#include "EnumParser.h"
#include "Int.h"
#include "Label.h"
#include "Parse.h"
#include "ParseImpl.h"
#include "ValueRefParser.h"
#include "CommonParams.h"

#include "../universe/Building.h"

#define DEBUG_PARSERS 0

#if DEBUG_PARSERS
namespace std {
    inline ostream& operator<<(ostream& os, const std::vector<boost::shared_ptr<Effect::EffectsGroup> >&) { return os; }
    inline ostream& operator<<(ostream& os, const std::map<std::string, BuildingType*>&) { return os; }
    inline ostream& operator<<(ostream& os, const std::pair<const std::string, BuildingType*>&) { return os; }
}
#endif

namespace {
    struct insert_ {
        typedef void result_type;

        void operator()(std::map<std::string, BuildingType*>& building_types, BuildingType* building_type) const {
            if (!building_types.insert(std::make_pair(building_type->Name(), building_type)).second) {
                std::string error_str = "ERROR: More than one building type in buildings.txt has the name " + building_type->Name();
                throw std::runtime_error(error_str.c_str());
            }
        }
    };
    const boost::phoenix::function<insert_> insert;

    struct rules {
        rules() {
            const parse::lexer& tok = parse::lexer::instance();

            qi::_1_type _1;
            qi::_2_type _2;
            qi::_3_type _3;
            qi::_4_type _4;
            qi::_a_type _a;
            qi::_b_type _b;
            qi::_c_type _c;
            qi::_d_type _d;
            qi::_r1_type _r1;
            qi::eps_type eps;
            using phoenix::new_;

            building_type
                =   tok.BuildingType_
                >   parse::label(Name_token)                > tok.string        [ _a = _1 ]
                >   parse::label(Description_token)         > tok.string        [ _b = _1 ]
                >   (   parse::label(CaptureResult_token)   >> parse::enum_parser<CaptureResult>() [ _d = _1 ]
                    |   eps [ _d = CR_CAPTURE ]
                    )
                >   parse::detail::common_params_parser() [ _c = _1 ]
                >   parse::label(Icon_token)               > tok.string
                [ insert(_r1, new_<BuildingType>(_a, _b, _c, _d, _1)) ]
                ;

            start
                =   +building_type(_r1)
                ;

            building_type.name("BuildingType");

#if DEBUG_PARSERS
            debug(building_type);
#endif

            qi::on_error<qi::fail>(start, parse::report_error(_1, _2, _3, _4));
        }

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            void (std::map<std::string, BuildingType*>&),
            qi::locals<
                std::string,
                std::string,
                CommonParams,
                CaptureResult
            >,
            parse::skipper_type
        > building_type_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            void (std::map<std::string, BuildingType*>&),
            parse::skipper_type
        > start_rule;

        building_type_rule          building_type;
        start_rule                  start;
    };
}

namespace parse {
    bool buildings(std::map<std::string, BuildingType*>& building_types) {
        bool result = true;

        for (const boost::filesystem::path& file : ListScripts("scripting/buildings")) {
            result &= detail::parse_file<rules, std::map<std::string, BuildingType*>>(file, building_types);
        }

        return result;
    }
}
