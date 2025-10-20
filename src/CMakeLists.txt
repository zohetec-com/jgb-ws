add_library(ws-app SHARED
    connection_callback.cpp
    object_callback.cpp
    wsapp.cpp)
install(TARGETS ws-app)
