add_library(test-client SHARED
    test-client.cpp)
target_link_libraries(test-client)
target_include_directories(test-client PRIVATE ../../src)
