## Project Setting
cmake_minimum_required(VERSION 3.15...3.29)
project(sabre LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


if(NOT CMAKE_BUILD_TYPE)
    message(STATUS "Build type not set - defaulting to Release")
    set(CMAKE_BUILD_TYPE Release)
endif()


# Find Boost
find_package(Boost REQUIRED)
if(Boost_FOUND)
  if(NOT TARGET Boost::headers)
    add_library(Boost::headers IMPORTED INTERFACE)
    if(TARGET Boost::boost)
      # Classic FindBoost
      set_property(TARGET Boost::boost PROPERTY INTERFACE_LINK_LIBRARIES Boost::boost)
    else()
      # Very old FindBoost, or newer Boost than CMake in older CMakes
      set_property(TARGET Boost::headers PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIRS})
    endif()
  endif()
endif()


# File filter
file(GLOB SOURCES "src/*.cpp")
file(GLOB DAG_SOURCES "src/DAG/*.cpp")
file(GLOB Vis_SOURCES "src/Vis/*.cpp")
file(GLOB Model_SOURCES "src/Model/*.cpp")


if(SKBUILD)
  execute_process(
    COMMAND "${PYTHON_EXECUTABLE}" -c
            "import pybind11; print(pybind11.get_cmake_dir())"
    OUTPUT_VARIABLE _tmp_dir
    OUTPUT_STRIP_TRAILING_WHITESPACE COMMAND_ECHO STDOUT
  )
  list(APPEND CMAKE_PREFIX_PATH "${_tmp_dir}")

  set(PYBIND11_FINDPYTHON ON)
  find_package(pybind11 REQUIRED CONFIG)
else()
  # Build Debug
  set(PYBIND11_FINDPYTHON ON)
  add_subdirectory(extern/pybind11)

  add_executable(
      sabre_test 
          ${SOURCES} 
          ${DAG_SOURCES}
          ${Model_SOURCES}
  )
  target_include_directories(
    sabre_test 
    PRIVATE 
        "src/include" 
        "src/include/DAG" 
        "src/include/Vis"
        "src/include/Model"
        "src/include/vendor"
  )    
  target_link_libraries(sabre_test PRIVATE Boost::headers)
  target_compile_definitions(sabre_test PRIVATE BOOST_NO_CXX98_FUNCTION_BASE)
endif()

# message(STATUS "Python Interpreter: ${Python_EXECUTABLE}")
# message(STATUS "Python Version: ${Python_VERSION}")


set(target sabre)
pybind11_add_module(
    ${target} THIN_LTO
        src/Binding/binding.cpp 
        ${SOURCES} 
        ${DAG_SOURCES}
        ${Model_SOURCES}
)
target_include_directories(
    ${target} 
    PRIVATE 
        "src/include" 
        "src/include/DAG" 
        "src/include/Vis"
        "src/include/Model"
        "src/include/vendor"
)    

target_link_libraries(sabre PRIVATE pybind11::headers)

if(Boost_FOUND)
    target_link_libraries(${target} PRIVATE Boost::headers)
    target_compile_definitions(${target} PRIVATE BOOST_NO_CXX98_FUNCTION_BASE)
endif()


# Install
install(TARGETS sabre LIBRARY DESTINATION .)