add_executable(wscli
    main.cpp)
target_link_libraries(wscli websockets curl ws-app jgb-core jansson)
install(TARGETS wscli)
