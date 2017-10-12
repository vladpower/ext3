project(FileIdentifier)
cmake_minimum_required(VERSION 2.8)
set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/../)
include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR} "src/")
add_definitions( -std=c++14)

file(GLOB CPPS "src/*.cpp" "src/utils/*.cpp")

add_executable(${PROJECT_NAME} ${CPPS})
target_link_libraries(${PROJECT_NAME} )
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
