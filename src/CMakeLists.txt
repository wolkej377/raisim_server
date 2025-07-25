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

# server examples
create_executable(primitives server/primitives.cpp)
create_executable(meshes server/meshes.cpp)
create_executable(laikago server/laikago.cpp)
create_executable(balls server/balls.cpp)
create_executable(heightmap server/heightmap.cpp)
create_executable(aliengo server/aliengo.cpp)
create_executable(materials server/material.cpp)
create_executable(materialStaticFriction server/materialStaticFriction.cpp)
create_executable(heightMapUsingPng server/heightMapUsingPng.cpp)
create_executable(dynamicHeightMap server/dynamicHeightMapUnrealOnly.cpp)
create_executable(wheeledRobot server/wheeledRobot.cpp)
create_executable(newtonsCradle server/newtonsCradle.cpp)
create_executable(visualObjects server/visualObjects.cpp)
create_executable(rayDemo server/rayDemo.cpp)
create_executable(rayDemo2 server/rayDemo2.cpp)
create_executable(anymal_stress_test server/anymals_stress_test.cpp)
create_executable(compound server/compound.cpp)
create_executable(cartPole server/cartPole.cpp)
create_executable(springTest server/springTest.cpp)
create_executable(trackedRobotAndTemplatedURDF server/trackedRobotAndTemplatedURDF.cpp)
create_executable(kinematicObject server/kinematicObject.cpp)
create_executable(sensors server/sensors.cpp)
create_executable(synchronousServerUpdate server/synchronousServerUpdate.cpp)
create_executable(deletion server/deletion.cpp)
create_executable(inverseDynamics server/inverseDynamics.cpp)
create_executable(dzhanibekovEffect server/dzhanibekovEffect.cpp)
create_executable(go1 server/go1.cpp)
create_executable(minitaur server/minitaur.cpp)

# maps
create_executable(mountain1 maps/mountain1.cpp)
create_executable(office1 maps/office1.cpp)
create_executable(lake1 maps/lake1.cpp)
create_executable(atlas maps/atlas.cpp)
create_executable(anymals maps/anymals.cpp)
create_executable(hill1 maps/hill1.cpp)
create_executable(kinova maps/kinova.cpp)
