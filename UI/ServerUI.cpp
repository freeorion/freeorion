//ServerUI.cpp

#include <iostream>
#include <fstream>

#ifndef _ServerUI_h_
#include "ServerUI.h"
#endif

ServerUI::ServerUI():
    LOG_FILENAME("ServerUI.log"),
    out(log4cpp::Category::getRoot())
{
    //clear log file
    std::ofstream file(LOG_FILENAME.c_str());
    file.close();

    //setup output
    log4cpp::FileAppender* out_appender=new log4cpp::FileAppender("out",LOG_FILENAME);
    log4cpp::PatternLayout* outlayout=new log4cpp::PatternLayout();
    
    outlayout->setConversionPattern("%d : %p - %m%n");
    out_appender->setLayout(outlayout);
    
    out.setAdditivity(false);
    out.setAppender(out_appender);
  
    out.setPriority(log4cpp::Priority::NOTSET);
}//ServerUI()

ServerUI::ServerUI(int i):
    LOG_FILENAME("ServerUI.log"),
    parser(i),
    out(log4cpp::Category::getRoot())
{
    //clear log file
    std::ofstream file(LOG_FILENAME.c_str());
    file.close();

    //setup output
    log4cpp::FileAppender* out_appender=new log4cpp::FileAppender("out",LOG_FILENAME);
    log4cpp::PatternLayout* outlayout=new log4cpp::PatternLayout();
    
    outlayout->setConversionPattern("%d : %p - %m%n");
    out_appender->setLayout(outlayout);
    
    out.setAdditivity(false);
    out.setAppender(out_appender);
  
    out.setPriority(log4cpp::Priority::NOTSET);
}//ServerUI(int)

ServerUI::~ServerUI()
{

}//~ServerUI()


