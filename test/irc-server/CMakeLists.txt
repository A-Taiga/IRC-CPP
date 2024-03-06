cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 20)

set(SERVER_ADDRESS_V4 127.0.0.1)
set(SERVER_ADDRESS_V6 127.0.0.1)
set(SERVER_PORT 6667)

macro(add_server_test TESTNAME)
    add_executable(${TESTNAME} ${TESTNAME}.cpp)
    target_include_directories(${TESTNAME} PRIVATE ../../source/irc-server/include)
    target_include_directories(${TESTNAME} PUBLIC ../../include)
    add_test(NAME ${TESTNAME} COMMAND ${TESTNAME} ${SERVER_ADDRESS_V4} ${SERVER_PORT})
endmacro(add_static_library)

add_server_test(irc_server_connect_v4)
add_server_test(irc_server_close_v4)
add_server_test(irc_server_shutdown_v4)
add_server_test(irc_server_send_v4)

add_server_test(irc_server_connect_v6)
add_server_test(irc_server_close_v6)
add_server_test(irc_server_shutdown_v6)
add_server_test(irc_server_send_v6)
