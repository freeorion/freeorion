//Parser.h

#ifndef _Parser_h_
#define _Parser_h_

#include <map>
#include <string>

//this is the default number of commands available
//  it is used to initialize the array

#define PARSER_DEFAULTCOMMANDS 64   //!< the number of commands Parser uses if it is not initialized with one

#define PARSER_FALSE "F"            //!< value if a value-less command exists on the command line
#define PARSER_TRUE  "T"            //!< value if a value-less command does not exist on the command line


//! \file This is the header file for a SIMPLE command line parser\n

//! parsers will consist of a command and parameter in the form:\n
//!  -command parameter\n
//!  where command is a string (case sensitive)\n
//!      and parameter is data for that parameter\n
//!
//! USAGE:\n
//! 1. Create Parser object\n
//! 2. AddCommand() for each command you want to be valid\n
//!  3. Initialize() the parser with the argc and argv\n
//!  4. Use the [] operator to access those values as std::string's\n

class Parser
{
public:
   //! \name structors  
   //@{
   
   //! Constructs a Parser object with the default number of available commands
    Parser();                                 //constructor
    
   //! Constructs a Parser object with a given number of available commands
   
   //! @param i the number of commands you wish to have available
   //!
    Parser(int i);                            //constructor: i=number of commands
    
    //! Destroys the object
    ~Parser();                                //destructor
    //!@}
    
   //! \name initialization  
   //@{
   
   //! Adds a command to the list of valid commands
   
   //! @param cmd the command to be added
   //! @param needvalue true if parser should expect to see a value for this command, defaults to true
    void AddCommand(std::string cmd, bool needvalue=true);  //adds a command to the valid list
    
    //! Initializes the Parser object
    
    //! @param argc number of arguments on the command line, should be passed un-modified from main()
    //! @param argv array of strings representing command line tokens, also should be passed from main()
    //! @return true if successful, otherwise it returns false and outputs an error message
    //!
    //! Initializes the parser by parsing the given command line parameters.
    //! All commands and values are stored internally.
    bool Initialize(int argc, char* argv[]);  //init with the arguments
    //!@}
    
   //! \name accessor 
   //@{
   
   //! Accesses the value associated with the given command
   
   //! @param cmd the command to access
   //! @return the value of the given command.  If the command doesn't exist, return value is UNDEFINED.
   //! If the command is not associated with a value, the return value will be either PARSER_TRUE, if
   //! the command exists in the command-line, or PARSER_FALSE if it doesn't.
   //! 
   //! This operator is the main accessor of the PArser object.  It will
   //! retrieve the value of the passed command.  Note that this function's behavior is undefined
   //! if the command has not been added to the parser as a valid command with AddValue().
    std::string operator[] (std::string cmd); //gets the data out of the map
    //!@}
private:
   //internally used functions
    bool isValidKey(std::string cmd);                //determines if the key is valid
    bool AddValue(std::string cmd, std::string val); //adds the value for the defined data
   
   //internal members
    std::string *valid_data;                //list of valid keys
    int currSize;                           //current size of array
    int numCommands;
    std::map<std::string,std::string> data; //the data in the map  
};//Parser

#endif


