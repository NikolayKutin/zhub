cmake_minimum_required(VERSION 3.16)


set(PROJECT zhub)
set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ -static")
#set(Boost_USE_STATIC_LIBS ON) 
set(Boost_USE_MULTITHREADED ON)

set(BOOST_ROOT "C:/development/boost_1_75_0")
set(YAML_CPP_ROOT "C:/development/yaml-cpp")

project(${PROJECT})
set(CMAKE_BINARY_DIR ${PROJECT_SOURCE_DIR}/bin)

include(ExternalProject)

ExternalProject_Add(mqtt_cpp
  GIT_REPOSITORY https://github.com/redboltz/mqtt_cpp
  SOURCE_DIR ${PROJECT_SOURCE_DIR}/mqtt_cpp
  GIT_TAG        master
  CONFIGURE_COMMAND ""
  BUILD_COMMAND     ""
  INSTALL_COMMAND   ""
  TEST_COMMAND      ""
)

if(WIN32)
  set (Win32_LIBRARIES wsock32 ws2_32)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_WIN32_WINNT=0x0601")
  if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")
    #set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
  endif(MSVC)
endif(WIN32)

FIND_PACKAGE(Boost)

set(YAML_CPP_LIBRARY yaml-cppd)

file(GLOB_RECURSE SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp)

include_directories(${Boost_INCLUDE_DIRS} ${PROJECT_SOURCE_DIR}/mqtt_cpp/include ${YAML_CPP_ROOT}/include)
link_directories(${BOOST_ROOT}/lib ${YAML_CPP_ROOT}/lib)

add_executable (${PROJECT} ${SOURCES})
add_dependencies(mqtt_cpp mqtt_cpp)
target_link_libraries(${PROJECT} ${Boost_LIBRARIES} ${Win32_LIBRARIES} ${YAML_CPP_LIBRARY})