# FindGraphviz.cmake
# Locate Graphviz
# This module defines
#  GRAPHVIZ_FOUND, if false, do not try to use Graphviz
#  GRAPHVIZ_INCLUDE_DIRS, where to find Graphviz headers
#  GRAPHVIZ_LIBRARIES, the libraries to link against to use Graphviz
#  GRAPHVIZ_LIBRARY_DIRS, the directories containing the Graphviz libraries

if(GRAPHVIZ_ROOT)
    set(GRAPHVIZ_INCLUDE_DIR ${GRAPHVIZ_ROOT}/include)
    set(GRAPHVIZ_LIBRARY_DIR ${GRAPHVIZ_ROOT}/lib)
endif()

find_path(GRAPHVIZ_INCLUDE_DIR graphviz/cgraph.h
          HINTS ${GRAPHVIZ_ROOT}/include
          PATH_SUFFIXES graphviz)

find_library(GRAPHVIZ_CGRAPH_LIBRARY NAMES cgraph
             HINTS ${GRAPHVIZ_ROOT}/lib)

find_library(GRAPHVIZ_GVC_LIBRARY NAMES gvc
             HINTS ${GRAPHVIZ_ROOT}/lib)

if(GRAPHVIZ_INCLUDE_DIR AND GRAPHVIZ_CGRAPH_LIBRARY AND GRAPHVIZ_GVC_LIBRARY)
    set(GRAPHVIZ_FOUND TRUE)
    set(GRAPHVIZ_LIBRARIES ${GRAPHVIZ_CGRAPH_LIBRARY} ${GRAPHVIZ_GVC_LIBRARY})
    set(GRAPHVIZ_INCLUDE_DIRS ${GRAPHVIZ_INCLUDE_DIR})
    get_filename_component(GRAPHVIZ_LIBRARY_DIRS ${GRAPHVIZ_CGRAPH_LIBRARY} DIRECTORY)
else()
    set(GRAPHVIZ_FOUND FALSE)
endif()

mark_as_advanced(GRAPHVIZ_INCLUDE_DIR GRAPHVIZ_CGRAPH_LIBRARY GRAPHVIZ_GVC_LIBRARY)