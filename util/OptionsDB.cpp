#include "OptionsDB.h"

#include <iostream>
#include <stdexcept>
#include <string>

namespace {
    std::vector<OptionsDBFn>& OptionsRegistry()
    {
        static std::vector<OptionsDBFn> options_db_registry;
        return options_db_registry;
    }

    std::string PreviousSectionName(const std::vector<GG::XMLElement*>& elem_stack)
    {
        std::string retval;
        for (unsigned int i = 1; i < elem_stack.size(); ++i) {
            retval += elem_stack[i]->Tag();
            if (i != elem_stack.size() - 1)
                retval += '.';
        }
        return retval;
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
{
}

OptionsDB::Option::Option(char short_name_, const std::string& name_, const boost::any& value_, const std::string& default_value_, const std::string& description_, 
                          const ValidatorBase* validator_/* = 0*/) :
	name(name_),
    short_name(short_name_),
	value(value_),
	default_value(default_value_),
	description(description_),
    validator(validator_)
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

void OptionsDB::GetUsage(std::ostream& os, const std::string& command_line/* = ""*/) const
{
    os << "Usage: " << command_line << "\n";

    int longest_param_name = 0;
    for (std::map<std::string, Option>::const_iterator it = m_options.begin(); it != m_options.end(); ++it) {
        if (longest_param_name < static_cast<int>(it->first.size()))
            longest_param_name = it->first.size();
    }

    int description_column = (longest_param_name + 7); // 7 comes from 4 for e.g. "-P, ", 2 for "--", and 1 space afterwards
    int description_width = 80 - description_column;

    if (description_width <= 0)
        throw std::runtime_error("The longest parameter name leaves no room for a description.");

    for (std::map<std::string, Option>::const_iterator it = m_options.begin(); it != m_options.end(); ++it) {
        if (it->second.short_name)
            os << "-" << it->second.short_name << ", --" << it->second.name;
        else
            os << "    --" << it->second.name;

        os << std::string(description_column - (it->second.name.size() + 6), ' ');

        bool first_line = true;
        unsigned int pos = 0;
        unsigned int last_pos = 0;
        const char whitespace[] = " \t\n\r\f";
        while (last_pos < std::string::npos) {
            unsigned int next_pos = it->second.description.find_first_not_of(whitespace, pos);
            if (next_pos != std::string::npos)
                next_pos = it->second.description.find_first_of(whitespace, next_pos);
            if (next_pos != std::string::npos)
                next_pos = it->second.description.find_first_not_of(whitespace, next_pos);

            if (next_pos == std::string::npos) {
                if (!first_line)
                    os << std::string(description_column, ' ');
                next_pos = it->second.description.size();
                if (static_cast<unsigned int>(description_width) < next_pos - last_pos) {
                    os << it->second.description.substr(last_pos, pos - last_pos) << "\n";
                    os << std::string(description_column, ' ') << it->second.description.substr(pos, next_pos) << "\n";
                } else {
                    os << it->second.description.substr(last_pos) << "\n";
                }
                last_pos = std::string::npos;
            } else if (next_pos - last_pos <= static_cast<unsigned int>(description_width)) {
                pos = next_pos;
            } else {
                if (!first_line)
                    os << std::string(description_column, ' ');
                os << it->second.description.substr(last_pos, pos - last_pos) << "\n";
                last_pos = pos;
                first_line = false;
            }
        }
        if (it->second.validator)
            os << std::string(description_column, ' ') << "default: " << it->second.default_value << "\n";
    }
}

GG::XMLDoc OptionsDB::GetXML() const
{
    GG::XMLDoc doc;

    std::vector<GG::XMLElement*> elem_stack;
    elem_stack.push_back(&doc.root_node);

    for (std::map<std::string, Option>::const_iterator it = m_options.begin(); it != m_options.end(); ++it) {
        unsigned int last_dot = it->first.find_last_of('.');
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
            unsigned int last_pos = 0;
            unsigned int pos = 0;
            while ((pos = section_name.find('.', last_pos)) != std::string::npos) {
                GG::XMLElement temp(section_name.substr(last_pos, pos - last_pos));
                elem_stack.back()->AppendChild(temp);
                elem_stack.push_back(&elem_stack.back()->Child(temp.Tag()));
                last_pos = pos + 1;
            }
            GG::XMLElement temp(section_name.substr(last_pos));
            elem_stack.back()->AppendChild(temp);
            elem_stack.push_back(&elem_stack.back()->Child(temp.Tag()));
        }

        GG::XMLElement temp(name);
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

void OptionsDB::Remove(const std::string& name)
{
    std::map<std::string, Option>::iterator it = m_options.find(name);
    if (it != m_options.end()) {
        Option::short_names.erase(it->second.short_name);
        m_options.erase(it);
    }
    m_option_removed_sig(name);
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

            if (option.validator) // non-flag
                option.FromString(argv[++i]);
            else // flag
                option.value = true;

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

    if (option_changed)
        m_options_changed_sig();
}

void OptionsDB::SetFromXML(const GG::XMLDoc& doc)
{
    for (int i = 0; i < doc.root_node.NumChildren(); ++i) {
        SetFromXMLRecursive(doc.root_node.Child(i), "");
    }

    m_options_changed_sig();
}

void OptionsDB::SetFromXMLRecursive(const GG::XMLElement& elem, const std::string& section_name)
{
    std::string option_name = section_name + (section_name == "" ? "" : ".") + elem.Tag();

    // flags have no text or children; their presence at all indicates a value of true
    std::string option_value = elem.NumChildren() || elem.Text() != "" ? elem.Text() : "true";

    if (option_value != "") {
        std::map<std::string, Option>::iterator it = m_options.find(option_name);

        if (it == m_options.end())
            throw std::runtime_error("Option \"" + option_name + "\", could not be found.");

        Option& option = it->second;
        if (option.value.empty())
            throw std::runtime_error("The value member of option \"" + option.name + "\" is undefined.");

        option.FromString(option_value);
    }

    for (int i = 0; i < elem.NumChildren(); ++i) {
        SetFromXMLRecursive(elem.Child(i), option_name);
    }
}
