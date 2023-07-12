message(STATUS "Config types:")

foreach(var ${CMAKE_CONFIGURATION_TYPES})
  message(STATUS "-  ${var}")
endforeach()