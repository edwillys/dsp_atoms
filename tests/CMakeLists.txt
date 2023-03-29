cmake_minimum_required(VERSION 3.10)

# set the project name
project(test)

# set standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_CXX_EXTENSIONS OFF)

# set config flags
#set(CMAKE_CXX_FLAGS "-Wall -Wextra")
#set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
#set(CMAKE_CXX_FLAGS_RELEASE "-O3")

# search for unit_test_framework
find_package(GTest CONFIG REQUIRED)
find_package(SndFile CONFIG REQUIRED)

include_directories(
    ${Boost_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/../src
)

# create a testapp target from the source files
add_executable(
    testapp 
    ${CMAKE_SOURCE_DIR}/main.cpp 
    ${CMAKE_SOURCE_DIR}/ResamplerTests.cpp
    ${CMAKE_SOURCE_DIR}/UpFirDownTests.cpp
    ${CMAKE_SOURCE_DIR}/AtomBiquadTests.cpp
    ${CMAKE_SOURCE_DIR}/AtomGainTests.cpp
    ${CMAKE_SOURCE_DIR}/AtomDiodeTests.cpp
    ${CMAKE_SOURCE_DIR}/TestUtils.cpp
    ${CMAKE_SOURCE_DIR}/../src/Sample.cpp
    ${CMAKE_SOURCE_DIR}/../src/Resampler.cpp
    ${CMAKE_SOURCE_DIR}/../src/UpFirDown.cpp
    ${CMAKE_SOURCE_DIR}/../src/AtomBiquad.cpp
    ${CMAKE_SOURCE_DIR}/../src/AtomGain.cpp
    ${CMAKE_SOURCE_DIR}/../src/AtomDiode.cpp
)

# Only needed if __builtin_assume_aligned is used
#SET( MY_GCC_FLAGS "-fpermissive")
#SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MY_GCC_FLAGS}")

# link Boost libraries to the new target
target_link_libraries(testapp 
    PRIVATE
    GTest::gtest 
    GTest::gtest_main 
    GTest::gmock 
    GTest::gmock_main
    SndFile::sndfile
)
