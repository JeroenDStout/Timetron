message(STATUS "Setup TinyXML2")

include(FetchContent)
cmake_policy(SET CMP0135 NEW)

FetchContent_Declare(
  tinyxml2
  URL     https://github.com/leethomason/tinyxml2/archive/refs/tags/9.0.0.zip
)
FetchContent_MakeAvailable(tinyxml2)

set_target_properties(tinyxml2 PROPERTIES FOLDER "dependencies")
if (tinyxml2_BUILD_TESTING)
  set_target_properties(xmltest  PROPERTIES FOLDER "tests")
endif()

if(MSVC)
  message(STATUS " - Additional warnings are disabled for tinyxml2 to allow compilation")
  target_compile_options(tinyxml2
    PRIVATE
      "/wd4365;" # signed/unsigned mismatch
      "/wd4774;" # format string expected in argument 2 is not a string literal
  )
  if (tinyxml2_BUILD_TESTING)
    target_compile_options(xmltest
      PRIVATE
        "/wd4365;" # signed/unsigned mismatch
        "/wd4774;" # format string expected in argument 2 is not a string literal
        "/wd4800;" # Implicit conversion
        "/wd5027;" # Move assignment operator was implicitly defined as deleted
        "/wd5039;" # Pointer or reference to potentially throwing function passed to 'extern "C"' function
    )
  endif()
endif()

function(configure_project_tinyxml2 project_ref)
  if(MSVC)
    message(STATUS " - Additional warnings are disabled for ${project_ref} to allow compilation")
    target_compile_options(${project_ref} 
      PRIVATE
        "/wd4365;" # signed/unsigned mismatch
        "/wd4774;" # format string expected in argument 2 is not a string literal
    )
  endif()
  include_directories(${tinyxml2_SOURCE_DIR})
  target_link_libraries(${project_ref} PRIVATE tinyxml2)
endfunction()