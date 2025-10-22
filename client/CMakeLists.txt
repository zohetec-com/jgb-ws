add_executable(wscli
    main.cpp
    ws-client.cpp
    wsapp.cpp
    connection_callback.cpp)
target_link_libraries(wscli websockets curl jgb-core jansson)
install(TARGETS wscli)
