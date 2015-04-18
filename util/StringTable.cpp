#include "StringTable.h"

#include "Logger.h"
#include "Directories.h"

#include <GG/utf8/checked.h>

#include <boost/filesystem/fstream.hpp>
#include <boost/xpressive/xpressive.hpp>
#include <boost/algorithm/string/replace.hpp>

#include <iostream>


// static(s)
const std::string StringTable_::S_DEFAULT_FILENAME = "en.txt";
const std::string StringTable_::S_ERROR_STRING = "ERROR: ";

// StringTable
StringTable_::StringTable_():
    m_filename(S_DEFAULT_FILENAME)
{ Load(); }

StringTable_::StringTable_(const std::string& filename, const StringTable_* lookups_fallback_table /* = 0 */):
    m_filename(filename)
{ Load(lookups_fallback_table); }

StringTable_::~StringTable_()
{}

bool StringTable_::StringExists(const std::string& index) const
{ return m_strings.find(index) != m_strings.end(); }

bool StringTable_::Empty() const
{ return m_strings.empty(); }

const std::string& StringTable_::operator[] (const std::string& index) const {
    static std::string error_retval;
    std::map<std::string, std::string>::const_iterator it = m_strings.find(index);
    return it == m_strings.end() ? error_retval = S_ERROR_STRING + index : it->second;
}

namespace {
    bool read_file(const boost::filesystem::path& path, std::string& file_contents) {
        boost::filesystem::ifstream ifs(path);
        if (!ifs)
            return false;

        // skip byte order mark (BOM)
        static const int UTF8_BOM[3] = {0x00EF, 0x00BB, 0x00BF};
        for (int i = 0; i < 3; i++) {
            if (UTF8_BOM[i] != ifs.get()) {
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
}

void StringTable_::Load(const StringTable_* lookups_fallback_table /* = 0 */) {
    boost::filesystem::path path = FilenameToPath(m_filename);
    std::string file_contents;

    bool read_success = read_file(path, file_contents);
    if (!read_success) {
        ErrorLogger() << "StringTable_::Load failed to read file at path: " << path.string();
        return;
    }
    std::map<std::string, std::string> fallback_lookup_strings;
    std::string fallback_table_file;
    if (lookups_fallback_table) {
        fallback_table_file = lookups_fallback_table->Filename();
        fallback_lookup_strings.insert(lookups_fallback_table->GetStrings().begin(), lookups_fallback_table->GetStrings().end());
    }

    using namespace boost::xpressive;

    const sregex IDENTIFIER = +_w;
    const sregex COMMENT = '#' >> *(~_n) >> _n;
    const sregex KEY = IDENTIFIER;
    const sregex SINGLE_LINE_VALUE = *(~_n);
    const sregex MULTI_LINE_VALUE = -*_;

    const sregex ENTRY =
        keep(*(space | +COMMENT)) >>
        KEY >> *blank >> (_n | COMMENT) >>
        (("'''" >> MULTI_LINE_VALUE >> "'''" >> *space >> _n) | SINGLE_LINE_VALUE >> _n);

    const sregex TRAILING_WS =
        *(space | COMMENT);

    const sregex REFERENCE =
        keep("[[" >> (s1 = IDENTIFIER) >> +space >> (s2 = IDENTIFIER) >> "]]");

    const sregex KEYEXPANSION =
        keep("[[" >> (s1 = IDENTIFIER) >> "]]");

    // parse input text stream
    std::string::iterator it = file_contents.begin();
    std::string::iterator end = file_contents.end();

    smatch matches;
    bool well_formed = false;
    std::string key, prev_key;
    try {
        // grab first line of file, which should be the name of this language
        well_formed = regex_search(it, end, matches, SINGLE_LINE_VALUE, regex_constants::match_continuous);
        it = end - matches.suffix().length();
        if (well_formed)
            m_language = matches.str(0);

        // match series of key-value entries to store as stringtable
        while (well_formed) {
            well_formed = regex_search(it, end, matches, ENTRY, regex_constants::match_continuous);
            it = end - matches.suffix().length();

            if (well_formed) {
                for (smatch::nested_results_type::const_iterator match_it = matches.nested_results().begin();
                     match_it != matches.nested_results().end(); ++match_it)
                {
                    if (match_it->regex_id() == KEY.regex_id()) {
                        key = match_it->str();
                    } else if (match_it->regex_id() == SINGLE_LINE_VALUE.regex_id() ||
                               match_it->regex_id() == MULTI_LINE_VALUE.regex_id())
                    {
                        assert(key != "");
                        if (m_strings.find(key) == m_strings.end()) {
                            m_strings[key] = match_it->str();
                            boost::algorithm::replace_all(m_strings[key], "\\n", "\n");
                        } else {
                            ErrorLogger() << "Duplicate string ID found: '" << key
                                                   << "' in file: '" << m_filename
                                                   << "'.  Ignoring duplicate.";
                        }
                        prev_key = key;
                        key.clear();
                    }
                }
            }
        }

        regex_search(it, end, matches, TRAILING_WS, regex_constants::match_continuous);
        it = end - matches.suffix().length();

        well_formed = it == end;
    } catch (std::exception& e) {
        ErrorLogger() << "Exception caught regex parsing Stringtable: " << e.what();
        ErrorLogger() << "Last and prior keys matched: " << key << ", " << prev_key;
        std::cerr << "Exception caught regex parsing Stringtable: " << e.what() << std::endl;
        std::cerr << "Last and prior keys matched: " << key << ", " << prev_key << std::endl;
        return;
    }

    if (well_formed) {
        // recursively expand keys -- replace [[KEY]] by the text resulting from expanding everything in the definition for KEY
        for (std::map<std::string, std::string>::iterator map_it = m_strings.begin();
             map_it != m_strings.end(); ++map_it)
        {
            //DebugLogger() << "Checking key expansion for: " << map_it->first;
            std::size_t position = 0; // position in the definition string, past the already processed part
            smatch match;
            std::map<std::string, std::size_t> cyclic_reference_check;
            cyclic_reference_check[map_it->first] = map_it->second.length();
            std::string rawtext = map_it->second;
            std::string cumulative_subsititions;
            while (regex_search(map_it->second.begin() + position, map_it->second.end(), match, KEYEXPANSION)) {
                position += match.position();
                //DebugLogger() << "checking next internal keyword match: " << match[1] << " with matchlen " << match.length();
                if (match[1].length() != match.length() - 4)
                    ErrorLogger() << "Positional error in key expansion: " << match[1] << " with length: " << match[1].length() << "and matchlen: " << match.length();
                // clear out any keywords that have been fully processed
                for (std::map< std::string, std::size_t >::iterator ref_check_it = cyclic_reference_check.begin(); 
                     ref_check_it != cyclic_reference_check.end(); )
                {
                    if (ref_check_it->second <= position) {
                        //DebugLogger() << "Popping from cyclic ref check: " << ref_check_it->first;
                        cyclic_reference_check.erase(ref_check_it++);
                    } else if (ref_check_it->second < position + match.length()) {
                        ErrorLogger() << "Expansion error in key expansion: [[" << ref_check_it->first << "]] having end " << ref_check_it->second;
                        ErrorLogger() << "         currently at expansion text position " << position << " with match length: " << match.length();
                        ErrorLogger() << "         of current expansion text:" << map_it->second;
                        ErrorLogger() << "         from keyword "<< map_it->first << " with raw text:" << rawtext;
                        ErrorLogger() << "         and cumulative substitions: " << cumulative_subsititions;
                        // will also trigger further error logging below
                        ref_check_it++;
                    } else
                        ref_check_it++;
                }
                if (cyclic_reference_check.find(match[1]) == cyclic_reference_check.end()) {
                    //DebugLogger() << "Pushing to cyclic ref check: " << match[1];
                    cyclic_reference_check[match[1]] = position + match.length();
                    std::map<std::string, std::string>::iterator map_lookup_it = m_strings.find(match[1]);
                    bool foundmatch = map_lookup_it != m_strings.end();
                    if (!foundmatch && lookups_fallback_table) {
                        DebugLogger() << "Key expansion: " << match[1] << " not found in primary stringtable: " << m_filename 
                                      << "; checking in fallback file" << fallback_table_file;
                        map_lookup_it = fallback_lookup_strings.find(match[1]);
                        foundmatch = map_lookup_it != fallback_lookup_strings.end();
                    }
                    if (foundmatch) {
                        const std::string substitution = map_lookup_it->second;
                        cumulative_subsititions += substitution + "|**|";
                        map_it->second.replace(position, match.length(), substitution);
                        std::size_t added_chars = substitution.length() - match.length();
                        for (std::map< std::string, std::size_t >::iterator ref_check_it = cyclic_reference_check.begin(); 
                            ref_check_it != cyclic_reference_check.end(); ref_check_it++)
                        {
                            ref_check_it->second += added_chars;
                        }
                        // replace recursively -- do not skip past substitution
                    } else {
                        ErrorLogger() << "Unresolved key expansion: " << match[1] << " in: " << m_filename << ".";
                        position += match.length();
                    }
                } else {
                    ErrorLogger() << "Cyclic key expansion: " << match[1] << " in: " << m_filename << "."
                                           << "         at expansion text position " << position;
                    ErrorLogger() << "         of current expansion text:" << map_it->second;
                    ErrorLogger() << "         from keyword "<< map_it->first << " with raw text:" << rawtext;
                    ErrorLogger() << "         and cumulative substitions: " << cumulative_subsititions;
                    position += match.length();
                }
            }
        }

        // nonrecursively replace references -- convert [[type REF]] to <type REF>string for REF</type>
        for (std::map<std::string, std::string>::iterator map_it = m_strings.begin();
             map_it != m_strings.end(); ++map_it)
        {
            std::size_t position = 0; // position in the definition string, past the already processed part
            smatch match;
            while (regex_search(map_it->second.begin() + position, map_it->second.end(), match, REFERENCE)) {
                position += match.position();
                std::map<std::string, std::string>::iterator map_lookup_it = m_strings.find(match[2]);
                bool foundmatch = map_lookup_it != m_strings.end();
                if (!foundmatch && lookups_fallback_table) {
                    DebugLogger() << "Key reference: " << match[2] << " not found in primary stringtable: " << m_filename 
                                  << "; checking in fallback file" << fallback_table_file;
                    map_lookup_it = fallback_lookup_strings.find(match[2]);
                    foundmatch = map_lookup_it != fallback_lookup_strings.end();
                }
                if (foundmatch) {
                    const std::string substitution =
                        '<' + match[1].str() + ' ' + match[2].str() + '>' + map_lookup_it->second + "</" + match[1].str() + '>';
                    map_it->second.replace(position, match.length(), substitution);
                    position += substitution.length();
                } else {
                    ErrorLogger() << "Unresolved reference: " << match[2] << " in: " << m_filename << ".";
                    position += match.length();
                }
            }
        }
    } else {
        ErrorLogger() << "StringTable file \"" << m_filename << "\" is malformed around line " << std::count(file_contents.begin(), it, '\n');
    }
}
