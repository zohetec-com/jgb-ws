add_library(sync-client SHARED
    connection_callback.cpp
    sync_client_app.cpp
    wsapp.cpp
    helper.cpp)
target_link_libraries(sync-client rt)
install(TARGETS sync-client)
