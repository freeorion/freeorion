#include "ParseImpl.h"
#include "Label.h"


#define DEBUG_PARSERS 0

#if DEBUG_PARSERS
namespace std {
    inline ostream& operator<<(ostream& os, const std::map<std::string, ShipDesign*>&) { return os; }
    inline ostream& operator<<(ostream& os, const std::pair<const std::string, ShipDesign*>&) { return os; }
    inline ostream& operator<<(ostream& os, const std::vector<std::string>&) { return os; }
}
#endif

namespace {
    struct insert_
    {
        template <typename Arg1, typename Arg2>
        struct result
        { typedef void type; };

        void operator()(std::map<std::string, ShipDesign*>& designs, ShipDesign* design) const
        {
            if (!designs.insert(std::make_pair(design->Name(false), design)).second) {
                std::string error_str = "ERROR: More than one predefined ship design in predefined_ship_designs.txt has the name " + design->Name(false);
                throw std::runtime_error(error_str.c_str());
            }
        }
    };
    const boost::phoenix::function<insert_> insert;

    struct rules
    {
        rules()
        {
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
            qi::_r1_type _r1;
            qi::_r2_type _r2;
            qi::_r3_type _r3;
            using phoenix::new_;
            using phoenix::push_back;

            design_prefix
                =    tok.ShipDesign_
                >    parse::label(Name_name)        > tok.string [ _r1 = _1 ]
                >    parse::label(Description_name) > tok.string [ _r2 = _1 ]
                >    parse::label(Hull_name)        > tok.string [ _r3 = _1 ]
                ;

            design
                =    design_prefix(_a, _b, _c)
                >    parse::label(Parts_name)
                >    (
                            '[' > +tok.string [ push_back(_d, _1) ] > ']'
                        |   tok.string [ push_back(_d, _1) ]
                        )
                >    parse::label(Graphic_name) > tok.string [ _e = _1 ]
                >    parse::label(Model_name)   > tok.string [ insert(_r1, new_<ShipDesign>(_a, _b, ALL_EMPIRES, 0, _c, _d, _e, _1)) ]
                ;

            start
                =   +design(_r1)
                ;

            design_prefix.name("ShipDesign");
            design.name("ShipDesign");

#if DEBUG_PARSERS
            debug(design_prefix);
            debug(design);
#endif

            qi::on_error<qi::fail>(start, parse::report_error(_1, _2, _3, _4));
        }

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            void (std::string&, std::string&, std::string&),
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
                std::string
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
}
