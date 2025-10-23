add_executable(wscli
    main.cpp)
target_link_libraries(wscli websockets curl jgb-ws jgb-core jansson)
install(TARGETS wscli)
