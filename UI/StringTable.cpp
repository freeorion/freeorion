//StringTable.cpp

#ifndef _StringTable_h_
#include "StringTable.h"
#endif

#ifndef _ClientUI_h_
#include "ClientUI.h"
#endif

using namespace std;

//init constants
const string StringTable::S_DEFAULT_FILENAME = "eng_stringtable.txt";
const string StringTable::S_ERROR_STRING = "STRINGTABLE_INVALID_INDEX";

StringTable::StringTable():
    m_filename(S_DEFAULT_FILENAME)
{
    Load();
}//StringTable()

StringTable::StringTable(const string& filename):
    m_filename(filename)
{
    Load();
}//StringTable(string)

StringTable::~StringTable()
{

}//~StringTable()

const string& StringTable::operator[] (std::string index)
{
    //since we're using a map now,
    // just do a find and return
    map<string, string>::iterator pos;
    
    pos=m_strings.find(index);
    
    if(pos == m_strings.end())
        return S_ERROR_STRING;
        
    //we got a value, now return the right one
    return pos->second;
    
}//operator() ()

void StringTable::Load()
{
    char temp[256]  = {0};
    char temp2[256] = {0};
    ifstream file;
    
    try
    {
        file.open(m_filename.c_str());    //open the file
    }
    catch(const exception& e)
    {
        ClientUI::MessageBox("Error opening StringTable file: \"" + m_filename + "\"");
        return;        //handle exception by showing error msg and then get out!
    }
    
    file.getline(temp,256);    //read the first line, which should be the language
    m_language = temp;
    
    //we now use 1 line for an identifier and one line per string
    while(file.peek() != EOF)
    {

        file.getline(temp,255);    //read identifier
        
        //continue reading until we reach a pertinent line
        // i.e. once that doesn't start with a # sign or a newline
        while( (strcmp(temp,"")==0) || (*temp == '#') )
        {
            //make sure we're not reading extraneous lines at the end of the file
            if(file.peek() == EOF)
            {
                file.close();
                return;
            }
            file.getline(temp,255);        //read another line
        }
        file.getline(temp2,255);   //read the string
        
       if(!m_strings.insert(make_pair((string)temp,(string)temp2)).second)
       {
           //found a duplicate or invalid key
           ClientUI::LogMessage("Duplicate string ID found: '" + (string)temp + "' in file: '" + m_filename + "'.  Ignoring duplicate.");
       }
       else
       {
           ClientUI::LogMessage("Inserted(" + (string)temp + ", " + string(temp2) + ")");
       }       
      
    }//while
    
    file.close();
    
}//Load()

