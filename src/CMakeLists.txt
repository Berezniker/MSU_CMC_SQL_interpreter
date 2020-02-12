# билдим библиотеки

add_library(Server
        server/server.cpp
        server/sever_interface.cpp
        )
add_library(Exeption
        exeption/exception.cpp
        )
add_library(Table
        table/Where_condition.cpp
        table/table.cpp
        )
add_library(Client
        client/client.cpp
        )

add_library(Analyze
        analyze/analyze.cpp
        )
target_link_libraries(Analyze Table Exeption)