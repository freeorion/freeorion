#include "Parse.h"

#include "ParseImpl.h"

#include "../universe/ShipDesign.h"

#include <boost/spirit/include/phoenix.hpp>


FO_COMMON_API extern const int ALL_EMPIRES;

#define DEBUG_PARSERS 0

#if DEBUG_PARSERS
namespace std {
    inline ostream& operator<<(ostream& os, const std::map<std::string, ShipDesign*>&) { return os; }
    inline ostream& operator<<(ostream& os, const std::pair<const std::string, ShipDesign*>&) { return os; }
    inline ostream& operator<<(ostream& os, const std::vector<std::string>&) { return os; }
}
#endif

namespace {
    const boost::phoenix::function<parse::detail::is_unique> is_unique_;

    const boost::phoenix::function<parse::detail::insert> insert_;

    struct rules {
        rules() {
            namespace phoenix = boost::phoenix;
            namespace qi = boost::spirit::qi;

            using phoenix::new_;
            using phoenix::push_back;

            qi::_1_type _1;
            qi::_2_type _2;
            qi::_3_type _3;
            qi::_4_type _4;
            qi::_a_type _a;
            qi::_b_type _b;
            qi::_c_type _c;
            qi::_d_type _d;
            qi::_e_type _e;
            qi::_f_type _f;
            qi::_pass_type _pass;
            qi::_r1_type _r1;
            qi::_r2_type _r2;
            qi::_r3_type _r3;
            qi::_r4_type _r4;
            qi::_r5_type _r5;
            qi::eps_type eps;

            const parse::lexer& tok = parse::lexer::instance();

            design_prefix
                =    tok.ShipDesign_
                >    parse::detail::label(Name_token)
                >   tok.string        [ _pass = is_unique_(_r5, ShipDesign_token, _1), _r1 = _1 ]
                >    parse::detail::label(Description_token) > tok.string [ _r2 = _1 ]
                > (
                     tok.NoStringtableLookup_ [ _r4 = false ]
                    | eps [ _r4 = true ]
                  )
                >    parse::detail::label(Hull_token)        > tok.string [ _r3 = _1 ]
                ;

            design
                =    design_prefix(_a, _b, _c, _f, _r1)
                >    parse::detail::label(Parts_token)
                >    (
                            ('[' > +tok.string [ push_back(_d, _1) ] > ']')
                        |    tok.string [ push_back(_d, _1) ]
                     )
                >   -(
                        parse::detail::label(Icon_token)     > tok.string [ _e = _1 ]
                     )
                >    parse::detail::label(Model_token)       > tok.string
                [ insert_(_r1, _a, new_<ShipDesign>(_a, _b, 0, ALL_EMPIRES, _c, _d, _e, _1, _f)) ]
                ;

            start
                =   +design(_r1)
                ;

            design_prefix.name("Name, Description, Lookup Flag, Hull");
            design.name("ShipDesign");

#if DEBUG_PARSERS
            debug(design_prefix);
            debug(design);
#endif

            qi::on_error<qi::fail>(start, parse::report_error(_1, _2, _3, _4));
        }

        typedef parse::detail::rule<
            void (std::string&, std::string&, std::string&, bool&, const std::map<std::string, ShipDesign*>&)
        > design_prefix_rule;

        typedef parse::detail::rule<
            void (std::map<std::string, ShipDesign*>&),
            boost::spirit::qi::locals<
                std::string,
                std::string,
                std::string,
                std::vector<std::string>,
                std::string,
                bool
            >
        > design_rule;

        typedef parse::detail::rule<
            void (std::map<std::string, ShipDesign*>&)
        > start_rule;

        design_prefix_rule design_prefix;
        design_rule design;
        start_rule start;
    };
}

namespace parse {
    bool ship_designs(const boost::filesystem::path& path, std::map<std::string, ShipDesign*>& designs)
    { return detail::parse_file<rules, std::map<std::string, ShipDesign*>>(path, designs); }

    bool ship_designs(std::map<std::string, ShipDesign*>& designs) {
        bool result = true;

        for(const boost::filesystem::path& file : ListScripts("scripting/ship_designs")) {
            result &= detail::parse_file<rules, std::map<std::string, ShipDesign*>>(file, designs);
        }

        return result;
    }

    bool monster_designs(std::map<std::string, ShipDesign*>& designs) {
        bool result = true;

        for (const boost::filesystem::path& file : ListScripts("scripting/monster_designs")) {
            result &= detail::parse_file<rules, std::map<std::string, ShipDesign*>>(file, designs);
        }

        return result;
    }
}
