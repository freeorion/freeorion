#include "ParseImpl.h"

#include "Double.h"
#include "EffectParser.h"
#include "Int.h"
#include "Label.h"
#include "ValueRefParser.h"
#include "../universe/Effect.h"


#define DEBUG_PARSERS 0

#if DEBUG_PARSERS
namespace std {
    inline ostream& operator<<(ostream& os, const std::vector<Effect::EffectBase*>&) { return os; }
    inline ostream& operator<<(ostream& os, const GG::Clr&) { return os; }
    inline ostream& operator<<(ostream& os, const ItemSpec&) { return os; }
}
#endif

namespace {

    struct effects_group_rules
    {
        effects_group_rules()
            {
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

    struct color_parser_rules
    {
        color_parser_rules()
            {
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
                    =    tok.int_ [ _val = _1, _pass = 0 < _1 && _1 <= 255 ]
                    ;

                start
                    =    '(' > channel [ _a = _1 ]
                    >    ',' > channel [ _b = _1 ]
                    >    ',' > channel [ _c = _1 ]
                    >>   (
                              ',' > channel [ construct<GG::Clr>(_a, _b, _c, _1) ]
                          |   eps [ construct<GG::Clr>(_a, _b, _c, 255) ]
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

    struct item_spec_parser_rules
    {
        item_spec_parser_rules()
            {
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

    void init()
    {
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

    namespace detail {

        effects_group_rule& effects_group_parser()
        {
            static effects_group_rules rules;
            return rules.start;
        }

        color_parser_rule& color_parser()
        {
            static color_parser_rules rules;
            return rules.start;
        }

        item_spec_parser_rule& item_spec_parser()
        {
            static item_spec_parser_rules rules;
            return rules.start;
        }

        void parse_file_common(const boost::filesystem::path& path,
                               const parse::lexer& l,
                               std::string& filename,
                               std::string& file_contents,
                               parse::text_iterator& first,
                               parse::token_iterator& it)
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
