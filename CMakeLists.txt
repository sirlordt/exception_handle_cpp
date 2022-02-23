cmake_minimum_required(VERSION 3.14)

project(main)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#Clang++ require the next line to debug std::string
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_GLIBCXX_DEBUG")

file(GLOB SOURCES "src/*.cpp")

add_executable(main ${SOURCES})
#  Vector.hpp
#  Exception.hpp
#  CallStackFrame.hpp
#  CommonConstants.hpp
#  CommonUtilities.hpp
#  SystemConstants.hpp
#  SystemUtilities.hpp
#   src/main.cpp
#)

target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/src)

target_link_libraries(main PRIVATE rt dl m pthread)
