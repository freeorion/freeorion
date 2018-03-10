#include "Parse.h"
#include "ParseImpl.h"

#include "ConditionParserImpl.h"
#include "EffectParser.h"
#include "EnumParser.h"
#include "ValueRefParser.h"

#include "../universe/Tech.h"
#include "../universe/Effect.h"
#include "../util/Logger.h"
#include "../util/Directories.h"

#include <boost/xpressive/xpressive.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/algorithm/string/find_iterator.hpp>

#define DEBUG_PARSERS 0

#if DEBUG_PARSERS
namespace std {
    inline ostream& operator<<(ostream& os, const std::vector<Effect::EffectBase*>&) { return os; }
    inline ostream& operator<<(ostream& os, const GG::Clr&) { return os; }
    inline ostream& operator<<(ostream& os, const ItemSpec&) { return os; }
}
#endif

namespace parse {
    using namespace boost::xpressive;
    const sregex MACRO_KEY = +_w;   // word character (alnum | _), one or more times, greedy
    const sregex MACRO_TEXT = -*_;  // any character, zero or more times, not greedy
    const sregex MACRO_DEFINITION = (s1 = MACRO_KEY) >> _n >> "'''" >> (s2 = MACRO_TEXT) >> "'''" >> _n;
    const sregex MACRO_INSERTION = "[[" >> *space >> (s1 = MACRO_KEY) >> *space >> !("(" >> (s2 = +~(set = ')', '\n')) >> ")") >> "]]";

    void parse_and_erase_macro_definitions(std::string& text, std::map<std::string, std::string>& macros) {
        try {
            std::string::iterator text_it = text.begin();
            while (true) {
                // find next macro definition
                smatch match;
                if (!regex_search(text_it, text.end(), match, MACRO_DEFINITION, regex_constants::match_default))
                    break;

                //const std::string& matched_text = match.str();  // [[MACRO_KEY]] '''macro text'''
                //DebugLogger() << "found macro definition:\n" << matched_text;

                // get macro key and macro text from match
                const std::string& macro_key = match[1];
                assert(macro_key != "");
                const std::string& macro_text = match[2];
                assert(macro_text != "");

                //DebugLogger() << "key: " << macro_key;
                //DebugLogger() << "text:\n" << macro_text;

                // store macro
                if (!macros.count(macro_key)) {
                    macros[macro_key] = macro_text;
                } else {
                    ErrorLogger() << "Duplicate macro key foud: " << macro_key << ".  Ignoring duplicate.";
                }

                // remove macro definition from text by replacing with a newline that is ignored by later parsing
                text.replace(text_it + match.position(), text_it + match.position() + match.length(), "\n");
                // subsequent scanning starts after macro defininition
                text_it = text.end() - match.suffix().length();
            }
        } catch (const std::exception& e) {
            ErrorLogger() << "Exception caught regex parsing script file: " << e.what();
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
            ErrorLogger() << "Exception caught regex parsing script file: " << e.what();
            std::cerr << "Exception caught regex parsing script file: " << e.what() << std::endl;
            return retval;
        }
        return retval;
    }

    bool macro_deep_referenced_in_text(const std::string& macro_to_find, const std::string& text,
                                       const std::map<std::string, std::string>& macros)
    {
        TraceLogger() << "Checking if " << macro_to_find << " deep referenced in text: " << text;
        // check of text directly references macro_to_find
        std::set<std::string> macros_directly_referenced_in_input_text = macros_directly_referenced_in_text(text);
        if (macros_directly_referenced_in_input_text.empty())
            return false;
        if (macros_directly_referenced_in_input_text.count(macro_to_find))
            return true;
        // check if macros referenced in text reference macro_to_find
        for (const std::string& direct_referenced_macro_key : macros_directly_referenced_in_input_text) {
            // get text of directly referenced macro
            auto macro_it = macros.find(direct_referenced_macro_key);
            if (macro_it == macros.end()) {
                ErrorLogger() << "macro_deep_referenced_in_text couldn't find referenced macro: " << direct_referenced_macro_key;
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
        for (const auto& macro : macros) {
            if (macro_deep_referenced_in_text(macro.first, macro.second, macros))
                ErrorLogger() << "Cyclic macro found: " << macro.first << " references itself (eventually)";
        }
    }

    void replace_macro_references(std::string& text,
                                  const std::map<std::string, std::string>& macros)
    {
        try {
            std::size_t position = 0; // position in the text, past the already processed part
            smatch match;
            while (regex_search(text.begin() + position, text.end(), match, MACRO_INSERTION, regex_constants::match_default)) {
                position += match.position();
                const std::string& matched_text = match.str();  // [[MACRO_KEY]] or [[MACRO_KEY(foo,bar,...)]]
                const std::string& macro_key = match[1];        // just MACRO_KEY
                // look up macro key to insert
                auto macro_lookup_it = macros.find(macro_key);
                if (macro_lookup_it != macros.end()) {
                    // verify that macro is safe: check for cyclic reference of macro to itself
                    if (macro_deep_referenced_in_text(macro_key, macro_lookup_it->second, macros)) {
                        ErrorLogger() << "Skipping cyclic macro reference: " << macro_key;
                        position += match.length();
                    } else {
                        // insert macro text in place of reference
                        std::string replacement = macro_lookup_it->second;
                        std::string macro_params = match[2]; // arg1,arg2,arg3,etc.
                        if (!macro_params.empty()) { // found macro parameters
                            int replace_number = 1;
                            for (boost::split_iterator<std::string::iterator> it =
                                    boost::make_split_iterator(macro_params, boost::first_finder(",", boost::is_iequal()));
                                it != boost::split_iterator<std::string::iterator>();
                                ++it, ++replace_number)
                            {
                                // not using %1% (and boost::fmt) because the replaced text may itself have %s inside it that will get eaten
                                boost::replace_all(replacement, "@" + std::to_string(replace_number) + "@", boost::copy_range<std::string>(*it));
                            }
                        }
                        text.replace(position, matched_text.length(), replacement);
                        // recursive replacement allowed, so don't skip past
                        // start of replacement text, so that inserted text can
                        // be matched on the next pass
                    }
                } else {
                    ErrorLogger() << "Unresolved macro reference: " << macro_key;
                    position += match.length();
                }
            }
        } catch (const std::exception& e) {
            ErrorLogger() << "Exception caught regex parsing script file: " << e.what();
            std::cerr << "Exception caught regex parsing script file: " << e.what() << std::endl;
            return;
        }
    }

    void macro_substitution(std::string& text) {
        //DebugLogger() << "macro_substitution for text:" << text;
        std::map<std::string, std::string> macros;

        parse_and_erase_macro_definitions(text, macros);
        check_for_cyclic_macro_references(macros);

        //DebugLogger() << "after macro pasring text:" << text;

        // recursively expand macro keys: replace [[MACRO_KEY]] in other macro
        // text with the macro text corresponding to MACRO_KEY.
        for (auto& macro : macros)
        { replace_macro_references(macro.second, macros); }

        // substitute macro keys - replace [[MACRO_KEY]] in the input text with
        // the macro text corresponding to MACRO_KEY
        replace_macro_references(text, macros);

        //DebugLogger() << "after macro substitution text: " << text;
    }

    bool read_file(const boost::filesystem::path& path, std::string& file_contents) {
        boost::filesystem::ifstream ifs(path);
        if (!ifs)
            return false;

        // skip byte order mark (BOM)
        for (int BOM : {0xEF, 0xBB, 0xBF}) {
            if (BOM != ifs.get()) {
                // no header set stream back to start of file
                ifs.seekg(0, std::ios::beg);
                // and continue
                break;
            }
        }

        std::getline(ifs, file_contents, '\0');

        // no problems?
        return true;
    }

    /**  \brief Return a vector of absolute paths to script files in the given path
     * 
     * @param[in] path relative or absolute directory (searched recursively)
     * @return Any *.focs.txt files in path or 'GetResourceDir() / path'.
     */
    std::vector<boost::filesystem::path> ListScripts(const boost::filesystem::path& path, bool allow_permissive) {
        std::vector<boost::filesystem::path> scripts;

        try {
            const auto& files = ListDir(path);
            for (const auto& file : files) {
                std::string fn_ext = file.extension().string();
                std::string fn_stem_ext = file.stem().extension().string();
                if (fn_ext == ".txt" && fn_stem_ext == ".focs") {
                    scripts.push_back(file);
                } else {
                    TraceLogger() << "Parse: Skipping file " << file.string()
                                  << " due to extension (" << fn_stem_ext << fn_ext << ")";
                }
            }

            // If in permissive mode and no scripts are found allow all files to be scripts.
            if (allow_permissive && scripts.empty() && !files.empty()) {
                WarnLogger() << PathToString(path) << " does not contain scripts with the expected suffix .focs.txt. "
                             << " Trying a more permissive mode and ignoring file suffix.";
                scripts = files;
            }
        } catch (const boost::filesystem::filesystem_error& ec) {
            ErrorLogger() << "Error accessing file " << ec.path1() << " (" << ec.what() << ")";
        }

        return scripts;
    }

    const sregex FILENAME_TEXT = -+_;   // any character, one or more times, not greedy
    const sregex FILENAME_INSERTION = bol >> "#include" >> *space >> "\"" >> (s1 = FILENAME_TEXT) >> "\"" >> *space >> _n;

    std::set<std::string> missing_include_files;

    /** \brief Resolve script directives
     * 
     * @param[in,out] text contents to search through
     * @param[in] file_search_path base path of content
     */
    void file_substitution(std::string& text, const boost::filesystem::path& file_search_path) {
        if (!boost::filesystem::is_directory(file_search_path)) {
            ErrorLogger() << "File parsing include substitution given search path that is not a directory: "
                          << file_search_path.string();
            return;
        }
        try {
            std::set<boost::filesystem::path> files_included;
            process_include_substitutions(text, file_search_path, files_included);
        } catch (const std::exception& e) {
            ErrorLogger() << "Exception caught regex parsing script file: " << e.what();
            std::cerr << "Exception caught regex parsing script file: " << e.what() << std::endl;
            return;
        }
    }

    /** \brief Replace all include statements with contents of file
     * 
     * Search for any include statements in *text* and replace them with the contents
     * of the file given.  File lookup is relative to *file_search_path* and will not
     * be included if found in *files_included*.
     * Each included file is added to *files_included*.
     * This is a recursive function, processing the contents of any included files.
     * 
     * @param[in,out] text content to search through
     * @param[in] file_search_path base path of content
     * @param[in,out] files_included canonical path of any files previously included
     * */
    void process_include_substitutions(std::string& text, const boost::filesystem::path& file_search_path,
                                       std::set<boost::filesystem::path>& files_included)
    {
        smatch match;
        while (regex_search(text.begin(), text.end(), match, FILENAME_INSERTION, regex_constants::match_default)) {
            const std::string& fn_match = match[1];
            if (fn_match.empty()) {
                continue;
            }
            const sregex INCL_ONCE_SEARCH = bol >> "#include" >> *space >> "\"" >> fn_match >> "\"" >> *space >> _n;
            boost::filesystem::path base_path;
            boost::filesystem::path match_path;
            // check for base path
            if (fn_match.substr(0, 1) == "/") {
                base_path = GetResourceDir();
                match_path = base_path / fn_match.substr(1);
            } else {
                base_path = file_search_path;
                match_path = base_path / fn_match;
            }
            std::string fn_str = boost::filesystem::path(fn_match).filename().string();
            if (fn_str.substr(0, 1) == "*") {
                if (match_path.parent_path().empty()) {
                    DebugLogger() << "Parse: " << match_path.parent_path().string() << " is empty, skipping.";
                    continue;
                }
                fn_str = fn_str.substr(1, fn_str.size() - 1);
                std::set<boost::filesystem::path> match_list;
                // filter results
                for (const boost::filesystem::path& file : ListDir(match_path.parent_path())) {
                    std::string it_str = file.filename().string();
                    std::size_t it_len = it_str.length();
                    std::size_t match_len = fn_str.length();
                    if (it_len > match_len) {
                        if (it_str.substr(it_len - match_len, match_len) == fn_str) {
                            match_list.insert(file);
                        }
                    }
                }
                // read in results
                std::string dir_text;
                for (const boost::filesystem::path& file : match_list) {
                    if (files_included.insert(boost::filesystem::canonical(file)).second) {
                        std::string new_text;
                        if (read_file(file, new_text)) {
                            new_text.append("\n");
                            dir_text.append(new_text);
                        } else {
                            ErrorLogger() << "Parse: Unable to read file " << file.string();
                        }
                    }
                }
                text = regex_replace(text, INCL_ONCE_SEARCH, dir_text, regex_constants::format_first_only);
            } else if (files_included.insert(boost::filesystem::canonical(match_path)).second) {
                std::string file_content;
                if (read_file(match_path, file_content)) {
                    file_content.append("\n");
                    process_include_substitutions(file_content, match_path.parent_path(), files_included);
                    text = regex_replace(text, INCL_ONCE_SEARCH, file_content, regex_constants::format_first_only);
                } else if (missing_include_files.insert(PathToString(match_path)).second) {
                    ErrorLogger() << "Parse: " << PathToString(match_path) << " was not found for inclusion (Path:"
                                  << PathToString(base_path) << ") (File:" << fn_str << ")";
                }
            }
            // remove any remaining includes of this file
            text = regex_replace(text, INCL_ONCE_SEARCH, "\n", regex_constants::match_default);
            // TraceLogger() << "Parse: contents after scrub of " << fn_match << ":\n" << text;
        }
    }

    namespace detail {
    double_grammar::double_grammar(const parse::lexer& tok) :
        double_grammar::base_type(double_, "double_grammar")
    {
        namespace phoenix = boost::phoenix;
        namespace qi = boost::spirit::qi;

        using phoenix::static_cast_;

        qi::_1_type _1;
        qi::_val_type _val;

        double_
            =    '-' >> tok.int_ [ _val = -static_cast_<double>(_1) ]
            |    tok.int_ [ _val = static_cast_<double>(_1) ]
            |    '-' >> tok.double_ [ _val = -_1 ]
            |    tok.double_ [ _val = _1 ]
            ;

        double_.name("real number");

#if DEBUG_PARSERS
        debug(double_);
#endif

    }

    int_grammar::int_grammar(const parse::lexer& tok) :
        int_grammar::base_type(int_, "int_grammar")
    {

        namespace phoenix = boost::phoenix;
        namespace qi = boost::spirit::qi;

        using phoenix::static_cast_;

        qi::_1_type _1;
        qi::_val_type _val;

        int_
            =    '-' >> tok.int_ [ _val = -_1 ]
            |    tok.int_ [ _val = _1 ]
            ;

        int_.name("integer");

#if DEBUG_PARSERS
        debug(detail::int_);
        debug(detail::double_);
#endif
    }

#define PARSING_LABELS_OPTIONAL false
    label_rule& Labeller::operator()(const parse::lexer::string_token_def& token) {
        auto it = m_rules.find(&token);
        if (it != m_rules.end())
            return it->second;

        label_rule& retval = m_rules[&token];
        if (PARSING_LABELS_OPTIONAL) {
            retval = -(token >> '=');
        } else {
            retval =  (token >> '=');
        }
        return retval;
    }

    tags_grammar::tags_grammar(const parse::lexer& tok,
                               Labeller& label) :
        tags_grammar::base_type(start, "tags_grammar"),
        one_or_more_string_tokens(tok)
    {
        namespace phoenix = boost::phoenix;
        namespace qi = boost::spirit::qi;

        using phoenix::insert;

        start %=
            -(
                label(tok.Tags_)
                >>  one_or_more_string_tokens
            )
            ;

        start.name("Tags");

#if DEBUG_PARSERS
        debug(start);
#endif
    }

    color_parser_grammar::color_parser_grammar(const parse::lexer& tok) :
        color_parser_grammar::base_type(start, "color_parser_grammar")
    {
        namespace phoenix = boost::phoenix;
        namespace qi = boost::spirit::qi;

        using phoenix::construct;
        using phoenix::if_;

        qi::_1_type _1;
        qi::_2_type _2;
        qi::_3_type _3;
        qi::_4_type _4;
        qi::_pass_type _pass;
        qi::_val_type _val;
        qi::eps_type eps;

        channel = tok.int_ [ _val = _1, _pass = 0 <= _1 && _1 <= 255 ];
        alpha   = (',' > channel) [ _val = _1 ] | eps [ _val = 255 ];
        start
            =  ( ('(' >> channel )
            >    (',' >> channel )
            >    (',' >> channel )
            >    alpha
            >    ')'
               ) [ _val  = construct<GG::Clr>(_1, _2, _3, _4)]
            ;

        channel.name("colour channel (0 to 255)");
        alpha.name("alpha channel (0 to 255) defaults to 255");
        start.name("Colour");

#if DEBUG_PARSERS
        debug(channel);
        debug(start);
#endif
    }

    item_spec_grammar::item_spec_grammar(
        const parse::lexer& tok,
        Labeller& label
    ) :
        item_spec_grammar::base_type(start, "item_spec_grammar"),
        unlockable_item_type_enum(tok)
    {
        namespace phoenix = boost::phoenix;
        namespace qi = boost::spirit::qi;

        using phoenix::construct;

        qi::_1_type _1;
        qi::_2_type _2;
        qi::_val_type _val;
        qi::omit_type omit_;

        start
            =  ( omit_[tok.Item_]
            >    label(tok.Type_) > unlockable_item_type_enum
            >    label(tok.Name_) > tok.string
               ) [ _val = construct<ItemSpec>(_1, _2) ]
            ;

        start.name("ItemSpec");

#if DEBUG_PARSERS
        debug(start);
#endif
    }


        /** \brief Load and parse script file(s) from given path
         * 
         * @param[in] path absolute path to a regular file
         * @param[in] l lexer instance to use
         * @param[out] filename filename of the given path
         * @param[out] file_contents parsed contents of file(s)
         * @param[out] first content iterator
         * @param[out] it lexer iterator
         */
        void parse_file_common(const boost::filesystem::path& path, const parse::lexer& lexer,
                               std::string& filename, std::string& file_contents,
                               parse::text_iterator& first, parse::text_iterator& last, parse::token_iterator& it)
        {
            filename = path.string();

            bool read_success = read_file(path, file_contents);
            if (!read_success) {
                ErrorLogger() << "Unable to open data file " << filename;
                return;
            }

            // add newline at end to avoid errors when one is left out, but is expected by parsers
            file_contents += "\n";

            file_substitution(file_contents, path.parent_path());
            macro_substitution(file_contents);

            first = file_contents.begin();
            last = file_contents.end();

            it = lexer.begin(first, last);
        }


        bool parse_file_end_of_file_warnings(const boost::filesystem::path& path,
                                             bool parser_success,
                                             std::string& file_contents,
                                             text_iterator& first,
                                             text_iterator& last)
        {
            if (!parser_success)
                WarnLogger() << "A parser failed while parsing " << path;

            auto length_of_unparsed_file = std::distance(first, last);
            bool parse_length_good = ((length_of_unparsed_file == 0)
                                      || (length_of_unparsed_file == 1 && *first == '\n'));

            if (!parse_length_good
                && length_of_unparsed_file > 0
                && static_cast<std::string::size_type>(length_of_unparsed_file) <= file_contents.size())
            {
                auto unparsed_section = file_contents.substr(file_contents.size() - std::abs(length_of_unparsed_file));
                std::copy(first, last, std::back_inserter(unparsed_section));
                WarnLogger() << "File \"" << path << "\" was incompletely parsed. " << std::endl
                             << "Unparsed section of file, " << length_of_unparsed_file <<" characters:" << std::endl
                             << unparsed_section;
            }

            return parser_success && parse_length_good;

        }

    }
}
