# specify the minimum version for CMake
cmake_minimum_required(VERSION 3.10)

# set the project name
project(tcp-sink)

# set the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# export commands database
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# add subdirectory
add_subdirectory(external/libflyzero)

# add the executable
add_executable(tcp-sink src/main.cpp src/tcp_sink_connection.cpp src/tcp_sink_server.cpp)

# add the target library
target_link_libraries(tcp-sink PRIVATE flyzero)

# add debian package building
add_custom_target(debian
    COMMAND mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/debian/usr/bin
    COMMAND cp ${CMAKE_CURRENT_BINARY_DIR}/tcp-sink ${CMAKE_CURRENT_BINARY_DIR}/debian/usr/bin
    COMMAND dpkg-deb --build debian tcp-sink_v1.0.0_${CMAKE_SYSTEM_PROCESSOR}.deb
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS tcp-sink
    COMMENT "Building Debian package")