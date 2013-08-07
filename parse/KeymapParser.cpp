#include "Label.h"
#include "Parse.h"
#include "ParseImpl.h"

#include <boost/spirit/home/phoenix.hpp>

#define DEBUG_PARSERS 0

#if DEBUG_PARSERS
namespace std {
    inline ostream& operator<<(ostream& os, const std::map<int, int>&) { return os; 
    inline ostream& operator<<(ostream& os, const std::map<std::string, std::map<int, int> >&) { return os; }
}
#endif

namespace {
    typedef std::map<int, int>              Keymap;
    typedef std::map<std::string, Keymap>   NamedKeymaps;

    struct insert_key_pair_ {
        template <typename Arg1, typename Arg2>
        struct result
        { typedef void type; };

        void operator()(Keymap& keymap, const Keymap::value_type& key_id_pair) const {
            keymap[key_id_pair.first] = key_id_pair.second;
            //std::cout << "inserted key pair: " << key_id_pair.first << ", " << key_id_pair.second << std::endl;
        }
    };
    const boost::phoenix::function<insert_key_pair_> insert_key_pair;

    struct insert_key_map_ {
        template <typename Arg1, typename Arg2>
        struct result
        { typedef void type; };

        void operator()(NamedKeymaps& named_keymaps, const NamedKeymaps::value_type& name_keymap) const {
            named_keymaps[name_keymap.first] = name_keymap.second;
            //std::cout << "inserted keymap: " << name_keymap.first << std::endl;
        }
    };
    const boost::phoenix::function<insert_key_map_> insert_key_map;

    struct rules {
        rules() {
            const parse::lexer& tok = parse::lexer::instance();

            qi::_1_type _1;
            qi::_2_type _2;
            qi::_3_type _3;
            qi::_4_type _4;
            qi::_a_type _a;
            qi::_b_type _b;
            qi::_r1_type _r1;
            using phoenix::construct;

            int_pair
                =   tok.int_ [ _a = _1 ] >> tok.int_ [ _b = _1 ]
                    [ insert_key_pair(_r1, construct<Keymap::value_type>(_a, _b)) ]
                ;

            keymap
                =   tok.Keymap_
                >>  parse::label(Name_token) >> tok.string [ _a = _1 ]
                >>  parse::label(Keys_token)
                >>  ( '[' >> *(int_pair(_b)) >> ']' )
                    [ insert_key_map(_r1, construct<NamedKeymaps::value_type>(_a, _b)) ]
                ;

            start
                =   *keymap(_r1)
                ;

            int_pair.name("IntPair");
            keymap.name("KeyMap");
            start.name("KeyMap List");

#if DEBUG_PARSERS
            debug(article);
#endif

            qi::on_error<qi::fail>(start, parse::report_error(_1, _2, _3, _4));
        }

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            void (Keymap&),
            qi::locals<int, int>,
            parse::skipper_type
        > int_pair_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            void (NamedKeymaps&),
            qi::locals<std::string, Keymap>,
            parse::skipper_type
        > keymap_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            void (NamedKeymaps&),
            parse::skipper_type
        > start_rule;

        int_pair_rule   int_pair;
        keymap_rule     keymap;
        start_rule      start;
    };
}

namespace parse {
    bool keymaps(const boost::filesystem::path& path, NamedKeymaps& nkm)
    { return detail::parse_file<rules, NamedKeymaps>(path, nkm); }
}
