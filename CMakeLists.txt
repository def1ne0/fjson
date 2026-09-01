cmake_minimum_required(VERSION 4.2)
project(fjson VERSION 0.1 LANGUAGES CXX)

add_library(fjson INTERFACE)
add_library(fjson::fjson ALIAS fjson)

target_include_directories(fjson INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_compile_features(fjson INTERFACE cxx_std_26)
target_compile_options(fjson INTERFACE
        $<$<CXX_COMPILER_ID:GNU>:-freflection>
)

include(GNUInstallDirs)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(TARGETS fjson
        EXPORT FjsonTargets
)

install(EXPORT FjsonTargets
        FILE FjsonTargets.cmake
        NAMESPACE fjson::
        DESTINATION ${CMAKE_INSTALL_DATADIR}/fjson
)

include(CTest)
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

