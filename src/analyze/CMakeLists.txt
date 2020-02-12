add_library(Analyze
        analyze.cpp
        analyze.h
#        ../exeption/exception.cpp
        )

include_directories(../exeption ../table)
target_link_libraries(Analyze Exeption Table)