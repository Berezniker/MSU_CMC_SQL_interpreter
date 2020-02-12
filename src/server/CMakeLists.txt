add_library(Server
        server.cpp
        sever_interface.cpp)

include_directories(../analyze)
target_link_libraries(Server Analyze)