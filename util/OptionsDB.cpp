#include "OptionsDB.h"

#include "i18n.h"
#include "Logger.h"
#include "OptionValidators.h"
#include "XMLDoc.h"

#include "util/Directories.h"

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
    std::vector<OptionsDBFn>& OptionsRegistry() {
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

    void StripQuotation(std::string& str) {
        using namespace boost::algorithm;
        if (starts_with(str, "\"") && ends_with(str, "\"")) {
            erase_first(str, "\"");
            erase_last(str, "\"");
        }
    }
}

/////////////////////////////////////////////
// Free Functions
/////////////////////////////////////////////
bool RegisterOptions(OptionsDBFn function) {
    OptionsRegistry().push_back(function);
    return true;
}

OptionsDB& GetOptionsDB() {
    static OptionsDB options_db;
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
// static(s)
std::map<char, std::string> OptionsDB::Option::short_names;

OptionsDB::Option::Option()
{}

OptionsDB::Option::Option(char short_name_, const std::string& name_, const boost::any& value_,
                          const boost::any& default_value_, const std::string& description_,
                          const ValidatorBase* validator_, bool storable_, bool flag_, bool recognized_,
                          const std::string& section) :
    name(name_),
    short_name(short_name_),
    value(value_),
    default_value(default_value_),
    description(description_),
    validator(validator_),
    storable(storable_),
    flag(flag_),
    recognized(recognized_),
    option_changed_sig_ptr(new boost::signals2::signal<void ()>())
{
    if (short_name_)
        short_names[short_name_] = name;

    auto name_it = name.rfind('.');
    if (name_it != std::string::npos)
        sections.emplace(name.substr(0, name_it));

    if (!section.empty())
        sections.emplace(section);
    else if (sections.empty())
        sections.emplace("misc");
}

bool OptionsDB::Option::SetFromString(const std::string& str) {
    bool changed = false;
    boost::any value_;

    if (!flag) {
        value_ = validator->Validate(str);
        changed = validator->String(value) != validator->String(value_);
    } else {
        value_ = boost::lexical_cast<bool>(str);    // if a flag, then the str parameter should just indicate true or false with "1" or "0"
        changed = (boost::lexical_cast<std::string>(boost::any_cast<bool>(value))
                   != boost::lexical_cast<std::string>(boost::any_cast<bool>(value_)));
    }

    if (changed) {
        value = std::move(value_);
        (*option_changed_sig_ptr)();
    }
    return changed;
}

bool OptionsDB::Option::SetToDefault() {
    bool changed = !ValueIsDefault();
    if (changed) {
        value = default_value;
        (*option_changed_sig_ptr)();
    }
    return changed;
}

std::string OptionsDB::Option::ValueToString() const {
    if (!flag)
        return validator->String(value);
    else
        return boost::lexical_cast<std::string>(boost::any_cast<bool>(value));
}

std::string OptionsDB::Option::DefaultValueToString() const {
    if (!flag)
        return validator->String(default_value);
    else
        return boost::lexical_cast<std::string>(boost::any_cast<bool>(default_value));
}

bool OptionsDB::Option::ValueIsDefault() const
{ return ValueToString() == DefaultValueToString(); }


/////////////////////////////////////////////
// OptionsDB::OptionSection
/////////////////////////////////////////////
OptionsDB::OptionSection::OptionSection() = default;

OptionsDB::OptionSection::OptionSection(const std::string& name_, const std::string& description_,
                                        std::function<bool (const std::string&)> option_predicate_) :
    name(name_),
    description(description_),
    option_predicate(option_predicate_)
{}

/////////////////////////////////////////////
// OptionsDB
/////////////////////////////////////////////
// static(s)
OptionsDB* OptionsDB::s_options_db = nullptr;

OptionsDB::OptionsDB() : m_dirty(false) {
    if (s_options_db)
        throw std::runtime_error("Attempted to create a duplicate instance of singleton class OptionsDB.");

    s_options_db = this;
}

void OptionsDB::Commit() {
    if (!m_dirty)
        return;
    boost::filesystem::ofstream ofs(GetConfigPath());
    if (ofs) {
        XMLDoc doc;
        GetOptionsDB().GetXML(doc);
        doc.WriteDoc(ofs);
        m_dirty = false;
    } else {
        std::cerr << UserString("UNABLE_TO_WRITE_CONFIG_XML") << std::endl;
        std::cerr << PathToString(GetConfigPath()) << std::endl;
        ErrorLogger() << UserString("UNABLE_TO_WRITE_CONFIG_XML");
        ErrorLogger() << PathToString(GetConfigPath());
    }
}

bool OptionsDB::CommitPersistent() {
    bool retval = false;
    auto config_file = GetPersistentConfigPath();
    XMLDoc doc;
    GetOptionsDB().GetXML(doc, true);
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

void OptionsDB::Validate(const std::string& name, const std::string& value) const {
    auto it = m_options.find(name);
    if (!OptionExists(it))
        throw std::runtime_error("Attempted to validate unknown option \"" + name + "\".");

    if (it->second.validator)
        it->second.validator->Validate(value);
    else if (it->second.flag)
        boost::lexical_cast<bool>(value);
}

std::string OptionsDB::GetValueString(const std::string& option_name) const {
    auto it = m_options.find(option_name);
    if (!OptionExists(it))
        throw std::runtime_error(("OptionsDB::GetValueString(): No option called \"" + option_name + "\" could be found.").c_str());
    return it->second.ValueToString();
}

std::string OptionsDB::GetDefaultValueString(const std::string& option_name) const {
    auto it = m_options.find(option_name);
    if (!OptionExists(it))
        throw std::runtime_error(("OptionsDB::GetDefaultValueString(): No option called \"" + option_name + "\" could be found.").c_str());
    return it->second.DefaultValueToString();
}

const std::string& OptionsDB::GetDescription(const std::string& option_name) const {
    auto it = m_options.find(option_name);
    if (!OptionExists(it))
        throw std::runtime_error(("OptionsDB::GetDescription(): No option called \"" + option_name + "\" could be found.").c_str());
    return it->second.description;
}

std::shared_ptr<const ValidatorBase> OptionsDB::GetValidator(const std::string& option_name) const {
    auto it = m_options.find(option_name);
    if (!OptionExists(it))
        throw std::runtime_error(("OptionsDB::GetValidator(): No option called \"" + option_name + "\" could be found.").c_str());
    return it->second.validator;
}

namespace {
    const std::size_t TERMINAL_LINE_WIDTH = 80;

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

        std::vector<std::string> lines { "" };
        for (const auto& token : tokens) {
            if (token == "\n") 
                lines.push_back("");
            else if (widths.second < lines.back().size() + token.size() + indents.second)
                lines.push_back(token + " ");
            else if (!token.empty())
                lines.back().append(token + " ");
        }

        std::string indent { std::string(indents.second, ' ') };
        std::stringstream retval;
        auto first_line = std::move(lines.front());
        retval << std::string(indents.first, ' ') << first_line << std::endl;
        for (auto line : lines)
            if (!line.empty())
                retval << indent << line << std::endl;

        return retval.str();
    }

    bool OptionNameHasParentSection(const std::string& lhs, const std::string& rhs) {
        auto it = lhs.find_last_of('.');
        if (it == std::string::npos)
            return false;
        return lhs.substr(0, it) == rhs;
    }
}

std::unordered_map<std::string, std::set<std::string>> OptionsDB::OptionsBySection(bool allow_unrecognized) const {
    // Determine sections after all predicate calls from known options
    std::unordered_map<std::string, std::unordered_set<std::string>> sections_by_option;
    for (const auto& option : m_options) {
        if (!allow_unrecognized && !option.second.recognized)
            continue;

        for (const auto& section : option.second.sections)
            sections_by_option[option.first].emplace(section);

        for (auto& section : m_sections)
            if (section.second.option_predicate && section.second.option_predicate(option.first))
                sections_by_option[option.first].emplace(section.first);
    }

    // tally the total number of options under each section
    std::unordered_map<std::string, std::size_t> total_options_per_section;
    for (const auto& option_section : sections_by_option) {
        auto option_name = option_section.first;
        auto dot_it = option_name.find_first_of(".");
        // increment count of each containing parent section
        while (dot_it != std::string::npos) {
            total_options_per_section[option_name.substr(0, dot_it)]++;
            dot_it++;
            dot_it = option_name.find_first_of(".", dot_it);
        }
    }

    // sort options into common sections
    std::unordered_map<std::string, std::set<std::string>> options_by_section;
    for (const auto& option : sections_by_option) {
        for (const auto& section : option.second) {
            auto section_name = section;
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

            options_by_section[section_name].emplace(option.first);
        }
    }

    // define which section are top level sections ("root"), move top level candidates with single option to misc
    for (const auto& section_it : total_options_per_section) {
        auto root_name = section_it.first.substr(0, section_it.first.find_first_of("."));
        // root_name with no dot element allowed to pass if an option is known, potentially moving to misc section
        auto total_it = total_options_per_section.find(root_name);
        if (total_it == total_options_per_section.end())
            continue;

        if (total_it->second > 1) {
            options_by_section["root"].emplace(root_name);
        } else if (section_it.first != "misc" &&
                   section_it.first != "root" &&
                   !m_sections.count(section_it.first))
        {
            // move option to misc section
            auto section_option_it = options_by_section.find(section_it.first);
            if (section_option_it == options_by_section.end())
                continue;
            for (auto&& option : section_option_it->second)
                options_by_section["misc"].emplace(std::move(option));
            options_by_section.erase(section_it.first);
        }
    }

    return options_by_section;
}

void OptionsDB::GetUsage(std::ostream& os, const std::string& command_line, bool allow_unrecognized) const {
    // Prevent logger output from garbling console display for low severity messages
    OverrideAllLoggersThresholds(LogLevel::warn);

    auto options_by_section = OptionsBySection(allow_unrecognized);
    if (!command_line.empty() || command_line == "all" || command_line == "raw") {
        // remove the root section if unneeded
        if (options_by_section.count("root"))
            options_by_section.erase("root");
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
    std::set<std::string> section_list {};
    // print option sections
    if (command_line != "all" && command_line != "raw") {
        std::size_t name_col_width = 20;
        if (command_line.empty()) {
            auto root_it = options_by_section.find("root");
            if (root_it != options_by_section.end()) {
                for (const auto& section : root_it->second)
                    if (section.find_first_of(".") == std::string::npos)
                        if (section_list.emplace(section).second && name_col_width < section.size())
                            name_col_width = section.size();
            }
        } else {
            for (const auto& it : options_by_section)
                if (OptionNameHasParentSection(it.first, command_line))
                    if (section_list.emplace(it.first).second && name_col_width < it.first.size())
                        name_col_width = it.first.size();
        }
        name_col_width += 5;

        if (!section_list.empty())
            os << UserString("COMMAND_LINE_SECTIONS") << ":" << std::endl;

        auto indents = std::make_pair(2, name_col_width + 4);
        auto widths = std::make_pair(TERMINAL_LINE_WIDTH - name_col_width, TERMINAL_LINE_WIDTH);
        for (const auto& section : section_list) {
            if (section == "misc") {
                print_misc_section = true;
                continue;
            }
            auto section_it = m_sections.find(section);
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
            os << std::endl;
    }


    // print options
    if (!command_line.empty()) {
        std::set<std::string> option_list;
        if (command_line == "all" || command_line == "raw") {
            for (const auto& option_section_it : options_by_section)
                for (const auto& option : option_section_it.second)
                    option_list.emplace(option);
        } else {
            auto option_section_it = options_by_section.find(command_line);
            if (option_section_it != options_by_section.end())
                option_list = option_section_it->second;
            // allow traversal by node when no other results are found
            if (option_list.empty() && section_list.empty())
                FindOptions(option_list, command_line, allow_unrecognized);
        }

        // insert command_line as option, if it exists
        if (command_line != "all" && command_line != "raw" && m_options.count(command_line))
            option_list.emplace(command_line);

        if (!option_list.empty())
            os << UserString("COMMAND_LINE_OPTIONS") << ":" << std::endl;

        for (const auto& option_name : option_list) {
            auto option_it = m_options.find(option_name);
            if (option_it == m_options.end() || (!allow_unrecognized && !option_it->second.recognized))
                continue;

            if (command_line == "raw") {
                os << option_name << ", " << option_it->second.description << "," << std::endl;
                if (option_it->second.short_name)
                    os << option_it->second.short_name << ", " << option_it->second.description << "," << std::endl;
            } else {
                // option name(s)
                if (option_it->second.short_name)
                    os << "-" << option_it->second.short_name << " | --" << option_name;
                else
                    os << "--" << option_name;

                // option description
                if (!option_it->second.description.empty())
                    os << std::endl << SplitText(UserString(option_it->second.description), {5, 7});
                else
                    os << std::endl;

                // option default value
                if (option_it->second.validator) {
                    auto validator_str = UserString("COMMAND_LINE_DEFAULT") + ": " + option_it->second.DefaultValueToString();
                    os << SplitText(validator_str, {5, 7}, {TERMINAL_LINE_WIDTH - validator_str.size(), 77});
                }
                os << std::endl;
            }
        }

        if (section_list.empty() && option_list.empty()) {
            os << UserString("COMMAND_LINE_NOT_FOUND") << ": " << command_line << std::endl << std::endl;
            os << UserString("COMMAND_LINE_USAGE") << std::endl;
        }
    }

    // reset override in case this function is later repurposed
    OverrideAllLoggersThresholds(boost::none);
}

void OptionsDB::GetXML(XMLDoc& doc, bool non_default_only) const {
    doc = XMLDoc();

    std::vector<XMLElement*> elem_stack;
    elem_stack.push_back(&doc.root_node);

    for (const auto& option : m_options) {
        if (!option.second.storable)
            continue;

        std::string::size_type last_dot = option.first.find_last_of('.');
        std::string section_name = last_dot == std::string::npos ? "" : option.first.substr(0, last_dot);
        std::string name = option.first.substr(last_dot == std::string::npos ? 0 : last_dot + 1);

        if (non_default_only) {
            bool is_default_nonflag = !option.second.flag;
            if (is_default_nonflag)
                is_default_nonflag = IsDefaultValue(m_options.find(option.first));

            // Skip unwanted config options
            // Storing "version.string" in persistent config would render all config options invalid after a new build
            // "version.gl.check.done" is automatically set to true after other logic is performed
            // BUG Some windows may be shown as a child of an other window, but not initially visible.
            //   The OptionDB default of "*.visible" in these cases may be false, but setting the option to false
            //   in a config file may prevent such windows from showing when requested.
            if (option.first == "version.string" || option.first == "version.gl.check.done" || name == "visible" ||
                !option.second.recognized || (is_default_nonflag))
            { continue; }

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

OptionsDB::OptionChangedSignalType& OptionsDB::OptionChangedSignal(const std::string& option) {
    auto it = m_options.find(option);
    if (it == m_options.end())
        throw std::runtime_error("OptionsDB::OptionChangedSignal() : Attempted to get signal for nonexistent option \"" + option + "\".");
    return *it->second.option_changed_sig_ptr;
}

void OptionsDB::Remove(const std::string& name) {
    auto it = m_options.find(name);
    if (it != m_options.end()) {
        Option::short_names.erase(it->second.short_name);
        m_options.erase(it);
        m_dirty = true;
    }
    OptionRemovedSignal(name);
}

void OptionsDB::RemoveUnrecognized(const std::string& prefix) {
    auto it = m_options.begin();
    while (it != m_options.end()) {
        if (!it->second.recognized && it->first.find(prefix) == 0)
            Remove((it++)->first); // note postfix operator++
        else
            ++it;
    }
}

void OptionsDB::FindOptions(std::set<std::string>& ret, const std::string& prefix, bool allow_unrecognized) const {
    ret.clear();
    for (auto& option : m_options)
        if ((option.second.recognized || allow_unrecognized) && option.first.find(prefix) == 0)
            ret.insert(option.first);
}

void OptionsDB::SetFromCommandLine(const std::vector<std::string>& args) {
    //bool option_changed = false;

    for (unsigned int i = 1; i < args.size(); ++i) {
        std::string current_token(args[i]);
        if (current_token.find("--") == 0) {
            std::string option_name = current_token.substr(2);

            auto it = m_options.find(option_name);

            if (it == m_options.end() || !it->second.recognized) { // unrecognized option: may be registered later on so we'll store it for now
                // Check for more parameters (if this is the last one, assume that it is a flag).
                std::string value_str("-");
                if (i + 1 < static_cast<unsigned int>(args.size())) {
                    value_str = args[i + 1]; // copy assignment
                    StripQuotation(value_str);
                }

                if (value_str.at(0) == '-') { // this is either the last parameter or the next parameter is another option, assume this one is a flag
                    m_options[option_name] = Option(static_cast<char>(0), option_name, true,
                                                    boost::lexical_cast<std::string>(false),
                                                    "", 0, false, true, false, std::string());
                } else { // the next parameter is the value, store it as a string to be parsed later
                    m_options[option_name] = Option(static_cast<char>(0), option_name,
                                                    value_str, value_str, "",
                                                    new Validator<std::string>(),
                                                    false, false, false, std::string()); // don't attempt to store options that have only been specified on the command line
                }

                WarnLogger() << "Option \"" << option_name << "\", was specified on the command line but was not recognized.  It may not be registered yet or could be a typo.";
            } else {
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
                        std::string value_str(args[++i]);
                        StripQuotation(value_str);
                        // ensure parameter is actually a parameter, and not the next option name (which would indicate
                        // that the option was specified without a parameter value, as if it was a flag)
                        if (value_str.at(0) == '-')
                            throw std::runtime_error("the option \"" + option.name +
                                                     "\" was followed by the parameter \"" + value_str +
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
            ) {
            std::string single_char_options = current_token.substr(1);

            if (single_char_options.size() == 0) {
                throw std::runtime_error("A \'-\' was given with no options.");
            } else {
                for (unsigned int j = 0; j < single_char_options.size(); ++j) {
                    auto short_name_it = Option::short_names.find(single_char_options[j]);

                    if (short_name_it == Option::short_names.end())
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
}

void OptionsDB::SetFromFile(const boost::filesystem::path& file_path,
                            const std::string& version)
{
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
    } catch (const std::exception&) {
        std::cerr << UserString("UNABLE_TO_READ_CONFIG_XML")  << ": "
                  << file_path << std::endl;
    }
}

void OptionsDB::SetFromXML(const XMLDoc& doc) {
    for (const XMLElement& child : doc.root_node.children)
    { SetFromXMLRecursive(child, ""); }
}

void OptionsDB::SetFromXMLRecursive(const XMLElement& elem, const std::string& section_name) {
    std::string option_name = section_name + (section_name.empty() ? "" : ".") + elem.Tag();

    if (!elem.children.empty()) {
        for (const XMLElement& child : elem.children)
            SetFromXMLRecursive(child, option_name);

    }

    auto it = m_options.find(option_name);

    if (it == m_options.end() || !it->second.recognized) {
        if (elem.Text().length() == 0) {
            // do not retain empty XML options
            return;
        } else {
            // Store unrecognized option to be parsed later if this options is added.
            m_options[option_name] = Option(static_cast<char>(0), option_name,
                                            elem.Text(), elem.Text(),
                                            "", new Validator<std::string>(),
                                            true, false, false, section_name);
        }

        TraceLogger() << "Option \"" << option_name << "\", was in config.xml but was not recognized.  It may not be registered yet or you may need to delete your config.xml if it is out of date.";
        m_dirty = true;
        return;
    }

    Option& option = it->second;
    //if (!option.flag && option.value.empty()) {
    //    ErrorLogger() << "The value member of option \"" << option.name << "\" in config.xml is undefined.";
    //    return;
    //}

    if (option.flag) {
        static auto lexical_true_str = boost::lexical_cast<std::string>(true);
        option.value = static_cast<bool>(elem.Text() == lexical_true_str);
    } else {
        try {
            m_dirty |= option.SetFromString(elem.Text());
        } catch (const std::exception& e) {
            ErrorLogger() << "OptionsDB::SetFromXMLRecursive() : while processing config.xml the following exception was caught when attempting to set option \"" << option_name << "\": " << e.what();
        }
    }
}

void OptionsDB::AddSection(const std::string& name, const std::string& description,
                           std::function<bool (const std::string&)> option_predicate)
{
    auto insert_result = m_sections.emplace(name, OptionSection(name, description, option_predicate));
    // if previously existing section, update description/predicate if empty/null
    if (!insert_result.second) {
        if (!description.empty() && insert_result.first->second.description.empty())
            insert_result.first->second.description = description;
        if (option_predicate != nullptr && insert_result.first->second.option_predicate == nullptr)
            insert_result.first->second.option_predicate = option_predicate;
    }
}

std::string ListToString(const std::vector<std::string>& input_list) {
    // list input strings in comma-separated-value format
    std::string retval;
    for (auto it = input_list.begin(); it != input_list.end(); ++it) {
        if (it != input_list.begin())
            retval += ",";
        std::string str(*it);
        boost::remove_erase_if(str, boost::is_any_of("<&>'\",[]|\a\b\f\n\r\t\b"));  // remove XML protected characters and a few other semi-randomly chosen characters to avoid corrupting enclosing XML document structure
        retval += str;
    }
    return retval;
}

std::vector<std::string> StringToList(const std::string& input_string) {
    std::vector<std::string> retval;
    typedef boost::tokenizer<boost::char_separator<char>> Tokenizer;
    boost::char_separator<char> separator(",");
    Tokenizer tokens(input_string, separator);
    for (const auto& token : tokens)
        retval.push_back(token);
    return retval;
}

