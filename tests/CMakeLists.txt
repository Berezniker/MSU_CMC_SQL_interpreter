add_executable(connection_test connection_test.cpp)
target_link_libraries(connection_test gtest gtest_main Server Exeption Analyze)
add_test(Tests connection_test)

add_executable(analyze_test analyze_test.cpp)
target_link_libraries(analyze_test gtest gtest_main Exeption Analyze)
add_test(Tests analyze_test)