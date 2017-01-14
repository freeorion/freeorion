#include "Label.h"
#include "Parse.h"
#include "ParseImpl.h"

#include "../universe/ShipDesign.h"

#include <boost/spirit/include/phoenix.hpp>

#define DEBUG_PARSERS 0

#if DEBUG_PARSERS
namespace std {
    inline ostream& operator<<(ostream& os, const std::map<std::string, ShipDesign*>&) { return os; }
    inline ostream& operator<<(ostream& os, const std::pair<const std::string, ShipDesign*>&) { return os; }
    inline ostream& operator<<(ostream& os, const std::vector<std::string>&) { return os; }
}
#endif

namespace {
    struct insert_ {
        typedef void result_type;

        void operator()(std::map<std::string, ShipDesign*>& designs, ShipDesign* design) const {
            if (!designs.insert(std::make_pair(design->Name(false), design)).second) {
                std::string error_str = "ERROR: More than one predefined ship design in predefined_ship_designs.txt has the name " + design->Name(false);
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
            qi::_e_type _e;
            qi::_f_type _f;
            qi::_r1_type _r1;
            qi::_r2_type _r2;
            qi::_r3_type _r3;
            qi::_r4_type _r4;
            qi::eps_type eps;
            using phoenix::new_;
            using phoenix::push_back;

            design_prefix
                =    tok.ShipDesign_
                >    parse::label(Name_token)        > tok.string [ _r1 = _1 ]
                >    parse::label(Description_token) > tok.string [ _r2 = _1 ]
                > (
                     tok.NoStringtableLookup_ [ _r4 = false ]
                    | eps [ _r4 = true ]
                  )
                >    parse::label(Hull_token)        > tok.string [ _r3 = _1 ]
                ;

            design
                =    design_prefix(_a, _b, _c, _f)
                >    parse::label(Parts_token)
                >    (
                            ('[' > +tok.string [ push_back(_d, _1) ] > ']')
                        |    tok.string [ push_back(_d, _1) ]
                     )
                >   -(
                        parse::label(Icon_token)     > tok.string [ _e = _1 ]
                     )
                >    parse::label(Model_token)       > tok.string
                [ insert(_r1, new_<ShipDesign>(_a, _b, 0, ALL_EMPIRES, _c, _d, _e, _1, _f)) ]
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

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            void (std::string&, std::string&, std::string&, bool&),
            parse::skipper_type
        > design_prefix_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            void (std::map<std::string, ShipDesign*>&),
            qi::locals<
                std::string,
                std::string,
                std::string,
                std::vector<std::string>,
                std::string,
                bool
            >,
            parse::skipper_type
        > design_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            void (std::map<std::string, ShipDesign*>&),
            parse::skipper_type
        > start_rule;

        design_prefix_rule design_prefix;
        design_rule design;
        start_rule start;
    };
}

namespace parse {
    bool ship_designs(const boost::filesystem::path& path, std::map<std::string, ShipDesign*>& designs)
    { return detail::parse_file<rules, std::map<std::string, ShipDesign*> >(path, designs); }

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
            result &= detail::parse_file<rules, std::map<std::string, ShipDesign*> >(file, designs);
        }

        return result;
    }
}
