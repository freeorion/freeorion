#include "Parse.h"

#include "ParseImpl.h"

#include "../universe/ShipDesign.h"

#include <boost/spirit/include/phoenix.hpp>
#include <boost/phoenix/function/adapt_function.hpp>

FO_COMMON_API extern const int ALL_EMPIRES;

#define DEBUG_PARSERS 0

#if DEBUG_PARSERS
namespace std {
    inline ostream& operator<<(ostream& os, const std::map<std::string, std::unique_ptr<ShipDesign>>&) { return os; }
    inline ostream& operator<<(ostream& os, const std::pair<const std::string, std::unique_ptr<ShipDesign>>&) { return os; }
    inline ostream& operator<<(ostream& os, const std::vector<std::string>&) { return os; }
}
#endif

namespace {
    const boost::phoenix::function<parse::detail::is_unique> is_unique_;

    void insert_ship_design(std::map<std::string, std::unique_ptr<ShipDesign>>& designs,
                            const std::string& name, const std::string& description,
                            const std::string& hull, const std::vector<std::string>& parts,
                            const std::string& icon, const std::string& model,
                            bool name_desc_in_stringtable, const std::string& uuid)
    {
        // TODO use make_unique when converting to C++14
        auto design = std::unique_ptr<ShipDesign>(
            new ShipDesign(name, description, 0, ALL_EMPIRES, hull, parts, icon, model, name_desc_in_stringtable, false, uuid));

        auto inserted_design = designs.insert(std::make_pair(design->Name(false), std::move(design)));
    };
    BOOST_PHOENIX_ADAPT_FUNCTION(void, insert_ship_design_, insert_ship_design, 9)

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
            qi::_g_type _g;
            qi::_pass_type _pass;
            qi::_r1_type _r1;
            qi::_r2_type _r2;
            qi::_r3_type _r3;
            qi::_r4_type _r4;
            qi::_r5_type _r5;
            qi::_r6_type _r6;
            qi::eps_type eps;

            const parse::lexer& tok = parse::lexer::instance();

            design_prefix
                =    tok.ShipDesign_
                >    parse::detail::label(Name_token)
                >    tok.string        [ _pass = is_unique_(_r6, ShipDesign_token, _1), _r1 = _1 ]
                >    (parse::detail::label(UUID_token)        > tok.string [ _r5 = _1 ]
                      | eps [ _r5 = "" ]
                     )
                >    parse::detail::label(Description_token) > tok.string [ _r2 = _1 ]
                > (
                     tok.NoStringtableLookup_ [ _r4 = false ]
                    | eps [ _r4 = true ]
                  )
                >    parse::detail::label(Hull_token)        > tok.string [ _r3 = _1 ]
                ;

            design
                =    design_prefix(_a, _b, _c, _f, _g, _r1)
                >    parse::detail::label(Parts_token)
                >    (
                            ('[' > +tok.string [ push_back(_d, _1) ] > ']')
                        |    tok.string [ push_back(_d, _1) ]
                     )
                >   -(
                        parse::detail::label(Icon_token)     > tok.string [ _e = _1 ]
                     )
                >    parse::detail::label(Model_token)       > tok.string
                [ insert_ship_design_(_r1, _a, _b, _c, _d, _e, _1, _f, _g) ]
                ;

            start
                =   +design(_r1)
                ;

            design_prefix.name("Name, UUID, Description, Lookup Flag, Hull");
            design.name("ShipDesign");

#if DEBUG_PARSERS
            debug(design_prefix);
            debug(design);
#endif

            qi::on_error<qi::fail>(start, parse::report_error(_1, _2, _3, _4));
        }

        typedef parse::detail::rule<
            void (std::string&, std::string&, std::string&, bool&, std::string&,
                  const std::map<std::string, std::unique_ptr<ShipDesign>>&)
        > design_prefix_rule;

        typedef parse::detail::rule<
            void (std::map<std::string, std::unique_ptr<ShipDesign>>&),
            boost::spirit::qi::locals<
                std::string,
                std::string,
                std::string,
                std::vector<std::string>,
                std::string,
                bool,
                std::string
            >
        > design_rule;

        typedef parse::detail::rule<
            void (std::map<std::string, std::unique_ptr<ShipDesign>>&)
        > start_rule;

        design_prefix_rule design_prefix;
        design_rule design;
        start_rule start;
    };
}

namespace parse {
    bool ship_designs(const boost::filesystem::path& path, std::map<std::string, std::unique_ptr<ShipDesign>>& designs) {
        bool result = true;
        for(const boost::filesystem::path& file : ListScripts(path)) {
            result &= detail::parse_file<rules, std::map<std::string, std::unique_ptr<ShipDesign>>>(file, designs);
        }
        return result;
    }

    bool ship_designs(std::map<std::string, std::unique_ptr<ShipDesign>>& designs) {
        bool result = true;

        for(const boost::filesystem::path& file : ListScripts("scripting/ship_designs")) {
            result &= detail::parse_file<rules, std::map<std::string, std::unique_ptr<ShipDesign>>>(file, designs);
        }

        return result;
    }

    bool monster_designs(std::map<std::string, std::unique_ptr<ShipDesign>>& designs) {
        bool result = true;

        for (const boost::filesystem::path& file : ListScripts("scripting/monster_designs")) {
            result &= detail::parse_file<rules, std::map<std::string, std::unique_ptr<ShipDesign>>>(file, designs);
        }

        return result;
    }
}
