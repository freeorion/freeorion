#include "Logger.h"

#include <fstream>

#include <log4cpp/Appender.hh>
#include <log4cpp/Category.hh>
#include <log4cpp/PatternLayout.hh>
#include <log4cpp/FileAppender.hh>

int g_indent = 0;

std::string DumpIndent()
{ return std::string(g_indent * 4, ' '); }

void InitLogger(const std::string& logFile, const std::string& pattern)
{
    // a platform-independent way to erase the old log
    std::ofstream temp(logFile.c_str());
    temp.close();

    // establish debug logging
    log4cpp::Appender* appender = new log4cpp::FileAppender("FileAppender", logFile);
    log4cpp::PatternLayout* layout = new log4cpp::PatternLayout();
    layout->setConversionPattern(pattern);
    appender->setLayout(layout);
    Logger().setAdditivity(false);  // make appender the only appender used...
    Logger().setAppender(appender);
    Logger().setAdditivity(true);   // ...but allow the addition of others later
}

log4cpp::Category& Logger()
{ return log4cpp::Category::getRoot(); }

int PriorityValue(const std::string& name)
{
    static std::map<std::string, int> priority_map;
    static bool init = false;
    if (!init) {
        using namespace log4cpp;
        priority_map["FATAL"] = Priority::FATAL;
        priority_map["EMERG"] = Priority::EMERG;
        priority_map["ALERT"] = Priority::ALERT;
        priority_map["CRIT"] = Priority::CRIT;
        priority_map["ERROR"] = Priority::ERROR;
        priority_map["WARN"] = Priority::WARN;
        priority_map["NOTICE"] = Priority::NOTICE;
        priority_map["INFO"] = Priority::INFO;
        priority_map["DEBUG"] = Priority::DEBUG;
        priority_map["NOTSET"] = Priority::NOTSET;
    }
    return priority_map[name];
}

