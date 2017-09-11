#include "Parse.h"

#include "ParseImpl.h"

#include "../universe/ShipDesign.h"

#include <boost/spirit/include/phoenix.hpp>
#include <boost/phoenix/function/adapt_function.hpp>
#include <boost/uuid/nil_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/optional/optional.hpp>

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
    void insert_ship_design(boost::optional<std::unique_ptr<ShipDesign>>& maybe_design,
                            const std::string& name, const std::string& description,
                            const std::string& hull, const std::vector<std::string>& parts,
                            const std::string& icon, const std::string& model,
                            bool name_desc_in_stringtable, const boost::uuids::uuid& uuid)
    {
        // TODO use make_unique when converting to C++14
        auto design = std::unique_ptr<ShipDesign>(
            new ShipDesign(name, description, 0, ALL_EMPIRES, hull, parts, icon, model,
                           name_desc_in_stringtable, false, uuid));

        maybe_design = std::move(design);
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
        rules(const std::string& filename,
              const parse::text_iterator& first, const parse::text_iterator& last)
        {
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
            qi::eps_type eps;

            const parse::lexer& tok = parse::lexer::instance();

            design_prefix
                =    tok.ShipDesign_
                >    parse::detail::label(Name_token) > tok.string [ _r1 = _1 ]
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
                =    design_prefix(_a, _b, _c, _f, _g)
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

            qi::on_error<qi::fail>(start, parse::report_error(filename, first, last, _1, _2, _3, _4));
        }

        using design_prefix_rule = parse::detail::rule<
            void (std::string&, std::string&, std::string&, bool&, boost::uuids::uuid&)>;

        using design_rule = parse::detail::rule<
            void (boost::optional<std::unique_ptr<ShipDesign>>&),
            boost::spirit::qi::locals<
                std::string,
                std::string,
                std::string,
                std::vector<std::string>,
                std::string,
                bool,
                boost::uuids::uuid
            >
        >;

        using start_rule = parse::detail::rule<
            void (boost::optional<std::unique_ptr<ShipDesign>>&)
        >;

        design_prefix_rule design_prefix;
        design_rule design;
        start_rule start;
    };

    struct manifest_rules {
        manifest_rules(const std::string& filename,
              const parse::text_iterator& first, const parse::text_iterator& last)
        {
            namespace phoenix = boost::phoenix;
            namespace qi = boost::spirit::qi;

            using phoenix::push_back;

            qi::_1_type _1;
            qi::_2_type _2;
            qi::_3_type _3;
            qi::_4_type _4;
            qi::_r1_type _r1;

            const parse::lexer& tok = parse::lexer::instance();

            design_manifest
                =    tok.ShipDesignOrdering_
                >    *(parse::detail::label(UUID_token)       > tok.string [ push_back(_r1, parse_uuid_(_1)) ])
                ;

            start
                =   +design_manifest(_r1)
                ;

            design_manifest.name("ShipDesignOrdering");

#if DEBUG_PARSERS
            debug(design_manifest);
#endif

            qi::on_error<qi::fail>(start, parse::report_error(filename, first, last, _1, _2, _3, _4));
        }

        using manifest_rule = parse::detail::rule<void (std::vector<boost::uuids::uuid>&)>;
        using start_rule = parse::detail::rule<void (std::vector<boost::uuids::uuid>&)>;

        manifest_rule design_manifest;
        start_rule start;
    };

}

namespace parse {
    std::pair<
        std::vector<std::pair<std::unique_ptr<ShipDesign>, boost::filesystem::path>>, // designs_and_paths
        std::vector<boost::uuids::uuid> //ordering
        >
    ship_designs(const boost::filesystem::path& path) {
        std::vector<std::pair<std::unique_ptr<ShipDesign>, boost::filesystem::path>> designs_and_paths;
        std::vector<boost::uuids::uuid> ordering;

        boost::filesystem::path manifest_file;

        // Allow files with any suffix in order to convert legacy ShipDesign files.
        bool permissive_mode = true;
        const auto& scripts = ListScripts(path, permissive_mode);

        for (const auto& file : scripts) {
            if (file.filename() == "ShipDesignOrdering.focs.txt" ) {
                manifest_file = file;
                continue;
            }

            try {
                boost::optional<std::unique_ptr<ShipDesign>> maybe_design;
                auto partial_result = detail::parse_file<rules, boost::optional<std::unique_ptr<ShipDesign>>>(file, maybe_design);
                if (!partial_result || !maybe_design)
                    continue;
                designs_and_paths.push_back({std::move(*maybe_design), file});

            } catch (const std::runtime_error& e) {
                ErrorLogger() << "Failed to parse ship design in " << file << " from " << path << " because " << e.what();;
            }
        }

        if (!manifest_file.empty()) {
            try {
                /*auto success =*/ detail::parse_file<manifest_rules, std::vector<boost::uuids::uuid>>(manifest_file, ordering);
            } catch (const std::runtime_error& e) {
                ErrorLogger() << "Failed to parse ship design manifest in " << manifest_file << " from " << path
                              << " because " << e.what();;
            }
        }

        return {std::move(designs_and_paths), ordering};
    }
}
namespace {
    /** Remove the file paths from the returned ShipDesigns.*/
    std::pair<
        std::vector<std::unique_ptr<ShipDesign>>, // designs
        std::vector<boost::uuids::uuid> // ordering
        >
    only_ship_designs(const boost::filesystem::path& path) {
        auto design_paths_and_ordering = parse::ship_designs(path);

        std::vector<std::unique_ptr<ShipDesign>> designs;
        for (auto& design_and_path : design_paths_and_ordering.first)
            designs.push_back(std::move(design_and_path.first));
        return {std::move(designs), design_paths_and_ordering.second};
    }
}

namespace parse {
    std::pair<
        std::vector<std::unique_ptr<ShipDesign>>, // designs
        std::vector<boost::uuids::uuid> // ordering
        >
    ship_designs()
    { return only_ship_designs("scripting/ship_designs"); }

    std::pair<
        std::vector<std::unique_ptr<ShipDesign>>, // designs
        std::vector<boost::uuids::uuid> // ordering
        >
    monster_designs()
    { return only_ship_designs("scripting/monster_designs"); }
}
