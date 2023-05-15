#include "OptionsDB.h"

#include "Directories.h"
#include "i18n.h"
#include "Logger.h"
#include "OptionValidators.h"
#include "XMLDoc.h"

#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <string>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/erase.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/range/algorithm_ext/erase.hpp>
#include <boost/tokenizer.hpp>

namespace {
    std::vector<OptionsDBFn>& OptionsRegistry() noexcept {
        static std::vector<OptionsDBFn> options_db_registry;
        return options_db_registry;
    }

    std::string PreviousSectionName(const std::vector<XMLElement*>& elem_stack) {
        std::string retval;
        for (unsigned int i = 1; i < elem_stack.size(); ++i) {
            retval += elem_stack[i]->Tag();
            if (i != elem_stack.size() - 1)
                retval += '.';
        }
        return retval;
    }

    std::string_view StripQuotation(std::string_view str) {
        if (str.size() < 2 || str[0] != '"' || str[str.size() - 1] != '"')
            return str;
        return str.substr(1, str.size() - 2);
    }

    ///< the master list of abbreviated option names, and their corresponding long-form names
    std::map<char, std::string> short_names;

    OptionsDB options_db;
}

/////////////////////////////////////////////
// Free Functions
/////////////////////////////////////////////
bool RegisterOptions(OptionsDBFn function) {
    OptionsRegistry().emplace_back(function);
    return true;
}

OptionsDB& GetOptionsDB() {
    if (!OptionsRegistry().empty()) {
        for (OptionsDBFn fn : OptionsRegistry())
            fn(options_db);
        OptionsRegistry().clear();
    }
    return options_db;
}


/////////////////////////////////////////////
// OptionsDB::Option
/////////////////////////////////////////////
OptionsDB::Option::Option(char short_name_, std::string name_, boost::any value_,
                          boost::any default_value_, std::string description_,
                          std::unique_ptr<ValidatorBase>&& validator_, bool storable_,
                          bool flag_, bool recognized_, std::string section) :
    name(std::move(name_)),
    short_name(short_name_),
    storable(storable_),
    flag(flag_),
    recognized(recognized_),
    value(std::move(value_)),
    default_value(std::move(default_value_)),
    description(std::move(description_)),
    validator(std::move(validator_))
{
    if (!validator)
        DebugLogger() << "Option " << name << " created with null validator...";

    auto name_it = name.rfind('.');
    if (name_it != std::string::npos)
        sections.insert(name.substr(0, name_it));

    if (short_name_) {
        auto [insertion_it, insertion_succeeded] = short_names.emplace(short_name_, name);
        if (!insertion_succeeded)
            ErrorLogger() << "Tried to insert short name " << short_name << " for option " << name
                          << " but that short name was already assigned to option " << insertion_it->second;
    }

    if (!section.empty())
        sections.insert(std::move(section));
    else if (sections.empty())
        sections.emplace("misc");
}

OptionsDB::Option::~Option() = default;

bool OptionsDB::Option::SetFromString(std::string_view str) {
    bool changed = false;
    boost::any value_;

    if (!flag) {
        if (validator) {
            value_ = validator->Validate(str);
            changed = validator->String(value) != validator->String(value_);
        } else {
            throw std::runtime_error("Option::SetFromString called with no OptionValidator set");
        }
    } else {
        value_ = boost::lexical_cast<bool>(str);    // if a flag, then the str parameter should just indicate true or false with "1" or "0"
        changed = (std::to_string(boost::any_cast<bool>(value))
                   != std::to_string(boost::any_cast<bool>(value_)));
    }

    if (changed) {
        value = std::move(value_);
        option_changed_sig();
    }
    return changed;
}

bool OptionsDB::Option::SetToDefault() {
    bool changed = !ValueIsDefault();
    if (changed) {
        value = default_value;
        option_changed_sig();
    }
    return changed;
}

std::string OptionsDB::Option::ValueToString() const {
    if (flag) {
        return std::to_string(boost::any_cast<bool>(value));
    } else if (validator)
        return validator->String(value);
    else
        throw std::runtime_error("Option::ValueToString called with no Validator set");
}

std::string OptionsDB::Option::DefaultValueToString() const {
    if (flag)
        return std::to_string(boost::any_cast<bool>(default_value));
    else if (validator)
        return validator->String(default_value);
    else
        throw std::runtime_error("Option::DefaultValueToString called with no Validator set");
}

bool OptionsDB::Option::ValueIsDefault() const
{ return ValueToString() == DefaultValueToString(); }


/////////////////////////////////////////////
// OptionsDB
/////////////////////////////////////////////
bool OptionsDB::Commit(bool only_if_dirty, bool only_non_default) {
    if (only_if_dirty && !m_dirty)
        return true;
    boost::filesystem::ofstream ofs(GetConfigPath());
    if (ofs) {
        XMLDoc doc;
        GetOptionsDB().GetXML(doc, only_non_default, true);
        doc.WriteDoc(ofs);
        m_dirty = false;
        return true;
    } else {
        std::cerr << UserString("UNABLE_TO_WRITE_CONFIG_XML") << std::endl;
        std::cerr << PathToString(GetConfigPath()) << std::endl;
        ErrorLogger() << UserString("UNABLE_TO_WRITE_CONFIG_XML");
        ErrorLogger() << PathToString(GetConfigPath());
        return false;
    }
}

bool OptionsDB::CommitPersistent() {
    bool retval = false;
    auto config_file = GetPersistentConfigPath();
    XMLDoc doc;
    GetOptionsDB().GetXML(doc, true, false);   // only output non-default options
    try {
        // Remove any previously existing file
        boost::filesystem::remove(config_file);

        boost::filesystem::ofstream ofs(GetPersistentConfigPath());
        if (ofs) {
            doc.WriteDoc(ofs);
            retval = true;
        } else {
            std::string err_msg = UserString("UNABLE_TO_WRITE_PERSISTENT_CONFIG_XML") + " : " + config_file.string();
            ErrorLogger() << err_msg;
            std::cerr << err_msg << std::endl;
        }
    } catch (const boost::filesystem::filesystem_error& ec) {
        ErrorLogger() << "Error during file operations when creating persistent config : " << ec.what();
    } catch (...) {
        std::string err_msg = "Unknown exception during persistent config creation";
        ErrorLogger() << err_msg;
        std::cerr << err_msg << std::endl;
    }

    return retval;
}

void OptionsDB::Validate(std::string_view name, std::string_view value) const {
    auto it = m_options.find(name);
    if (!OptionExists(it))
        throw std::runtime_error(std::string{"Attempted to validate unknown option \""}.append(name).append("\"."));

    if (it->second.flag)
        boost::lexical_cast<bool>(value);
    else if (it->second.validator)
        it->second.validator->Validate(value);
    else
        throw std::runtime_error("Attempted to validate option with no validator set");
}

std::string OptionsDB::GetValueString(std::string_view option_name) const {
    auto it = m_options.find(option_name);
    if (!OptionExists(it))
        throw std::runtime_error(std::string{"OptionsDB::GetValueString(): No option called \""}.append(option_name).append("\" could be found."));
    return it->second.ValueToString();
}

std::string OptionsDB::GetDefaultValueString(std::string_view option_name) const {
    auto it = m_options.find(option_name);
    if (!OptionExists(it))
        throw std::runtime_error(std::string{"OptionsDB::GetDefaultValueString(): No option called \""}.append(option_name).append("\" could be found."));
    return it->second.DefaultValueToString();
}

const std::string& OptionsDB::GetDescription(std::string_view option_name) const {
    auto it = m_options.find(option_name);
    if (!OptionExists(it))
        throw std::runtime_error(std::string{"OptionsDB::GetDescription(): No option called \""}.append(option_name).append("\" could be found."));
    return it->second.description;
}

const ValidatorBase* OptionsDB::GetValidator(std::string_view option_name) const {
    auto it = m_options.find(option_name);
    if (!OptionExists(it))
        throw std::runtime_error(std::string{"OptionsDB::GetValidator(): No option called \""}.append(option_name).append("\" could be found."));
    return it->second.validator.get();
}

namespace {
    constexpr std::size_t TERMINAL_LINE_WIDTH = 80;

    constexpr std::string_view lexical_true_str = "1"; // = std::to_string(true);

    /** Breaks and indents text over multiple lines when it exceeds width limits
     * @param text String to format, tokenized by spaces, tabs, and newlines (newlines retained but potentially indented)
     * @param indents amount of space prior to text. First for initial line, second for any new lines.
     * @param widths width to limit the text to. First for initial line, second for any new lines.
     * @returns string Formatted results of @p text
     */
    std::string SplitText(const std::string& text, std::pair<std::size_t, std::size_t> indents = { 0, 0 },
                          std::pair<std::size_t, std::size_t> widths = { TERMINAL_LINE_WIDTH, TERMINAL_LINE_WIDTH })
    {
        boost::char_separator<char> separator { " \t", "\n" };
        boost::tokenizer<boost::char_separator<char>> tokens { text, separator };

        std::vector<std::string> lines{""};
        for (const auto& token : tokens) {
            if (token == "\n") 
                lines.emplace_back();
            else if (widths.second < lines.back().size() + token.size() + indents.second)
                lines.push_back(token + " ");
            else if (!token.empty())
                lines.back().append(token + " ");
        }

        std::string indent(indents.second, ' ');
        std::stringstream retval;
        auto first_line = std::move(lines.front());
        retval << std::string(indents.first, ' ') << first_line << "\n";
        for (const auto& line : lines)
            if (!line.empty())
                retval << indent << line << "\n";

        return retval.str();
    }

    bool OptionNameHasParentSection(std::string_view lhs, std::string_view rhs) {
        auto it = lhs.find_last_of('.');
        if (it == std::string::npos)
            return false;
        return lhs.substr(0, it) == rhs;
    }
}

std::unordered_map<std::string_view, std::set<std::string_view>> OptionsDB::OptionsBySection(bool allow_unrecognized) const {
    // Determine sections after all predicate calls from known options
    std::unordered_map<std::string_view, std::unordered_set<std::string_view>> sections_by_option;
    for (const auto& [option_name, option] : m_options) {
        if (!allow_unrecognized && !option.recognized)
            continue;

        for (const auto& section : option.sections)
            sections_by_option[option_name].emplace(section);

        for (auto& section : m_sections)
            if (section.second.option_predicate && section.second.option_predicate(option_name))
                sections_by_option[option_name].insert(section.first);
    }

    // tally the total number of options under each section
    std::unordered_map<std::string_view, std::size_t> total_options_per_section;
    for (const auto& [option_name, sections_set] : sections_by_option) {
        (void)sections_set; // quiet warning
        auto dot_it = option_name.find_first_of(".");
        // increment count of each containing parent section
        while (dot_it != std::string::npos) {
            total_options_per_section[option_name.substr(0, dot_it)]++;
            dot_it++;
            dot_it = option_name.find_first_of(".", dot_it);
        }
    }

    // sort options into common sections
    std::unordered_map<std::string_view, std::set<std::string_view>> options_by_section;
    for (const auto& [option_name, sections_set] : sections_by_option) {
        for (std::string_view section_name : sections_set) {
            auto defined_section_it = m_sections.find(section_name);
            bool has_descr = defined_section_it != m_sections.end() ?
                             !defined_section_it->second.description.empty() :
                             false;

            // move options from sparse sections to more common parent
            auto section_count = total_options_per_section[section_name];
            auto section_end_it = section_name.find_last_of(".");
            while (!has_descr && section_count < 4 && section_end_it != std::string::npos) {
                auto new_section_name = section_name.substr(0, section_end_it);
                // prevent moving into dense sections
                if (total_options_per_section[new_section_name] > ( 7 - section_count ))
                    break;
                total_options_per_section[section_name]--;
                section_name = new_section_name;
                section_end_it = section_name.find_last_of(".");
                section_count = total_options_per_section[section_name];

                defined_section_it = m_sections.find(section_name);
                if (defined_section_it != m_sections.end())
                    has_descr = !defined_section_it->second.description.empty();
            }

            options_by_section[section_name].emplace(option_name);
        }
    }

    // define which section are top level sections ("root"), move top level candidates with single option to misc
    for (auto& [section_name, option_count] : total_options_per_section) {
        (void)option_count; // quiet warning
        auto root_name = section_name.substr(0, section_name.find_first_of("."));
        // root_name with no dot element allowed to pass if an option is known, potentially moving to misc section
        auto total_it = total_options_per_section.find(root_name);
        if (total_it == total_options_per_section.end())
            continue;
        auto count = total_it->second;

        if (count > 1)
            options_by_section["root"].insert(std::move(root_name));

        else if (section_name != "misc" && section_name != "root" && !m_sections.contains(section_name)) {
            // no section found with specified name, so move options in section to misc section
            auto section_options_it = options_by_section.find(section_name);
            if (section_options_it == options_by_section.end())
                continue;
            auto& section_options = section_options_it->second;

            for (auto& option : section_options)
                options_by_section["misc"].insert(option);
            options_by_section.erase(section_name);
        }
    }

    return options_by_section;
}

void OptionsDB::GetUsage(std::ostream& os, std::string_view command_line, bool allow_unrecognized) const {
    // Prevent logger output from garbling console display for low severity messages
    OverrideAllLoggersThresholds(LogLevel::warn);

    auto options_by_section = OptionsBySection(allow_unrecognized);
    if (!command_line.empty() || command_line == "all" || command_line == "raw") {
        // remove the root section if unneeded
        if (options_by_section.contains("root"))
            options_by_section.erase("root");
    }

    if (command_line.empty()) {
        if (UserStringExists("COMMAND_LINE_HELP_GENERAL_DESCRIPTION"))
            os << UserString("COMMAND_LINE_HELP_GENERAL_DESCRIPTION") << "\n";
        else
            os << "\nFreeOrion is a 4X Space Strategy game.\n\n";
    }

    // print description of command_line arg as section
    if (command_line == "all") {
        os << UserString("OPTIONS_DB_SECTION_ALL") << " ";
    } else if (command_line == "raw") {
        os << UserString("OPTIONS_DB_SECTION_RAW") << " ";
    } else {
        auto command_section_it = m_sections.find(command_line);
        if (command_section_it != m_sections.end() && !command_section_it->second.description.empty())
            os << UserString(command_section_it->second.description) << " ";
    }

    bool print_misc_section = command_line.empty();
    std::set<std::string_view> section_list;
    // print option sections
    if (command_line != "all" && command_line != "raw") {
        std::size_t name_col_width = 20;
        if (command_line.empty()) {
            const auto root_it = options_by_section.find("root");
            if (root_it != options_by_section.end()) {
                for (const auto& section : root_it->second)
                    if (section.find_first_of(".") == std::string::npos)
                        if (section_list.emplace(section).second && name_col_width < section.size())
                            name_col_width = section.size();
            }
        } else {
            for (const auto& [sec, opts] : options_by_section) {
                (void)opts; // ignored
                if (OptionNameHasParentSection(sec, command_line))
                    if (section_list.emplace(sec).second && name_col_width < sec.size())
                        name_col_width = sec.size();
            }
        }
        name_col_width += 5;

        if (!section_list.empty())
            os << UserString("COMMAND_LINE_SECTIONS") << ":\n";

        const auto indents = std::pair(2, name_col_width + 4);
        const auto widths = std::pair(TERMINAL_LINE_WIDTH - name_col_width, TERMINAL_LINE_WIDTH);
        for (std::string_view section : section_list) {
            if (section == "misc") {
                print_misc_section = true;
                continue;
            }
            const auto section_it = m_sections.find(section);
            std::string descr = (section_it == m_sections.end()) ? "" : UserString(section_it->second.description);

            os << std::setw(2) << "" // indent
               << std::setw(name_col_width) << std::left << section // section name
               << SplitText(descr, indents, widths); // section description
        }

        if (print_misc_section) {
            // Add special miscellaneous section to bottom
            os << std::setw(2) << "" << std::setw(name_col_width) << std::left << "misc";
            os << SplitText(UserString("OPTIONS_DB_SECTION_MISC"), indents, widths);
        }

        // add empty line between groups and options
        if (!section_list.empty() && !print_misc_section)
            os << "\n";
    }


    // print options
    if (!command_line.empty()) {
        std::vector<std::string_view> option_list;
        if (command_line == "all" || command_line == "raw") {
            for (const auto& [sec, opts] : options_by_section) {
                (void)sec; // ignored
                for (const auto& option : opts)
                    option_list.push_back(option);
            }
        } else {
            auto option_section_it = options_by_section.find(command_line);
            if (option_section_it != options_by_section.end())
                option_list = {option_section_it->second.begin(), option_section_it->second.end()};
            // allow traversal by node when no other results are found
            if (option_list.empty() && section_list.empty())
                option_list = FindOptions(command_line, allow_unrecognized);
        }

        // insert command_line as option, if it exists
        if (command_line != "all" && command_line != "raw" && m_options.count(command_line))
            option_list.push_back(command_line);

        if (!option_list.empty())
            os << UserString("COMMAND_LINE_OPTIONS") << ":\n";

        for (std::string_view option_name : option_list) {
            const auto option_it = m_options.find(option_name);
            if (option_it == m_options.end() || (!allow_unrecognized && !option_it->second.recognized))
                continue;

            if (command_line == "raw") {
                os << option_name << ", " << option_it->second.description << ",\n";
                if (option_it->second.short_name)
                    os << option_it->second.short_name << ", " << option_it->second.description << ",\n";
            } else {
                // option name(s)
                if (option_it->second.short_name)
                    os << "-" << option_it->second.short_name << " | --" << option_name;
                else
                    os << "--" << option_name;

                // option description
                if (!option_it->second.description.empty())
                    os << "\n" << SplitText(UserString(option_it->second.description), {5, 7});
                else
                    os << "\n";

                // option default value
                if (option_it->second.validator) {
                    const auto validator_str = UserString("COMMAND_LINE_DEFAULT") + ": " + option_it->second.DefaultValueToString();
                    os << SplitText(validator_str, {5, 7}, {TERMINAL_LINE_WIDTH - validator_str.size(), 77});
                }
                os << "\n";
            }
        }

        if (section_list.empty() && option_list.empty()) {
            os << UserString("COMMAND_LINE_NOT_FOUND") << ": " << command_line << "\n\n";
            os << UserString("COMMAND_LINE_USAGE") << "\n";
        }
    }

    // reset override in case this function is later repurposed
    OverrideAllLoggersThresholds(boost::none);
}

void OptionsDB::GetXML(XMLDoc& doc, bool non_default_only, bool include_version) const {
    doc = XMLDoc();

    std::vector<XMLElement*> elem_stack;
    elem_stack.push_back(&doc.root_node);

    for (const auto& option : m_options) {
        if (!option.second.storable)
            continue;

        if (!option.second.recognized)
            continue;

        std::string::size_type last_dot = option.first.find_last_of('.');
        std::string section_name = last_dot == std::string::npos ? "" : option.first.substr(0, last_dot);
        std::string name = option.first.substr(last_dot == std::string::npos ? 0 : last_dot + 1);

        // "version.gl.check.done" is automatically set to true after other logic is performed
        if (option.first == "version.gl.check.done")
            continue;

        // Skip unwanted config options
        // BUG Some windows may be shown as a child of an other window, but not initially visible.
        //   The OptionDB default of "*.visible" in these cases may be false, but setting the option to false
        //   in a config file may prevent such windows from showing when requested.
        if (name == "visible")
            continue;

        // Storing "version.string" in persistent config would render all config options invalid after a new build
        if (!include_version && option.first == "version.string")
            continue;

        // do want to store version string if requested, regardless of whether
        // it is default. for other strings, if storing non-default only,
        // check if option is default and if it is, skip it.
        if (non_default_only && option.first != "version.string") {
            bool is_default_nonflag = !option.second.flag && IsDefaultValue(m_options.find(option.first));
            if (is_default_nonflag)
                continue;

            // Default value of flag options will throw bad_any_cast, fortunately they always default to false
            if (option.second.flag && !boost::any_cast<bool>(option.second.value))
                continue;
        }


        while (1 < elem_stack.size()) {
            std::string prev_section = PreviousSectionName(elem_stack);
            if (prev_section == section_name) {
                section_name.clear();
                break;
            } else if (section_name.find(prev_section + '.') == 0) {
                section_name = section_name.substr(prev_section.size() + 1);
                break;
            }
            elem_stack.pop_back();
        }
        if (!section_name.empty()) {
            std::string::size_type last_pos = 0;
            std::string::size_type pos = 0;
            while ((pos = section_name.find('.', last_pos)) != std::string::npos) {
                XMLElement temp(section_name.substr(last_pos, pos - last_pos));
                elem_stack.back()->children.push_back(temp);
                elem_stack.push_back(&elem_stack.back()->Child(temp.Tag()));
                last_pos = pos + 1;
            }
            XMLElement temp(section_name.substr(last_pos));
            elem_stack.back()->children.push_back(temp);
            elem_stack.push_back(&elem_stack.back()->Child(temp.Tag()));
        }

        XMLElement temp(name);
        if (option.second.validator) {
            temp.SetText(option.second.ValueToString());
        } else if (option.second.flag) {
            if (!boost::any_cast<bool>(option.second.value))
                continue;
        }
        elem_stack.back()->children.push_back(temp);
        elem_stack.push_back(&elem_stack.back()->Child(temp.Tag()));
    }
}

OptionsDB::OptionChangedSignalType& OptionsDB::OptionChangedSignal(std::string_view option) {
    //DebugLogger() << "getting option changed signal for string" << option;
    auto it = m_options.find(option);
    if (it == m_options.end())
        throw std::runtime_error(std::string{"OptionsDB::OptionChangedSignal() : Attempted to get signal for nonexistent option \""}
                                 .append(option).append("\"."));
    return it->second.option_changed_sig;
}

void OptionsDB::Remove(std::string_view name) {
    auto it = m_options.find(name);
    if (it != m_options.end()) {
        short_names.erase(it->second.short_name);
        m_options.erase(it);
        m_dirty = true;
    }
}

void OptionsDB::RemoveUnrecognized(std::string_view prefix) {
    auto it = m_options.begin();
    while (it != m_options.end()) {
        if (!it->second.recognized && it->first.find(prefix) == 0)
            Remove((it++)->first); // note postfix operator++
        else
            ++it;
    }
}

void OptionsDB::FindOptions(std::set<std::string>& ret, std::string_view prefix, bool allow_unrecognized) const {
    ret.clear();
    for (auto& [option_name, option] : m_options)
        if ((option.recognized || allow_unrecognized) && option_name.find(prefix) == 0)
            ret.insert(option_name);
}

std::vector<std::string_view> OptionsDB::FindOptions(std::string_view prefix, bool allow_unrecognized) const {
    std::vector<std::string_view> ret;
    ret.reserve(m_options.size());
    for (auto& [option_name, option] : m_options)
        if ((option.recognized || allow_unrecognized) && option_name.find(prefix) == 0)
            ret.push_back(option_name);
    return ret;
}

void OptionsDB::SetToDefault(std::string_view name) {
    auto it = m_options.find(name);
    if (!OptionExists(it))
        throw std::runtime_error("Attempted to reset value of nonexistent option \"" + std::string{name});
    it->second.value = it->second.default_value;
}

void OptionsDB::SetFromCommandLine(const std::vector<std::string>& args) {
    //bool option_changed = false;

    for (unsigned int i = 1; i < args.size(); ++i) {
        std::string_view current_token{args[i]};

        if (current_token.find("--") == 0) {
            std::string option_name{current_token.substr(2)}; // need a string here as there is no unordered_map heterogeneous lookup :(

            if (option_name.empty())
                throw std::runtime_error("A \'--\' was given with no option name.");

            auto it = m_options.find(option_name); 

            if (it == m_options.end() || !it->second.recognized) {
                // unrecognized option: may be registered later on so we'll store it for now
                // Check for more parameters (if this is the last one, assume that it is a flag).
                std::string_view value_str{"-"};
                if (i + 1 < static_cast<unsigned int>(args.size()))
                    value_str = StripQuotation(args[i + 1]);

                if (value_str.front() == '-') {
                    // this is either the last parameter or the next parameter is another option, assume this one is a flag
                    m_options.emplace(option_name,
                                      Option{static_cast<char>(0), option_name, true, false, "",
                                             std::make_unique<Validator<bool>>(), false, true, false});
                } else {
                    // the next parameter is the value, store it as a string to be parsed later, but
                    // don't attempt to store options that have only been specified on the command line
                    m_options.emplace(option_name,
                                      Option{static_cast<char>(0), option_name, value_str, value_str, "",
                                             std::make_unique<Validator<std::string>>(), false, false, false});
                }

                WarnLogger() << "Option \"" << option_name << "\", was specified on the command line but was not recognized."
                             << " It may not be registered yet or could be a typo.";


            } else {
                // recognized option
                Option& option = it->second;
                if (option.value.empty())
                    throw std::runtime_error("The value member of option \"--" + option.name + "\" is undefined.");

                if (!option.flag) { // non-flag
                    try {
                        // check if parameter exists...
                        if (i + 1 >= static_cast<unsigned int>(args.size())) {
                            m_dirty |= option.SetFromString("");
                            continue;
                        }
                        // get parameter value
                        std::string_view value_str = StripQuotation(args[++i]);

                        // ensure parameter is actually a parameter, and not the next option name (which would indicate
                        // that the option was specified without a parameter value, as if it was a flag)
                        if (!value_str.empty() && value_str.front() == '-')
                            throw std::runtime_error("the option \"" + option.name +
                                                     "\" was followed by the parameter \"" + std::string{value_str} +
                                                     "\", which appears to be an option flag, not a parameter value, because it begins with a \"-\" character.");
                        m_dirty |= option.SetFromString(value_str);
                    } catch (const std::exception& e) {
                        throw std::runtime_error("OptionsDB::SetFromCommandLine() : the following exception was caught when attempting to set option \"" + option.name + "\": " + e.what() + "\n\n");
                    }
                } else { // flag
                    option.value = true;
                }
            }

            //option_changed = true;
        } else if (current_token.find('-') == 0
#ifdef FREEORION_MACOSX
                && current_token.find("-psn") != 0 // Mac OS X passes a process serial number to all applications using Carbon or Cocoa, it should be ignored here
#endif
            )
        {
            auto single_char_options = current_token.substr(1);

            if (single_char_options.empty())
                throw std::runtime_error("A \'-\' was given with no options.");

            for (unsigned int j = 0; j < single_char_options.size(); ++j) {
                auto short_name_it = short_names.find(single_char_options[j]);

                if (short_name_it == short_names.end())
                    throw std::runtime_error(std::string("Unknown option \"-") + single_char_options[j] + "\" was given.");

                auto name_it = m_options.find(short_name_it->second);

                if (name_it == m_options.end())
                    throw std::runtime_error("Option \"--" + short_name_it->second + "\", abbreviated as \"-" + short_name_it->first + "\", could not be found.");

                Option& option = name_it->second;
                if (option.value.empty())
                    throw std::runtime_error("The value member of option \"--" + option.name + "\" is undefined.");

                if (!option.flag) {
                    if (j < single_char_options.size() - 1) {
                        throw std::runtime_error(std::string("Option \"-") + single_char_options[j] + "\" was given with no parameter.");
                    } else {
                        if (i + 1 >= static_cast<unsigned int>(args.size()))
                            m_dirty |= option.SetFromString("");
                        else
                            m_dirty |= option.SetFromString(args[++i]);
                    }
                } else {
                    option.value = true;
                }
            }
        }
    }
}

void OptionsDB::SetFromFile(const boost::filesystem::path& file_path, std::string_view version) {
    XMLDoc doc;
    try {
        boost::filesystem::ifstream ifs(file_path);
        if (ifs) {
            doc.ReadDoc(ifs);
            if (version.empty() || (doc.root_node.ContainsChild("version") &&
                                    doc.root_node.Child("version").ContainsChild("string") &&
                                    version == doc.root_node.Child("version").Child("string").Text()))
            { GetOptionsDB().SetFromXML(doc); }
        }
    } catch (...) {
        std::cerr << UserString("UNABLE_TO_READ_CONFIG_XML")  << ": "
                  << file_path << std::endl;
    }
}

void OptionsDB::SetFromXML(const XMLDoc& doc) {
    for (const XMLElement& child : doc.root_node.children)
    { SetFromXMLRecursive(child, ""); }
}

void OptionsDB::SetFromXMLRecursive(const XMLElement& elem, std::string_view section_name) {
    std::string option_name = std::string{section_name}.append(section_name.empty() ? "" : ".").append(elem.Tag());
    if (option_name == "version.string")
        return;

    if (!elem.children.empty()) {
        for (const XMLElement& child : elem.children)
            SetFromXMLRecursive(child, option_name);
    }

    auto it = m_options.find(option_name);

    if (it == m_options.end() || !it->second.recognized) {

        TraceLogger() << "Option \"" << option_name << "\", was in config.xml but was not recognized."
                      << " It may not be registered yet or you may need to delete your config.xml if it is out of date.";

        if (elem.Text().length() == 0) {
            // do not retain empty XML options
            return;
        } else {
            // Store unrecognized option to be parsed later if this options is added.
            Option option{static_cast<char>(0), option_name, elem.Text(), elem.Text(), "",
                          std::make_unique<Validator<std::string>>(), true, false, false,
                          std::string{section_name}};
            m_options.emplace(std::move(option_name), std::move(option));
        }

        m_dirty = true;
        return;
    }

    Option& option = it->second;
    //if (!option.flag && option.value.empty()) {
    //    ErrorLogger() << "The value member of option \"" << option.name << "\" in config.xml is undefined.";
    //    return;
    //}

    if (option.flag) {
        option.value = static_cast<bool>(elem.Text() == lexical_true_str);
    } else {
        try {
            m_dirty |= option.SetFromString(elem.Text());
        } catch (const std::exception& e) {
            ErrorLogger() << "OptionsDB::SetFromXMLRecursive() : while processing config.xml the following exception was caught when attempting to set option \""
                          << option_name << "\" to \"" << elem.Text() << "\": " << e.what();
        }
    }
}

void OptionsDB::AddSection(const char* name, std::string description,
                           std::function<bool (const std::string&)> option_predicate)
{
    auto insert_result = m_sections.emplace(name, OptionSection{name, description, option_predicate});
    // if previously existing section, update description/predicate if empty/null
    if (!insert_result.second) {
        if (!description.empty() && insert_result.first->second.description.empty())
            insert_result.first->second.description = std::move(description);
        if (option_predicate != nullptr && insert_result.first->second.option_predicate == nullptr)
            insert_result.first->second.option_predicate = std::move(option_predicate);
    }
}

template <>
std::vector<std::string> OptionsDB::Get<std::vector<std::string>>(std::string_view name) const
{
    auto it = m_options.find(name);
    if (!OptionExists(it))
        throw std::runtime_error(std::string{"OptionsDB::Get<std::vector<std::string>>() : Attempted to get nonexistent option: "}.append(name));
    try {
        return boost::any_cast<std::vector<std::string>>(it->second.value);
    } catch (const boost::bad_any_cast&) {
        ErrorLogger() << "bad any cast converting value option named: " << name << ". Returning default value instead";
        try {
            return boost::any_cast<std::vector<std::string>>(it->second.default_value);
        } catch (const boost::bad_any_cast&) {
            ErrorLogger() << "bad any cast converting default value of std::vector<std::string> option named: " << name << ". Returning empty vector instead";
            return std::vector<std::string>();
        }
    }
}

std::string ListToString(std::vector<std::string> input_list) {
    // list input strings in comma-separated-value format
    std::string retval;
    retval.reserve(20*input_list.size()); // guesstimate
    unsigned int count = 0;
    for (auto& input_string : input_list) {
        if (count++ > 0)
            retval += ",";

        // remove XML protected characters and a few other semi-randomly chosen
        // characters to avoid corrupting enclosing XML document structure
        boost::remove_erase_if(input_string, boost::is_any_of("<&>'\",[]|\a\b\f\n\r\t\b"));
        retval.append(input_string);
    }
    return retval;
}

std::vector<std::string> StringToList(std::string_view input_string) {
    std::vector<std::string> retval;
    retval.reserve(5); // guesstimate
    static constexpr std::string_view separator{","};

    // adapted from https://github.com/fenbf/StringViewTests/blob/7e2e4c4f17dda2ed4569cca409fa2f02e0ef77ad/StringViewTest.cpp#L151
    std::string_view::const_pointer first = input_string.data();
    std::string_view::const_pointer second = nullptr;
    std::string_view::const_pointer last = first + input_string.size();
    // some tokens,delimited by commas,as an example
    // f                                            l

    for (; second != last && first != last; first = second + 1) {
        // some tokens,delimited by commas,as an example
        //            sf                                l
        second = std::find_first_of(
            first, last, std::cbegin(separator), std::cend(separator));
        // some tokens,delimited by commas,as an example
        //             f                  s             l
        if (first != second)
            retval.emplace_back(first, second - first);
    }

    return retval;
}

std::vector<std::string> StringToList(const char* input_string)
{ return StringToList(std::string_view{input_string}); }

std::vector<std::string> StringToList(const std::string& input_string) {
    using Tokenizer = boost::tokenizer<boost::char_separator<char>>;
    static const boost::char_separator<char> separator{","};
    Tokenizer tokens{input_string, separator};
    return {tokens.begin(), tokens.end()};
}
