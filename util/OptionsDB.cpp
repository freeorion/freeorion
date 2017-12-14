#include "OptionsDB.h"

#include "i18n.h"
#include "Logger.h"
#include "OptionValidators.h"
#include "XMLDoc.h"

#include "util/Directories.h"

#include <iostream>
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
                          const ValidatorBase* validator_, bool storable_, bool flag_, bool recognized_) :
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
        value = value_;
        (*option_changed_sig_ptr)();
    }
    return changed;
}

bool OptionsDB::Option::SetToDefault() {
    bool changed = false;
    if (!flag) {
        changed = validator->String(value) != validator->String(default_value);
    } else {
        changed = (boost::lexical_cast<std::string>(boost::any_cast<bool>(value))
                   != boost::lexical_cast<std::string>(boost::any_cast<bool>(default_value)));
    }
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


/////////////////////////////////////////////
// OptionsDB
/////////////////////////////////////////////
// static(s)
OptionsDB* OptionsDB::s_options_db = nullptr;

OptionsDB::OptionsDB() {
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

void OptionsDB::GetUsage(std::ostream& os, const std::string& command_line/* = ""*/) const {
    os << UserString("COMMAND_LINE_USAGE") << command_line << "\n";

    int longest_param_name = 0;
    for (auto& option : m_options) {
        if (longest_param_name < static_cast<int>(option.first.size()))
            longest_param_name = option.first.size();
    }

    int description_column = 5;
    int description_width = 80 - description_column;

    if (description_width <= 0)
        throw std::runtime_error("The longest parameter name leaves no room for a description.");

    for (const auto& option : m_options) {
        // Ignore unrecognized options that have not been formally registered
        // with Add().
        if (!option.second.recognized)
            continue;

        if (option.second.short_name)
            os << "-" << option.second.short_name << ", --" << option.second.name << "\n";
        else
            os << "--" << option.second.name << "\n";

        os << std::string(description_column - 1, ' ');

        typedef boost::tokenizer<boost::char_separator<char>> Tokenizer;
        boost::char_separator<char> separator(" \t");
        Tokenizer tokens(UserString(option.second.description), separator);
        int curr_column = description_column;
        for (const auto& token : tokens) {
            if (80 < curr_column + token.size()) {
                os << "\n" << std::string(description_column, ' ') << token;
                curr_column = description_column + token.size();
            } else {
                os << " " << token;
                curr_column += token.size() + 1;
            }
        }

        if (option.second.validator) {
            std::stringstream stream;
            stream << UserString("COMMAND_LINE_DEFAULT") << option.second.DefaultValueToString();
            if (80 < curr_column + stream.str().size() + 3) {
                os << "\n" << std::string(description_column, ' ') << stream.str() << "\n";
            } else {
                os << " | " << stream.str() << "\n";
            }
        } else {
            os << "\n";
        }
        os << "\n";
    }
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
                section_name = "";
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

            std::map<std::string, Option>::iterator it = m_options.find(option_name);

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
                                                    "", 0, false, true, false);
                } else { // the next parameter is the value, store it as a string to be parsed later
                    m_options[option_name] = Option(static_cast<char>(0), option_name,
                                                    value_str, value_str, "",
                                                    new Validator<std::string>(),
                                                    false, false, false); // don't attempt to store options that have only been specified on the command line
                }

                WarnLogger() << "Option \"" << option_name << "\", was specified on the command line but was not recognized.  It may not be registered yet or could be a typo.";
            } else {
                Option& option = it->second;
                if (option.value.empty())
                    throw std::runtime_error("The value member of option \"--" + option.name + "\" is undefined.");

                if (!option.flag) { // non-flag
                    try {
                        // ensure a parameter exists...
                        if (i + 1 >= static_cast<unsigned int>(args.size()))
                            throw std::runtime_error("the option \"" + option.name +
                                                     "\" was specified, at the end of the list, with no parameter value.");
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
                    std::map<char, std::string>::iterator short_name_it = Option::short_names.find(single_char_options[j]);

                    if (short_name_it == Option::short_names.end())
                        throw std::runtime_error(std::string("Unknown option \"-") + single_char_options[j] + "\" was given.");

                    std::map<std::string, Option>::iterator name_it = m_options.find(short_name_it->second);

                    if (name_it == m_options.end())
                        throw std::runtime_error("Option \"--" + short_name_it->second + "\", abbreviated as \"-" + short_name_it->first + "\", could not be found.");

                    Option& option = name_it->second;
                    if (option.value.empty())
                        throw std::runtime_error("The value member of option \"--" + option.name + "\" is undefined.");

                    if (!option.flag) {
                        if (j < single_char_options.size() - 1)
                            throw std::runtime_error(std::string("Option \"-") + single_char_options[j] + "\" was given with no parameter.");
                        else
                            m_dirty |= option.SetFromString(args[++i]);
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
    std::string option_name = section_name + (section_name == "" ? "" : ".") + elem.Tag();

    if (!elem.children.empty()) {
        for (const XMLElement& child : elem.children)
            SetFromXMLRecursive(child, option_name);

    } else {
        auto it = m_options.find(option_name);

        if (it == m_options.end() || !it->second.recognized) {
            // Store unrecognized option to be parsed later if this options is added.
            if (elem.Text().length() == 0) { // empty string: may be a flag
                m_options[option_name] = Option(static_cast<char>(0), option_name, true,
                                                boost::lexical_cast<std::string>(false),
                                                "", 0, true, true, false);
            } else { // otherwise just store the string to be parsed later
                m_options[option_name] = Option(static_cast<char>(0), option_name,
                                                elem.Text(), elem.Text(), "",
                                                new Validator<std::string>(),
                                                true, false, false);
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
            option.value = true;
        } else {
            try {
                m_dirty |= option.SetFromString(elem.Text());
            } catch (const std::exception& e) {
                ErrorLogger() << "OptionsDB::SetFromXMLRecursive() : while processing config.xml the following exception was caught when attempting to set option \"" << option_name << "\": " << e.what();
            }
        }
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

