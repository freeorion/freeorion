#include "StringTable.h"

#include "ClientUI.h"
#include "../util/MultiplayerCommon.h"

using namespace std;

namespace {
    void HandleNewlines(std::string& str)
    {
        std::string::size_type pos = 0;
        while ((pos = str.find("\\n", pos)) != std::string::npos) {
            str.replace(pos, 2, "\n");
        }
    }

    bool temp_header_bool = RecordHeaderFile(StringTableRevision());
    bool temp_source_bool = RecordSourceFile("$RCSfile$", "$Revision$");
}

// static(s)
const string StringTable::S_DEFAULT_FILENAME = "eng_stringtable.txt";
const string StringTable::S_ERROR_STRING = "ERROR: ";

// StringTable
StringTable::StringTable():
    m_filename(S_DEFAULT_FILENAME)
{
    Load();
}

StringTable::StringTable(const string& filename):
    m_filename(filename)
{
    Load();
}

StringTable::~StringTable()
{

}

const string& StringTable::operator[] (std::string index)
{
    static std::string retval = ""; // keep this because we are returning a reference
    // since we're using a map now,
    // just do a find and return
    map<string, string>::iterator pos;
    
    pos=m_strings.find(index);
    
    if (pos == m_strings.end())
        return (retval = S_ERROR_STRING + index);  // output the error string along with the index so we can debug
        
    // we got a value, now return the right one
    return pos->second;
    
}

void StringTable::Load()
{
    string temp;
    string temp2;
    ifstream ifs;

    try {
        ifs.open(m_filename.c_str());    //open the file
    } catch (const exception& e) {
        ClientUI::MessageBox("Error opening StringTable file: \"" + m_filename + "\"", true);
        return;        // handle exception by showing error msg and then get out!
    }

    getline(ifs, m_language);

    //we now use 1 line for an identifier and one line per string
    while(ifs.peek() != EOF) {
        getline(ifs, temp);

        //continue reading until we reach a pertinent line
        // i.e. once that doesn't start with a # sign or a newline
        while (temp.empty() || temp[0] == '#') {
            // make sure we're not reading extraneous lines at the end of the file
            if (ifs.peek() == EOF) {
                return;
            }
            getline(ifs, temp);
        }
        getline(ifs, temp2);

        HandleNewlines(temp2);

        if (!m_strings.insert(make_pair(temp, temp2)).second) {
            //found a duplicate or invalid key
            ClientUI::LogMessage("Duplicate string ID found: '" + (string)temp + "' in file: '" + m_filename + "'.  Ignoring duplicate.");
        } else {
            ClientUI::LogMessage("Inserted(" + (string)temp + ", " + string(temp2) + ")");
        }
    }
}

