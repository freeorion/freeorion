#include "Parse.h"

#include "ParseImpl.h"

#include "../util/Directories.h"

#include <boost/spirit/include/phoenix.hpp>


#define DEBUG_PARSERS 0

#if DEBUG_PARSERS
namespace std {
    inline ostream& operator<<(ostream& os, const std::map<int, int>&) { return os; 
    inline ostream& operator<<(ostream& os, const std::map<std::string, std::map<int, int>>&) { return os; }
}
#endif

namespace {
    typedef std::map<int, int>              Keymap;
    typedef std::map<std::string, Keymap>   NamedKeymaps;

    struct insert_key_pair_ {
        typedef void result_type;

        void operator()(Keymap& keymap, const Keymap::value_type& key_id_pair) const {
            keymap[key_id_pair.first] = key_id_pair.second;
            //std::cout << "inserted key pair: " << key_id_pair.first << ", " << key_id_pair.second << std::endl;
        }
    };
    const boost::phoenix::function<insert_key_pair_> insert_key_pair;

    struct insert_key_map_ {
        typedef void result_type;

        void operator()(NamedKeymaps& named_keymaps,
                        const NamedKeymaps::value_type& name_keymap) const
        {
            named_keymaps[name_keymap.first] = name_keymap.second;
            //std::cout << "inserted keymap: " << name_keymap.first << std::endl;
        }
    };
    const boost::phoenix::function<insert_key_map_> insert_key_map;

    struct rules {
        rules(const std::string& filename,
              const parse::text_iterator& first, const parse::text_iterator& last)
        {
            namespace phoenix = boost::phoenix;
            namespace qi = boost::spirit::qi;

            using phoenix::construct;

            qi::_1_type _1;
            qi::_2_type _2;
            qi::_3_type _3;
            qi::_4_type _4;
            qi::_a_type _a;
            qi::_b_type _b;
            qi::_r1_type _r1;

            const parse::lexer& tok = parse::lexer::instance();

            int_pair
                =   tok.int_ [ _a = _1 ] >> tok.int_ [ _b = _1 ]
                    [ insert_key_pair(_r1, construct<Keymap::value_type>(_a, _b)) ]
                ;

            keymap
                =   tok.Keymap_
                >   parse::detail::label(Name_token) > tok.string [ _a = _1 ]
                >   parse::detail::label(Keys_token)
                >   ( '[' > *(int_pair(_b)) > ']' )
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

            qi::on_error<qi::fail>(start, parse::report_error(filename, first, last, _1, _2, _3, _4));
        }

        typedef parse::detail::rule<
            void (Keymap&),
            boost::spirit::qi::locals<int, int>
        > int_pair_rule;

        typedef parse::detail::rule<
            void (NamedKeymaps&),
            boost::spirit::qi::locals<std::string, Keymap>
        > keymap_rule;

        typedef parse::detail::rule<
            void (NamedKeymaps&)
        > start_rule;

        int_pair_rule   int_pair;
        keymap_rule     keymap;
        start_rule      start;
    };
}

namespace parse {
    bool keymaps(NamedKeymaps& nkm) {
        boost::filesystem::path path = GetResourceDir() / "scripting/keymaps.inf";
        return detail::parse_file<rules, NamedKeymaps>(path, nkm);
    }
}
