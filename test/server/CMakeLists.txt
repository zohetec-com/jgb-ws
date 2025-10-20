add_library(test-server SHARED
    test-server.cpp)
target_link_libraries(test-server)
target_include_directories(test-server PRIVATE ../../src)
