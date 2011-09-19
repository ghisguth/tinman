include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_executable(tinman main.cpp tinman.cpp)
target_link_libraries(tinman m fmodex64 ${Boost_LIBRARIES})
set_target_properties(tinman PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_RPATH};/usr/local/lib/" BUILD_WITH_INSTALL_RPATH "1")