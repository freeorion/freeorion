//Parser.cpp

#include "Parser.h"

#include <iostream>

using namespace std;

Parser::Parser()
{
//constructs the parser object with the default space allocated for commands

    numCommands=PARSER_DEFAULTCOMMANDS;
    valid_data=new string[numCommands];
    currSize=0;
}

Parser::Parser(int i)
{
//constructs the parser object with a user-defined number of commands
//  i: the maximum number of commands

    numCommands=i;
    valid_data=new string[numCommands];
    currSize=0;
}

Parser::~Parser()
{
//destroys the object and deallocates memory

    delete(valid_data);
}

string Parser::operator[](string cmd)
{
//returns the value associated with the given command
//  cmd: the command whose value we wish to retrieve

    return data[cmd];
}

void Parser::AddCommand(string cmd, bool needvalue)
{
//adds a command to the list of valid commands
//if the same command is added more than once, the function ignores the request
//  cmd: the command to add

    //make sure we don't add the same key twice
    if(isValidKey(cmd))
        return;
    valid_data[currSize++]=cmd;
    if(needvalue)
    {
        data[cmd]="";
    }
    else
    {
        data[cmd]=PARSER_FALSE;
    }
}

bool Parser::AddValue(string cmd, string val)
{
//adds a value to a given command
//if the command is invalid, this function prints an error message on stdout
// and return false
//otherwise, it adds the value, associates it with the command, and returns true
//  cmd: the command
//  val: the value for that command

    //returns true if successful
    if(!isValidKey(cmd))
    {
        //print an error message for each invalid argument
        cout<<"Invalid switch: \""<<cmd<<"\" containing value: \""<<val<<"\""<<endl;
        return false;
    }
    data[cmd]=val;
    return true;
}

bool Parser::isValidKey(string cmd)
{
//returns true if the command is valid (i.e. found in the list)
//returns false if the command does not exist
//  cmd: the command to check

   for(int i=0;i<currSize;++i)
   {
      if(valid_data[i]==cmd)
            return true;
   } 
   
   return false;
}

bool Parser::Initialize(int argc, char* argv[])
{
//PRECONDITION: AddCommand() must have been called at least once prior
// to calling this function.
//
//This function initializes the parser by assigning command-value pairs
// based on the data passed in from main()
//the function returns true if successful and prints an error message on stdout
// and returns false if it fails.
//  argc: the number of command line tokens
//  argv: array of command line tokens
//NOTE: argc, and argv should be passed UNMODIFIED from main()

    //load the data
    for(int i=1;i<argc;)
    {
        //see if we need to look for a value
        if(data[argv[i]]==PARSER_FALSE || data[argv[i]]==PARSER_TRUE) 
        {
            //we don't need a value, so add it as TRUE
            if(!AddValue(argv[i],PARSER_TRUE))
            {
                cout<<"A parser error occurred."<<endl;
            }
            ++i;//increment i only once to get the next argument
            continue;
        }
        else if(!AddValue(argv[i],argv[i+1]))
        {
            cout<<"A parser error occurred."<<endl;
            return false;
        }
        i+=2;//increment i by 2 since we need to get the next command
    }
    return true;
}
