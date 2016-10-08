#include "StringTable.h"

#include "Logger.h"
#include "Directories.h"
#include "../parse/Parse.h"

#include <boost/filesystem/fstream.hpp>
#include <boost/xpressive/xpressive.hpp>
#include <boost/algorithm/string/replace.hpp>

#include <iostream>


namespace {
    const std::string DEFAULT_FILENAME = "en.txt";
    const std::string ERROR_STRING = "ERROR: ";
}


StringTable::StringTable():
    m_filename(DEFAULT_FILENAME)
{ Load(); }

StringTable::StringTable(const std::string& filename, std::shared_ptr<const StringTable> fallback):
    m_filename(filename)
{ Load(fallback); }

StringTable::~StringTable()
{}

bool StringTable::StringExists(const std::string& key) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_strings.count(key);
}

const std::string& StringTable::operator[] (const std::string& key) const {
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_strings.find(key);
    if (it != m_strings.end())
        return it->second;

    auto error = m_error_strings.insert(ERROR_STRING + key);
    return *(error.first);
}

void StringTable::Load(std::shared_ptr<const StringTable> fallback) {
    std::lock_guard<std::mutex> lock(m_mutex);

    if (fallback && !fallback->m_initialized) {
        // this prevents deadlock if two stringtables were to be loaded
        // simultaneously with eachother as fallback tables
        ErrorLogger() << "StringTable::Load given uninitialized stringtable as fallback. Ignoring.";
        fallback = nullptr;
    }

    auto path = FilenameToPath(m_filename);
    std::string file_contents;

    bool read_success = parse::read_file(path, file_contents);
    if (!read_success) {
        ErrorLogger() << "StringTable::Load failed to read file at path: " << path.string();
        //m_initialized intentionally left false
        return;
    }
    // add newline at end to avoid errors when one is left out, but is expected by parsers
    file_contents += "\n";

    parse::file_substitution(file_contents, path.parent_path());

    std::map<std::string, std::string> fallback_lookup_strings;
    std::string fallback_table_file;
    if (fallback) {
        std::lock_guard<std::mutex> fallback_lock(fallback->m_mutex);
        fallback_table_file = fallback->Filename();
        fallback_lookup_strings.insert(fallback->m_strings.begin(), fallback->m_strings.end());
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
    auto it = file_contents.begin();
    auto end = file_contents.end();

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
                for (auto match_it = matches.nested_results().begin();
                     match_it != matches.nested_results().end(); ++match_it)
                {
                    if (match_it->regex_id() == KEY.regex_id()) {
                        key = match_it->str();
                    } else if (match_it->regex_id() == SINGLE_LINE_VALUE.regex_id() ||
                               match_it->regex_id() == MULTI_LINE_VALUE.regex_id())
                    {
                        assert(key != "");
                        if (!m_strings.count(key)) {
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
        m_initialized = true;
        return;
    }

    if (well_formed) {
        // recursively expand keys -- replace [[KEY]] by the text resulting from expanding everything in the definition for KEY
        for (auto& entry : m_strings) {
            //DebugLogger() << "Checking key expansion for: " << entry.first;
            std::size_t position = 0; // position in the definition string, past the already processed part
            smatch match;
            std::map<std::string, std::size_t> cyclic_reference_check;
            cyclic_reference_check[entry.first] = entry.second.length();
            std::string rawtext = entry.second;
            std::string cumulative_subsititions;

            while (regex_search(entry.second.begin() + position, entry.second.end(), match, KEYEXPANSION)) {
                position += match.position();
                //DebugLogger() << "checking next internal keyword match: " << match[1] << " with matchlen " << match.length();
                if (match[1].length() != match.length() - 4)
                    ErrorLogger() << "Positional error in key expansion: " << match[1] << " with length: "
                                  << match[1].length() << "and matchlen: " << match.length();
                // clear out any keywords that have been fully processed
                for (auto ref_check_it = cyclic_reference_check.begin(); 
                     ref_check_it != cyclic_reference_check.end(); )
                {
                    if (ref_check_it->second <= position) {
                        //DebugLogger() << "Popping from cyclic ref check: " << ref_check_it->first;
                        ref_check_it = cyclic_reference_check.erase(ref_check_it);
                    } else if (ref_check_it->second < position + match.length()) {
                        ErrorLogger() << "Expansion error in key expansion: [[" << ref_check_it->first << "]] having end " << ref_check_it->second;
                        ErrorLogger() << "         currently at expansion text position " << position << " with match length: " << match.length();
                        ErrorLogger() << "         of current expansion text:" << entry.second;
                        ErrorLogger() << "         from keyword "<< entry.first << " with raw text:" << rawtext;
                        ErrorLogger() << "         and cumulative substitions: " << cumulative_subsititions;
                        // will also trigger further error logging below
                        ++ref_check_it;
                    } else
                        ++ref_check_it;
                }
                if (!cyclic_reference_check.count(match[1])) {
                    //DebugLogger() << "Pushing to cyclic ref check: " << match[1];
                    cyclic_reference_check[match[1]] = position + match.length();
                    auto map_lookup_it = m_strings.find(match[1]);
                    bool foundmatch = map_lookup_it != m_strings.end();
                    if (!foundmatch && !fallback_lookup_strings.empty()) {
                        DebugLogger() << "Key expansion: " << match[1] << " not found in primary stringtable: " << m_filename
                                      << "; checking in fallback file: " << fallback_table_file;
                        map_lookup_it = fallback_lookup_strings.find(match[1]);
                        foundmatch = map_lookup_it != fallback_lookup_strings.end();
                    }
                    if (foundmatch) {
                        const std::string substitution = map_lookup_it->second;
                        cumulative_subsititions += substitution + "|**|";
                        entry.second.replace(position, match.length(), substitution);
                        std::size_t added_chars = substitution.length() - match.length();
                        for (auto& ref_check : cyclic_reference_check) {
                            ref_check.second += added_chars;
                        }
                        // replace recursively -- do not skip past substitution
                    } else {
                        ErrorLogger() << "Unresolved key expansion: " << match[1] << " in: " << m_filename << ".";
                        position += match.length();
                    }
                } else {
                    ErrorLogger() << "Cyclic key expansion: " << match[1] << " in: " << m_filename << "."
                                           << "         at expansion text position " << position;
                    ErrorLogger() << "         of current expansion text:" << entry.second;
                    ErrorLogger() << "         from keyword "<< entry.first << " with raw text:" << rawtext;
                    ErrorLogger() << "         and cumulative substitions: " << cumulative_subsititions;
                    position += match.length();
                }
            }
        }

        // nonrecursively replace references -- convert [[type REF]] to <type REF>string for REF</type>
        for (auto& entry : m_strings) {
            std::size_t position = 0; // position in the definition string, past the already processed part
            smatch match;
            while (regex_search(entry.second.begin() + position, entry.second.end(), match, REFERENCE)) {
                position += match.position();
                auto map_lookup_it = m_strings.find(match[2]);
                bool foundmatch = map_lookup_it != m_strings.end();
                if (!foundmatch && !fallback_lookup_strings.empty()) {
                    DebugLogger() << "Key reference: " << match[2] << " not found in primary stringtable: " << m_filename 
                                  << "; checking in fallback file" << fallback_table_file;
                    map_lookup_it = fallback_lookup_strings.find(match[2]);
                    foundmatch = map_lookup_it != fallback_lookup_strings.end();
                }
                if (foundmatch) {
                    const std::string substitution =
                        '<' + match[1].str() + ' ' + match[2].str() + '>' + map_lookup_it->second + "</" + match[1].str() + '>';
                    entry.second.replace(position, match.length(), substitution);
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

    m_initialized = true;
}
