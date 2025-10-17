add_executable(sscli connection_callback.cpp helper.cpp sync-service-client.cpp)
target_link_libraries(sscli jgb-core websockets boost_thread boost_chrono boost_filesystem boost_system pcre2-8 crypto ssl)
install(TARGETS sscli)

add_executable(check-setting check-setting.cpp)
target_link_libraries(check-setting jgb-core boost_thread boost_chrono boost_filesystem boost_system pcre2-8)
install(TARGETS check-setting)
