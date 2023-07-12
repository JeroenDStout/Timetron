message(STATUS "Setup Python3")

include(ExternalProject)

find_package(Python3 COMPONENTS Interpreter)

#
# Packages
#

macro(python_cpm_package name)
  message(STATUS "Add Python package ${name}")
  CPMAddPackage(NAME ${ARGV})
  list(APPEND python_paths "${${name}_SOURCE_DIR}") 
endmacro()
