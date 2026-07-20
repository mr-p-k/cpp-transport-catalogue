cmake_minimum_required(VERSION 3.29)

project(tc LANGUAGES CXX VERSION 0.3.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(${PROJECT_NAME} main.cpp
    domain.h domain.cpp
    geo.h geo.cpp
    json.h json.cpp
    json_reader.h json_reader.cpp
    map_renderer.h map_renderer.cpp
    request_handler.h request_handler.cpp
    svg.h svg.cpp
    transport_catalogue.h transport_catalogue.cpp
)

target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror -pedantic -pedantic-errors>
    $<$<CXX_COMPILER_ID:Clang>:-stdlib=libc++ -v>

    $<$<AND:$<CONFIG:Debug>,$<CXX_COMPILER_ID:GNU>>:-D_GLIBCXX_DEBUG>
    $<$<AND:$<CONFIG:Debug>,$<CXX_COMPILER_ID:Clang>>:-D_LIBCPP_DEBUG=1>
    $<$<AND:$<CONFIG:Debug>,$<CXX_COMPILER_ID:MSVC>>:/D_ITERATOR_DEBUG_LEVEL=2>

    $<$<AND:$<CONFIG:Debug>,$<NOT:$<CXX_COMPILER_ID:MSVC>>,$<NOT:$<PLATFORM_ID:Windows>>>:-fsanitize=undefined>
)

target_link_options(${PROJECT_NAME} PRIVATE
    $<$<CXX_COMPILER_ID:Clang>:-fuse-ld=lld -stdlib=libc++ -lc++abi>

    $<$<AND:$<CONFIG:Debug>,$<NOT:$<CXX_COMPILER_ID:MSVC>>,$<NOT:$<PLATFORM_ID:Windows>>>:-fsanitize=undefined>
)

include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
