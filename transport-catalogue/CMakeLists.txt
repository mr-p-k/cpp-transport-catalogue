cmake_minimum_required(VERSION 3.29)

project(transport_catalogue LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(transport_catalogue main.cpp
    geo.h
    input_reader.h input_reader.cpp
    stat_reader.h stat_reader.cpp
    transport_catalogue.h transport_catalogue.cpp
)

# add for debug config additional flags to compiler and linker
# also for clang compiler config uses LLVM libc++ instead GNU libstdc++
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