add_executable(wscli
    main.cpp
    ws-client.cpp
    wsapp.cpp
    connection_callback.cpp)
target_link_libraries(wscli jgb-core websockets curl boost_thread boost_chrono boost_filesystem boost_system pcre2-8 crypto ssl)
install(TARGETS wscli)
