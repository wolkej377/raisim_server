cmake_minimum_required(VERSION 3.5)
project(raisim_examples LANGUAGES CXX)

#==
# Dependencies
#==

find_package(raisim CONFIG REQUIRED)
find_package(Eigen3 REQUIRED HINTS ${Eigen3_HINT})

file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/../rsc" DESTINATION ${CMAKE_BINARY_DIR}/src)

include_directories ( ${EIGEN3_INCLUDE_DIRS} )

#==
# Build Targets
#==

function(create_executable app_name file_name)
    add_executable(${app_name} ${file_name})
    target_link_libraries(${app_name} PUBLIC raisim::raisim pthread)
    target_compile_options(${app_name} PRIVATE "$<$<CONFIG:RELEASE>:-O3>")
    target_compile_options(${app_name} PRIVATE "$<$<CONFIG:DEBUG>:-O0>")
    target_include_directories(${app_name} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
endfunction()

# maps
create_executable(mytest maps/mytest.cpp)