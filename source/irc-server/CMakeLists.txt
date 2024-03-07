cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 20)

project(irc-server)
add_executable(irc-server
    main.cpp
    server.cpp
    xlog.cpp
    kqueue.cpp
    )
target_include_directories(irc-server PRIVATE include)
target_include_directories(irc-server PUBLIC ../../include)
