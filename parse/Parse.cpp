#include "ParseImpl.h"

#include "Double.h"
#include "EffectParser.h"
#include "Int.h"
#include "Label.h"
#include "ValueRefParser.h"
#include "../universe/Effect.h"

#include <boost/xpressive/xpressive.hpp>
#include <boost/algorithm/string/replace.hpp>

#define DEBUG_PARSERS 0

#if DEBUG_PARSERS
namespace std {
    inline ostream& operator<<(ostream& os, const std::vector<Effect::EffectBase*>&) { return os; }
    inline ostream& operator<<(ostream& os, const GG::Clr&) { return os; }
    inline ostream& operator<<(ostream& os, const ItemSpec&) { return os; }
}
#endif

namespace {
    struct effects_group_rules {
        effects_group_rules() {
            const parse::lexer& tok = parse::lexer::instance();

            qi::_1_type _1;
            qi::_a_type _a;
            qi::_b_type _b;
            qi::_c_type _c;
            qi::_d_type _d;
            qi::_val_type _val;
            qi::lit_type lit;
            using phoenix::construct;
            using phoenix::new_;
            using phoenix::push_back;

            effects_group
                =    tok.EffectsGroup_
                >>   parse::label(Scope_name) > parse::detail::condition_parser [ _a = _1 ]
                >>  -(
                        parse::label(Activation_name) >> parse::detail::condition_parser [ _b = _1 ]
                     )
                >>  -(
                        parse::label(StackingGroup_name) >> tok.string [ _c = _1 ]
                     )
                >>   parse::label(Effects_name)
                >>   (
                            '[' > +parse::effect_parser() [ push_back(_d, _1) ] > ']'
                        |   parse::effect_parser() [ push_back(_d, _1) ]
                     )
                     [ _val = new_<Effect::EffectsGroup>(_a, _b, _d, _c) ]
                ;

            start
                =    '[' > +effects_group [ push_back(_val, construct<boost::shared_ptr<const Effect::EffectsGroup> >(_1)) ] > ']'
                |    effects_group [ push_back(_val, construct<boost::shared_ptr<const Effect::EffectsGroup> >(_1)) ]
                ;

            effects_group.name("EffectsGroup");
            start.name("EffectsGroups");

#if DEBUG_PARSERS
            debug(effects_group);
            debug(start);
#endif
        }

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            Effect::EffectsGroup* (),
            qi::locals<
                Condition::ConditionBase*,
                Condition::ConditionBase*,
                std::string,
                std::vector<Effect::EffectBase*>
            >,
            parse::skipper_type
        > effects_group_rule;

        effects_group_rule effects_group;
        parse::detail::effects_group_rule start;
    };

    struct color_parser_rules {
        color_parser_rules() {
            const parse::lexer& tok = parse::lexer::instance();

            qi::_1_type _1;
            qi::_a_type _a;
            qi::_b_type _b;
            qi::_c_type _c;
            qi::_pass_type _pass;
            qi::_val_type _val;
            qi::eps_type eps;
            qi::uint_type uint_;
            using phoenix::construct;
            using phoenix::if_;

            channel
                =    tok.int_ [ _val = _1, _pass = 0 <= _1 && _1 <= 255 ]
                ;

            start
                =    '(' > channel [ _a = _1 ]
                >    ',' > channel [ _b = _1 ]
                >    ',' > channel [ _c = _1 ]
                >>   (
                        (
                            ',' > channel [ _val = construct<GG::Clr>(_a, _b, _c, _1) ]
                        )
                        |         eps [ _val = construct<GG::Clr>(_a, _b, _c, phoenix::val(255)) ]
                     )
                >    ')'
                ;

            channel.name("colour channel (0 to 255)");
            start.name("Colour");

#if DEBUG_PARSERS
            debug(channel);
            debug(start);
#endif
        }

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            unsigned int (),
            parse::skipper_type
        > rule;

        rule channel;
        parse::detail::color_parser_rule start;
    };

    struct item_spec_parser_rules {
        item_spec_parser_rules() {
            const parse::lexer& tok = parse::lexer::instance();

            qi::_1_type _1;
            qi::_a_type _a;
            qi::_val_type _val;
            using phoenix::construct;

            start
                =    tok.Item_
                >    parse::label(Type_name) > parse::enum_parser<UnlockableItemType>() [ _a = _1 ]
                >    parse::label(Name_name) > tok.string [ _val = construct<ItemSpec>(_a, _1) ]
                ;

            start.name("ItemSpec");

#if DEBUG_PARSERS
            debug(start);
#endif
        }

        parse::detail::item_spec_parser_rule start;
    };
}

namespace parse {
    void init() {
        const lexer& tok = lexer::instance();
        qi::_1_type _1;
        qi::_val_type _val;
        using phoenix::static_cast_;

        int_
            =    '-' > tok.int_ [ _val = -_1 ]
            |    tok.int_ [ _val = _1 ]
            ;

        double_
            =    '-' >> tok.int_ [ _val = -static_cast_<double>(_1) ]
            |    tok.int_ [ _val = static_cast_<double>(_1) ]
            |    '-' > tok.double_ [ _val = -_1 ]
            |    tok.double_ [ _val = _1 ]
            ;

        int_.name("integer");
        double_.name("real number");

#if DEBUG_PARSERS
        debug(int_);
        debug(double_);
#endif

        value_ref_parser<int>();

        condition_parser();
    }

    using namespace boost::xpressive;
    const sregex MACRO_KEY = +_w;
    const sregex MACRO_TEXT = -*_;
    const sregex MACRO_DEFINITION = (s1 = MACRO_KEY) >> _n >> "'''" >> (s2 = MACRO_TEXT) >> "'''" >> _n;
    const sregex MACRO_INSERTION = "[[" >> *space >> (s1 = MACRO_KEY) >> *space >> "]]";

    void parse_and_erase_macros_from_text(std::string& text, std::map<std::string, std::string>& macros) {
        try {
            std::string::iterator text_it = text.begin();
            while (true) {
                // find next macro definition
                smatch match;
                if (!regex_search(text_it, text.end(), match, MACRO_DEFINITION, regex_constants::match_default))
                    break;

                const std::string& matched_text = match.str();  // [[MACRO_KEY]] '''macro text'''
                //Logger().debugStream() << "found macro definition:\n" << matched_text;

                // get macro key and macro text from match
                const std::string& macro_key = match[1];
                assert(macro_key != "");
                const std::string& macro_text = match[2];
                assert(macro_text != "");

                Logger().debugStream() << "key: " << macro_key;
                Logger().debugStream() << "text:\n" << macro_text;

                // store macro
                if (macros.find(macro_key) == macros.end()) {
                    macros[macro_key] = macro_text;
                } else {
                    Logger().errorStream() << "Duplicate macro key foud: " << macro_key << ".  Ignoring duplicate.";
                }

                // remove macro definition from text by replacing with whitespace that is ignored by later parsing
                text.replace(text_it + match.position(), text_it + match.position() + match.length(), match.length(), ' ');
                // subsequent scanning starts after macro defininition
                text_it = text.end() - match.suffix().length();
            }
        } catch (const std::exception& e) {
            Logger().errorStream() << "Exception caught regex parsing script file: " << e.what();
            std::cerr << "Exception caught regex parsing script file: " << e.what() << std::endl;
            return;
        }
    }

    std::set<std::string> macros_directly_referenced_in_text(const std::string& text) {
        std::set<std::string> retval;
        try {
            std::size_t position = 0; // position in the text, past the already processed part
            smatch match;
            while (regex_search(text.begin() + position, text.end(), match, MACRO_INSERTION, regex_constants::match_default)) {
                position += match.position() + match.length();
                retval.insert(match[1]);
            }
        } catch (const std::exception& e) {
            Logger().errorStream() << "Exception caught regex parsing script file: " << e.what();
            std::cerr << "Exception caught regex parsing script file: " << e.what() << std::endl;
            return retval;
        }
    }

    bool macro_deep_referenced_in_text(const std::string& macro_to_find, const std::string& text,
                                       const std::map<std::string, std::string>& macros)
    {
        // check of text directly references macro_to_find
        std::set<std::string> macros_directly_referenced_in_input_text = macros_directly_referenced_in_text(text);
        if (macros_directly_referenced_in_input_text.empty())
            return false;
        if (macros_directly_referenced_in_input_text.find(macro_to_find) != macros_directly_referenced_in_input_text.end())
            return true;
        // check if macros referenced in text reference macro_to_find
        for (std::set<std::string>::const_iterator direct_refs_it = macros_directly_referenced_in_input_text.begin();
             direct_refs_it != macros_directly_referenced_in_input_text.end(); ++direct_refs_it)
        {
            // get text of directly referenced macro
            const std::string& direct_referenced_macro_key = *direct_refs_it;
            std::map<std::string, std::string>::const_iterator macro_it = macros.find(direct_referenced_macro_key);
            if (macro_it == macros.end()) {
                Logger().errorStream() << "macro_deep_referenced_in_text couldn't find referenced macro: " << direct_referenced_macro_key;
                continue;
            }
            const std::string& macro_text = macro_it->second;
            // check of text of directly referenced macro has any reference to the macro_to_find
            if (macro_deep_referenced_in_text(macro_to_find, macro_text, macros))
                return true;
        }
        // didn't locate macro_to_find in any of the macros referenced in this text
        return false;
    }

    void check_for_cyclic_macro_references(const std::map<std::string, std::string>& macros) {
        for (std::map<std::string, std::string>::const_iterator macro_it = macros.begin();
             macro_it != macros.end(); ++macro_it)
        {
            if (macro_deep_referenced_in_text(macro_it->first, macro_it->second, macros))
                Logger().errorStream() << "Cyclic macro found: " << macro_it->first << " references itself (eventually)";
        }
        Logger().debugStream() << "No cyclic macro references found in file.";
    }

    void replace_macro_references(std::string& text, const std::map<std::string, std::string>& macros) {
        try {
            std::size_t position = 0; // position in the text, past the already processed part
            smatch match;
            while (regex_search(text.begin() + position, text.end(), match, MACRO_INSERTION, regex_constants::match_default)) {
                position += match.position();
                const std::string& matched_text = match.str();  // [[MACRO_KEY]]
                const std::string& macro_key = match[1];        // just MACRO_KEY
                // look up macro key to insert
                std::map<std::string, std::string>::const_iterator macro_lookup_it = macros.find(macro_key);
                if (macro_lookup_it != macros.end()) {
                    // insert macro text in place of reference
                    text.replace(position, matched_text.length(), macro_lookup_it->second);
                    // recusrive replacement allowed, so don't skip past
                    // start of replacement text, so that inserted text can
                    // be matched on the next pass
                } else {
                    Logger().errorStream() << "Unresolved macro reference: " << macro_key;
                    position += match.length();
                }
            }
        } catch (const std::exception& e) {
            Logger().errorStream() << "Exception caught regex parsing script file: " << e.what();
            std::cerr << "Exception caught regex parsing script file: " << e.what() << std::endl;
            return;
        }
    }

    void macro_substitution(std::string& text) {
        Logger().debugStream() << "macro_substitution for text:" << text;
        std::map<std::string, std::string> macros;

        parse_and_erase_macros_from_text(text, macros);
        check_for_cyclic_macro_references(macros);

        // recursively expand macro keys: replace [[MACRO_KEY]] in other macro
        // text with the macro text corresponding to MACRO_KEY.
        for (std::map<std::string, std::string>::iterator macro_it = macros.begin();
             macro_it != macros.end(); ++macro_it)
        {
            replace_macro_references(macro_it->second, macros);
        }

        // substitute macro keys - replace [[MACRO_KEY]] in the input text with
        // the macro text corresponding to MACRO_KEY
        replace_macro_references(text, macros);

        Logger().debugStream() << "after macro substitution text: " << text;
    }

    namespace detail {
        effects_group_rule& effects_group_parser() {
            static effects_group_rules rules;
            return rules.start;
        }

        color_parser_rule& color_parser() {
            static color_parser_rules rules;
            return rules.start;
        }

        item_spec_parser_rule& item_spec_parser() {
            static item_spec_parser_rules rules;
            return rules.start;
        }

        void parse_file_common(const boost::filesystem::path& path, const parse::lexer& l,
                               std::string& filename, std::string& file_contents,
                               parse::text_iterator& first, parse::token_iterator& it)
        {
            filename = path.string();

            {
                boost::filesystem::ifstream ifs(path);
                if (ifs) {
                    std::getline(ifs, file_contents, '\0');
                } else {
                    Logger().errorStream() << "Unable to open data file " << filename;
                    return;
                }
            }

            macro_substitution(file_contents);

            first = parse::text_iterator(file_contents.begin());
            parse::text_iterator last(file_contents.end());

            parse::detail::s_text_it = &first;
            parse::detail::s_begin = first;
            parse::detail::s_end = last;
            parse::detail::s_filename = filename.c_str();
            it = l.begin(first, last);
        }
    }
}
