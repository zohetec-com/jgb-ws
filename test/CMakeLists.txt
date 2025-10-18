add_library(test-client SHARED
    test_client_app.cpp)
target_link_libraries(test-client)
target_include_directories(test-client PRIVATE ../src)
