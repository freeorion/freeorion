#include "Parse.h"

#include "ParseImpl.h"
#include "EnumParser.h"
#include "ConditionParserImpl.h"
#include "ValueRefParser.h"
#include "CommonParams.h"

#include "../universe/Building.h"
#include "../universe/Enums.h"


#define DEBUG_PARSERS 0

#if DEBUG_PARSERS
namespace std {
    inline ostream& operator<<(ostream& os, const std::vector<std::shared_ptr<Effect::EffectsGroup>>&) { return os; }
    inline ostream& operator<<(ostream& os, const std::map<std::string, std::unique_ptr<BuildingType>>&) { return os; }
    inline ostream& operator<<(ostream& os, const std::pair<const std::string, std::unique_ptr<BuildingType>>&) { return os; }
}
#endif

namespace {
    const boost::phoenix::function<parse::detail::is_unique> is_unique_;

    void insert_building(std::map<std::string, std::unique_ptr<BuildingType>>& building_types,
                         const std::string& name,
                         const std::string& description,
                         const CommonParams& common_params,
                         CaptureResult& capture_result,
                         const std::string& icon)
    {
        // TODO use make_unique when converting to C++14
        auto building_type = std::unique_ptr<BuildingType>(
            new BuildingType(name, description, common_params, capture_result, icon));

        building_types.insert(std::make_pair(building_type->Name(), std::move(building_type)));
    }


    BOOST_PHOENIX_ADAPT_FUNCTION(void, insert_building_, insert_building, 6)

    struct rules {
        rules(const std::string& filename,
              const parse::text_iterator& first, const parse::text_iterator& last)
        {
            namespace phoenix = boost::phoenix;
            namespace qi = boost::spirit::qi;

            qi::_1_type _1;
            qi::_2_type _2;
            qi::_3_type _3;
            qi::_4_type _4;
            qi::_a_type _a;
            qi::_b_type _b;
            qi::_c_type _c;
            qi::_d_type _d;
            qi::_pass_type _pass;
            qi::_r1_type _r1;
            qi::eps_type eps;

            const parse::lexer& tok = parse::lexer::instance();

            building_type
                =   tok.BuildingType_
                >   parse::detail::label(Name_token)
                >   tok.string        [ _pass = is_unique_(_r1, BuildingType_token, _1), _a = _1 ]
                >   parse::detail::label(Description_token)         > tok.string        [ _b = _1 ]
                >   (   parse::detail::label(CaptureResult_token)   >> parse::capture_result_enum() [ _d = _1 ]
                    |   eps [ _d = CR_CAPTURE ]
                    )
                >   parse::detail::common_params_parser() [ _c = _1 ]
                >   parse::detail::label(Icon_token)      > tok.string
                [ insert_building_(_r1, _a, _b, _c, _d, _1) ]
                ;

            start
                =   +building_type(_r1)
                ;

            building_type.name("BuildingType");

#if DEBUG_PARSERS
            debug(building_type);
#endif

            qi::on_error<qi::fail>(start, parse::report_error(filename, first, last, _1, _2, _3, _4));
        }

        typedef parse::detail::rule<
            void (std::map<std::string, std::unique_ptr<BuildingType>>&),
            boost::spirit::qi::locals<
                std::string,
                std::string,
                CommonParams,
                CaptureResult
            >
        > building_type_rule;

        typedef parse::detail::rule<
            void (std::map<std::string, std::unique_ptr<BuildingType>>&)
        > start_rule;

        building_type_rule          building_type;
        start_rule                  start;
    };
}

namespace parse {
    std::map<std::string, std::unique_ptr<BuildingType>> buildings() {
        std::map<std::string, std::unique_ptr<BuildingType>> building_types;
        for (const boost::filesystem::path& file : ListScripts("scripting/buildings")) {
            /*auto success =*/ detail::parse_file<rules, std::map<std::string, std::unique_ptr<BuildingType>>>(file, building_types);
        }

        return building_types;
    }
}
