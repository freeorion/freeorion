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

const string& StringTable::operator[] (int index)
{
    try
    { 
        const string* retval = &m_strings.at(index);
        return *retval;
    }
    catch (const exception& error)
    {
        return S_ERROR_STRING;
    }
}//operator() ()

void StringTable::Load()
{
    char temp[256] = {0};
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
    
    file.getline(temp,256);    //read the first line
    m_language = temp;
    
    //read first 4 characters of each line for the index and discard
    //we don't use the index in the file, its just to make it human-readable
    while(file.peek() != EOF)
    {
        file.get(temp,6);    //read 4 characters
  // NON-PORTABLE?     file.eatwhite();     //eliminate leading whitespace
        file.getline(temp,256);   //read the string
        
        m_strings.push_back(string(temp));  //add to the table
    }//while
    
    
}//Load()

