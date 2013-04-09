cmake_minimum_required(VERSION 2.6)
cmake_policy(VERSION 2.6.4)

set(BUILD_STATIC ON)
set(BUILD_SHARED OFF)

set(THIS_LIB_SOURCES
    src/Appender.cpp
    src/AppenderSkeleton.cpp
    src/BasicLayout.cpp
    src/Category.cpp
    src/CategoryStream.cpp
    src/Configurator.cpp
    src/FactoryParams.cpp
    src/FileAppender.cpp
    src/HierarchyMaintainer.cpp
    src/LayoutAppender.cpp
    src/Localtime.cpp
    src/LoggingEvent.cpp
    src/NDC.cpp
    src/PatternLayout.cpp
    src/Priority.cpp
    src/StringUtil.cpp
    src/TimeStamp.cpp
)

if (WIN32)
    list(APPEND THIS_LIB_SOURCES src/MSThreads.cpp)
else ()
    list(APPEND THIS_LIB_SOURCES src/PThreads.cpp)
endif ()

library_all_variants(log4cpp)

