add_executable(wscli
    main.cpp)
target_link_libraries(wscli jgb-ws websockets curl ssl crypto jgb-core jansson pcre2-8 boost_thread boost_chrono boost_filesystem boost_system)
install(TARGETS wscli)
