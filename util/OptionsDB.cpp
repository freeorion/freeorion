#include "OptionsDB.h"

#include "MultiplayerCommon.h"
#include "OptionValidators.h"

#include <iostream>
#include <stdexcept>
#include <string>

#include <boost/spirit.hpp>
#include <boost/algorithm/string/erase.hpp>
#include <boost/algorithm/string/predicate.hpp>

namespace {
    std::vector<OptionsDBFn>& OptionsRegistry()
    {
        static std::vector<OptionsDBFn> options_db_registry;
        return options_db_registry;
    }

    std::string PreviousSectionName(const std::vector<XMLElement*>& elem_stack)
    {
        std::string retval;
        for (unsigned int i = 1; i < elem_stack.size(); ++i) {
            retval += elem_stack[i]->Tag();
            if (i != elem_stack.size() - 1)
                retval += '.';
        }
        return retval;
    }

    struct PushBack
    {
        PushBack(std::vector<std::string>& string_vec) : m_string_vec(string_vec) {}
        void operator()(const char* first, const char* last) const {m_string_vec.push_back(std::string(first, last));}
        std::vector<std::string>& m_string_vec;
    };

    void StripQuotation(std::string& str)
    {
        using namespace boost::algorithm;
        if (starts_with(str, "\"") && ends_with(str, "\""))
        {
            erase_first(str, "\"");
            erase_last(str, "\"");
        }
    }
}

/////////////////////////////////////////////
// Free Functions
/////////////////////////////////////////////
bool RegisterOptions(OptionsDBFn function)
{
    OptionsRegistry().push_back(function);
    return true;
}

OptionsDB& GetOptionsDB()
{
    static OptionsDB options_db;
    if (unsigned int registry_size = OptionsRegistry().size()) {
        for (unsigned int i = 0; i < registry_size; ++i)
            OptionsRegistry()[i](options_db);
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

OptionsDB::Option::Option(char short_name_, const std::string& name_, const boost::any& value_, const std::string& default_value_, const std::string& description_, 
                          const ValidatorBase* validator_, bool storable_) :
    name(name_),
    short_name(short_name_),
    value(value_),
    default_value(default_value_),
    description(description_),
    validator(validator_),
    storable(storable_),
    option_changed_sig_ptr(new boost::signal<void ()>())
{
    if (short_name_)
        short_names[short_name_] = name;
}

void OptionsDB::Option::FromString(const std::string& str)
{
    if (validator) { // non-flag
        value = validator->Validate(str);
    } else { // flag
        value = boost::lexical_cast<bool>(str);
    }
}

std::string OptionsDB::Option::ToString() const
{
    if (validator) { // non-flag
        return validator->String(value);
    } else { // flag
        return boost::lexical_cast<std::string>(boost::any_cast<bool>(value));
    }
}


/////////////////////////////////////////////
// OptionsDB
/////////////////////////////////////////////
// static(s)
OptionsDB* OptionsDB::s_options_db = 0;

OptionsDB::OptionsDB()
{
    if (s_options_db)
        throw std::runtime_error("Attempted to create a duplicate instance of singleton class OptionsDB.");

    s_options_db = this;
}

void OptionsDB::Validate(const std::string& name, const std::string& value) const
{
    std::map<std::string, Option>::const_iterator it = m_options.find(name);
    if (it == m_options.end())
        throw std::runtime_error("Attempted to validate unknown option \"" + name + "\".");

    if (it->second.validator) { // non-flag
        it->second.validator->Validate(value);
    } else { // flag
        boost::lexical_cast<bool>(value);
    }
}

const std::string& OptionsDB::GetDescription(const std::string& option_name) const
{
    std::map<std::string, Option>::const_iterator it = m_options.find(option_name);
    if (it == m_options.end())
        throw std::runtime_error(("OptionsDB::GetDescription(): No option called \"" + option_name + "\" could be found.").c_str());
    return it->second.description;
}
const std::string& OptionsDB::GetDefaultValue(const std::string& option_name) const
{
    std::map<std::string, Option>::const_iterator it = m_options.find(option_name);
    if (it == m_options.end())
        throw std::runtime_error(("OptionsDB::GetDefaultValue(): No option called \"" + option_name + "\" could be found.").c_str());
    return it->second.default_value;
}

boost::shared_ptr<const ValidatorBase> OptionsDB::GetValidator(const std::string& option_name) const
{
    std::map<std::string, Option>::const_iterator it = m_options.find(option_name);
    if (it == m_options.end())
        throw std::runtime_error(("OptionsDB::GetValidator(): No option called \"" + option_name + "\" could be found.").c_str());
    return it->second.validator;
}

void OptionsDB::GetUsage(std::ostream& os, const std::string& command_line/* = ""*/) const
{
    os << UserString("COMMAND_LINE_USAGE") << command_line << "\n";

    int longest_param_name = 0;
    for (std::map<std::string, Option>::const_iterator it = m_options.begin(); it != m_options.end(); ++it) {
        if (longest_param_name < static_cast<int>(it->first.size()))
            longest_param_name = it->first.size();
    }

    int description_column = 5;
    int description_width = 80 - description_column;

    if (description_width <= 0)
        throw std::runtime_error("The longest parameter name leaves no room for a description.");

    for (std::map<std::string, Option>::const_iterator it = m_options.begin(); it != m_options.end(); ++it) {
        if (it->second.short_name)
            os << "-" << it->second.short_name << ", --" << it->second.name << "\n";
        else
            os << "--" << it->second.name << "\n";

        os << std::string(description_column - 1, ' ');

        std::vector<std::string> tokenized_strings;
        using boost::spirit::anychar_p;
        using boost::spirit::rule;
        using boost::spirit::space_p;
        rule<> tokenizer = +(*space_p >> (+(anychar_p - space_p))[PushBack(tokenized_strings)]);
        parse(UserString(it->second.description).c_str(), tokenizer);
        int curr_column = description_column;
        for (unsigned int i = 0; i < tokenized_strings.size(); ++i) {
            if (80 < curr_column + tokenized_strings[i].size() + (i ? 1 : 0)) {
                os << "\n" << std::string(description_column, ' ') << tokenized_strings[i];
                curr_column = description_column + tokenized_strings[i].size();
            } else {
                os << " " << tokenized_strings[i];
                curr_column += tokenized_strings[i].size() + 1;
            }
        }
        if (it->second.validator) {
            std::stringstream stream;
            stream << UserString("COMMAND_LINE_DEFAULT") << it->second.default_value;
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

XMLDoc OptionsDB::GetXML() const
{
    XMLDoc doc;

    std::vector<XMLElement*> elem_stack;
    elem_stack.push_back(&doc.root_node);

    for (std::map<std::string, Option>::const_iterator it = m_options.begin(); it != m_options.end(); ++it) {
        if (!it->second.storable)
            continue;
        std::string::size_type last_dot = it->first.find_last_of('.');
        std::string section_name = last_dot == std::string::npos ? "" : it->first.substr(0, last_dot);
        std::string name = it->first.substr(last_dot == std::string::npos ? 0 : last_dot + 1);
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
                elem_stack.back()->AppendChild(temp);
                elem_stack.push_back(&elem_stack.back()->Child(temp.Tag()));
                last_pos = pos + 1;
            }
            XMLElement temp(section_name.substr(last_pos));
            elem_stack.back()->AppendChild(temp);
            elem_stack.push_back(&elem_stack.back()->Child(temp.Tag()));
        }

        XMLElement temp(name);
        if (it->second.validator) { // non-flag
            temp.SetText(it->second.ToString());
        } else { // flag
            if (!boost::any_cast<bool>(it->second.value))
                continue;
        }
        elem_stack.back()->AppendChild(temp);
        elem_stack.push_back(&elem_stack.back()->Child(temp.Tag()));
    }

    return doc;
}

OptionsDB::OptionChangedSignalType& OptionsDB::OptionChangedSignal(const std::string& option)
{
    std::map<std::string, Option>::const_iterator it = m_options.find(option);
    if (it == m_options.end())
        throw std::runtime_error("OptionsDB::OptionChangedSignal() : Attempted to get signal for nonexistent option \"" + option + "\".");
    return *it->second.option_changed_sig_ptr;
}

void OptionsDB::Remove(const std::string& name)
{
    std::map<std::string, Option>::iterator it = m_options.find(name);
    if (it != m_options.end()) {
        Option::short_names.erase(it->second.short_name);
        m_options.erase(it);
    }
    OptionRemovedSignal(name);
}

void OptionsDB::SetFromCommandLine(int argc, char* argv[])
{
    bool option_changed = false;

    for (int i = 1; i < argc; ++i) {
        std::string current_token(argv[i]);
        if (current_token.find("--") == 0) {
            std::string option_name = current_token.substr(2);
            
            std::map<std::string, Option>::iterator it = m_options.find(option_name);
            
            if (it == m_options.end())
                throw std::runtime_error("Option \"" + current_token + "\", could not be found.");
            
            Option& option = it->second;
            if (option.value.empty())
                throw std::runtime_error("The value member of option \"--" + option.name + "\" is undefined.");

            if (option.validator) { // non-flag
                try {
                    // ensure a parameter exists...
                    if (i + 1 >= argc)
                        throw std::runtime_error("the option \"" + option.name + 
                                                 "\" was specified, at the end of the list, with no parameter value.");
                    // get parameter value
                    std::string value_str(argv[++i]);
                    StripQuotation(value_str);
                    // ensure parameter is actually a parameter, and not the next option name (which would indicate
                    // that the option was specified without a parameter value, as if it was a flag)
                    if (value_str.find("-") != std::string::npos)
                        throw std::runtime_error("the option \"" + option.name + 
                                                 "\" was followed by the parameter \"" + value_str + 
                                                 "\", which appears to be an option flag, not a parameter value, because it contains a \"-\" character.");
                    option.FromString(value_str);
                } catch (const std::exception& e) {
                    throw std::runtime_error("OptionsDB::SetFromCommandLine() : the following exception was caught when attemptimg to set option \"" + option.name + "\": " + e.what() + "\n\n");
                }
            } else { // flag
                option.value = true;
            }

            option_changed = true;
        } else if (current_token.find('-') == 0) {
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

                    if (option.validator) { // non-flag
                        if (j < single_char_options.size() - 1)
                            throw std::runtime_error(std::string("Option \"-") + single_char_options[j] + "\" was given with no parameter.");
                        else
                            option.FromString(argv[++i]);
                    } else { // flag
                        option.value = true;
                    }

                    option_changed = true;
                }
            }
        }
    }
}

void OptionsDB::SetFromXML(const XMLDoc& doc)
{
    for (int i = 0; i < doc.root_node.NumChildren(); ++i) {
        SetFromXMLRecursive(doc.root_node.Child(i), "");
    }
}

void OptionsDB::SetFromXMLRecursive(const XMLElement& elem, const std::string& section_name)
{
    std::string option_name = section_name + (section_name == "" ? "" : ".") + elem.Tag();

    // flags have no text or children; their presence at all indicates a value of true
    std::string option_value = elem.NumChildren() || elem.Text() != "" ? elem.Text() : "1";

    if (option_value != "") {
        std::map<std::string, Option>::iterator it = m_options.find(option_name);

        if (it == m_options.end())
            throw std::runtime_error("Option \"" + option_name + "\", could not be found.");

        Option& option = it->second;
        if (option.value.empty())
            throw std::runtime_error("The value member of option \"" + option.name + "\" is undefined.");

        try {
            option.FromString(option_value);
        } catch (const std::exception& e) {
            throw std::runtime_error("OptionsDB::SetFromXMLRecursive() : the following exception was caught when attemptimg to set option \"" + option_name + "\": " + e.what());
        }
    }

    for (int i = 0; i < elem.NumChildren(); ++i) {
        SetFromXMLRecursive(elem.Child(i), option_name);
    }
}
