//ServerUI.h
#ifndef _ServerUI_h_
#define _ServerUI_h_

#include <string>

#include "Parser.h"

//includes for log4cpp
#include "log4cpp/Category.hh"
#include "log4cpp/FileAppender.hh"
#include "log4cpp/PatternLayout.hh"

//! ServerUI encapsulates all methods and objects necessary for interface with the server.

class ServerUI
{
private:
//! \name Constants
//!@{

    const std::string LOG_FILENAME;  //!< The ServerUI log file name.
    
//!@}

public:
//! \name Structor
//!@{
    
    //! Default construction.    
    //! Constructs a ServerUI object with the default number of parameters.
    ServerUI();	
    
    //! Construction with a certain number of parameters
    
    //! @param i the max number of parameters to create
    ServerUI(int i);
    
    //! Default destruction
    ~ServerUI();    

//!@}

public:
//! \name Accessible members
//!@{
    
    Parser             parser;    //!< The only parser object
    log4cpp::Category& out;       //!< Output to standard output and log file
    
//!@}

}; //ServerUI

#endif

