cmake_minimum_required(VERSION 2.6)
cmake_policy(VERSION 2.6.4)

set(log4cpp_SOURCES
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
    list(APPEND log4cpp_SOURCES src/MSThreads.cpp)
else ()
    list(APPEND log4cpp_SOURCES src/PThreads.cpp)
endif ()

add_library(log4cpp STATIC ${log4cpp_SOURCES})

if (CMAKE_COMPILER_IS_GNUCXX OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set_target_properties(log4cpp
        PROPERTIES
        COMPILE_FLAGS "-fPIC"
    )
endif ()

