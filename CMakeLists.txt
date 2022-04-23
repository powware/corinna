cmake_minimum_required(VERSION 3.16.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(corinna LANGUAGES CXX)


add_library(corinna INTERFACE)

target_include_directories(corinna INTERFACE ${PROJECT_SOURCE_DIR}/include)


add_executable(corinna-examples ${PROJECT_SOURCE_DIR}/examples/corinna.cpp)

target_link_libraries(corinna-examples PRIVATE corinna)


include(FetchContent)

FetchContent_Declare(
    cppcoro
    GIT_REPOSITORY https://github.com/andreasbuhr/cppcoro.git
    GIT_TAG 7cc9433436fe8f2482138019cfaafce8e1d7a896
)

FetchContent_MakeAvailable(cppcoro)

add_executable(cppcoro-examples ${PROJECT_SOURCE_DIR}/examples/cppcoro.cpp)

target_link_libraries(cppcoro-examples PRIVATE cppcoro)