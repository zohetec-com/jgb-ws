add_library(ws-app SHARED
    object_callback.cpp
    connection_callback.cpp
    ws-client.cpp
    wsapp.cpp)
add_library(sqlite-app SHARED
    sqlite-app.cpp)
install(TARGETS ws-app sqlite-app)
