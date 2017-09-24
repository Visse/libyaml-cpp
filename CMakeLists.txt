cmake_minimum_required(VERSION 3.6)
project(libyaml-cpp)



option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option (BUILD_TESTS "Build tests" ON)

find_package(libyaml)

include_directories(include)
add_library(libyaml-cpp
    include/libyaml-cpp.h
    src/libyaml-cpp.cpp
)
target_link_libraries(libyaml-cpp
    PRIVATE libyaml
)

if(BUILD_SHARED_LIBS)
    target_compile_definitions(libyaml-cpp PRIVATE LIBYAML_CPP_EXPORT)
else(BUILD_SHARED_LIBS)
    target_compile_definitions(libyaml-cpp PUBLIC LIBYAML_CPP_STATIC)
endif(BUILD_SHARED_LIBS)

if (BUILD_TESTS)
    enable_testing()
    
    add_executable (tests tests/tests.cpp)
    target_link_libraries(tests libyaml-cpp)
    add_test (NAME tests COMMAND tests)
endif (BUILD_TESTS)


install(TARGETS libyaml-cpp
    EXPORT libyaml-cpp
    RUNTIME DESTINATION bin/$<CONFIG>
    ARCHIVE DESTINATION lib/$<CONFIG>
    INCLUDES DESTONATION include
)
install(FILES include/libyaml-cpp.h DESTINATION include/)
install(EXPORT libyaml-cpp DESTINATION cmake FILE  libyaml-cpp-config.cmake)
