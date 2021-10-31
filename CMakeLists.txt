cmake_minimum_required(VERSION 3.16)

project(RavEntities)

file(GLOB SRC src/*.cpp src/*.hpp)

add_library(${PROJECT_NAME} ${SRC})
target_compile_features("${PROJECT_NAME}" PUBLIC cxx_std_17)
set_target_properties("${PROJECT_NAME}" PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(${PROJECT_NAME} PUBLIC "src/")

file(GLOB TESTSRC test/*.cpp test/*.hpp)
add_executable(${PROJECT_NAME}Test ${TESTSRC})
target_link_libraries(${PROJECT_NAME}Test PRIVATE ${PROJECT_NAME})