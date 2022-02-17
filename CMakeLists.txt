project(homura)
cmake_minimum_required(VERSION 3.10.0 FATAL_ERROR)

set(PACKAGE_VERSION "1.0.0.0")
set(CMAKE_CXX_STANDARD 17)

set(THIRD_PARTY_ROOT /usr/local/third_party)

set(Boost_USE_STATIC_LIBS ON) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME ON) 
find_package(Boost 1.56.0)

if(Boost_FOUND)
    include_directories( ${Boost_INCLUDE_DIRS} )
    link_directories(${Boost_LIBRARIES})
endif()

find_package(MPI 1.8.0)
if(MPI_FOUND)
    include_directories(${MPI_CXX_INCLUDE_PATH})
endif()

set (GTEST_ROOT ${THIRD_PARTY_ROOT}/gtest)
find_package(GTest REQUIRED)
if(gtest_found)
    link_directories(${GTEST_LIBRARIES})
endif()

include_directories(${THIRD_PARTY_ROOT}/zeromq/include)
include_directories(${THIRD_PARTY_ROOT}/yaml/include)
include_directories(${CMAKE_SOURCE_DIR}/mio/include)
# include_directories(${CMAKE_SOURCE_DIR}/mio/include/mct)
# include_directories(/usr/include)

link_directories(${THIRD_PARTY_ROOT}/zeromq/lib)
link_directories(${THIRD_PARTY_ROOT}/yaml/lib)
# link_directories(/usr/lib)

# add_subdirectory(mio)
add_subdirectory(test)
