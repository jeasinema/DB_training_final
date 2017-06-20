cmake_minimum_required (VERSION 2.6)

# project
set(PROJECT_NAME DriveSharing)
project(${PROJECT_NAME})

# path
set (BASE_PATH ./)

# for c++11
add_definitions(-std=c++11)

# Options. Turn on with 'cmake -Dmyvarname=ON'.
option(test "Build all tests." OFF) # Makes boolean 'test' available.
set(CMAKE_CXX_FLAGS "-g -Wall -Os -lmetis")

# normal 
# include
include_directories(
    ${BASE_PATH}
    ${BASE_PATH}include
)

# add executable
set (${PROJECT_NAME}_SRC
)
# generate project as a lib, so we can reuse it when testing
# add_library(${PROJECT_NAME}_lib ${${PROJECT_NAME}_SRC})
add_executable (${PROJECT_NAME} ./src/main.cpp)
# target_link_libraries(${PROJECT_NAME} ${PROJECT_NAME}_lib)

# for metis
find_library(METIS_LIBRARY metis HINTS /usr/local/lib)
include_directories(/usr/local/include)
target_link_libraries(${PROJECT_NAME} ${METIS_LIBRARY})

