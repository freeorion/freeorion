#include "Parse.h"

#include "ParseImpl.h"

#include "../universe/ShipDesign.h"

#include <boost/spirit/include/phoenix.hpp>
#include <boost/phoenix/function/adapt_function.hpp>
#include <boost/uuid/nil_generator.hpp>
#include <boost/uuid/uuid_io.hpp>

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
                            bool name_desc_in_stringtable, const boost::uuids::uuid& uuid)
    {
        // TODO use make_unique when converting to C++14
        auto design = std::unique_ptr<ShipDesign>(
            new ShipDesign(name, description, 0, ALL_EMPIRES, hull, parts, icon, model, name_desc_in_stringtable, false, uuid));

        auto inserted_design = designs.insert(std::make_pair(design->Name(false), std::move(design)));
    };
    BOOST_PHOENIX_ADAPT_FUNCTION(void, insert_ship_design_, insert_ship_design, 9)

    // A UUID validator
    bool is_valid_uuid(const std::string& uuid_string) {
        try {
            boost::lexical_cast<boost::uuids::uuid>(uuid_string);
        } catch (boost::bad_lexical_cast&) {
            ErrorLogger() << uuid_string << " is not a valid UUID.  A valid UUID looks like 01234567-89ab-cdef-0123-456789abcdef";
            return false;
        }
        return true;
    };

    // A lazy UUID parser
    BOOST_PHOENIX_ADAPT_FUNCTION(bool, is_valid_uuid_, is_valid_uuid, 1)

    // A UUID parser
    boost::uuids::uuid parse_uuid(const std::string& uuid_string) {
        try {
            return boost::lexical_cast<boost::uuids::uuid>(uuid_string);
        } catch (boost::bad_lexical_cast&) {
            // This should never happen because the previous is_valid should short circuit.
            ErrorLogger() << uuid_string << " is not a valid UUID.  A valid UUID looks like 01234567-89ab-cdef-0123-456789abcdef";
            return boost::uuids::nil_generator()();
        }
    };
    // A lazy UUID parser
    BOOST_PHOENIX_ADAPT_FUNCTION(boost::uuids::uuid, parse_uuid_, parse_uuid, 1)

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
                >    ((parse::detail::label(UUID_token)
                       > tok.string [_pass = is_valid_uuid_(_1),  _r5 = parse_uuid_(_1) ])
                      | eps [ _r5 = boost::uuids::nil_generator()() ]
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

        using design_prefix_rule = parse::detail::rule<
            void (std::string&, std::string&, std::string&, bool&, boost::uuids::uuid&,
                  const std::map<std::string, std::unique_ptr<ShipDesign>>&)
        >;

        typedef parse::detail::rule<
            void (std::map<std::string, std::unique_ptr<ShipDesign>>&),
            boost::spirit::qi::locals<
                std::string,
                std::string,
                std::string,
                std::vector<std::string>,
                std::string,
                bool,
                boost::uuids::uuid
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
            try {
                result &= detail::parse_file<rules, std::map<std::string, std::unique_ptr<ShipDesign>>>(file, designs);
            } catch (const std::runtime_error& e) {
                result = false;
                ErrorLogger() << "Failed to parse ship design in " << file << " from " << path << " because " << e.what();;
            }
        }
        return result;
    }

    bool ship_designs(std::map<std::string, std::unique_ptr<ShipDesign>>& designs)
    { return ship_designs("scripting/ship_designs", designs); }

    bool monster_designs(std::map<std::string, std::unique_ptr<ShipDesign>>& designs)
    { return ship_designs("scripting/monster_designs", designs); }
}
