message(STATUS "Setup CXX")
  
#
# Architecture
#

set(CMAKE_CXX_STANDARD 17)

if(MSVC)
  #use 64bit toolchain
  set(CMAKE_VS_PLATFORM_TOOLSET_HOST_ARCHITECTURE x64)
  
  #If the generator is not Ninja, enable multithreaded compilation (/MP)
  if (NOT ${CMAKE_GENERATOR} STREQUAL "Ninja") 
  	add_compile_options(/MP)
  endif()
endif()

#
# Warnings
#

if(MSVC)
  add_compile_options(
    /Wall /WX
    /wd4061 # Switch of Enum is not explicitly handled by a case label (even if default: is present).
    /wd4514 # Unreferenced inline function
    /wd4583 # Destructor is not implicitly called
    /wd4623 # Default constructor was implicitly defined as deleted
    /wd4625 # Copy constructor was implicitly defined as deleted
    /wd4626 # Assignment operator was implicitly defined as deleted
    /wd4668 # 'symbol' is not defined as a preprocessor macro, replacing with '0' for 'directives'
    /wd4710 # Function not inlined
    /wd4711 # Function automatically inlined
    /wd4820 # Struct padding
    /wd4868 # Compiler may not enforce left-to-right evaluation order in braced initializer list
    /wd5026 # Move constructor was implicitly defined as deleted
    /wd5045 # Spectre mitigation
    /wd5219 # Implicit conversion from 'type-1' to 'type-2', possible loss of data
    /wd5262 # Implicit fall-through
    /wd5264 # 'const' variable is not used
  )
  
  #Generate PDB in all build configurations
  add_compile_options("$<$<NOT:$<CONFIG:Debug>>:/Zi>")
  add_link_options("$<$<NOT:$<CONFIG:Debug>>:/DEBUG>")
else()
  #Flags for all non-MSVC compilers:
  add_compile_options(
    -Wall -Werror
  )
  
  #Disable RTTI 
  add_compile_options(-fno-rtti)
endif()


#
# Projects
#

function( configure_cxx_target project_ref )
		
  message(STATUS "Configure CXX Target ${project_ref}")
	
  set_property(TARGET ${project_ref} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$(Configuration)")
  
  if(MSVC)
    set_target_properties(${project_ref}          PROPERTIES COMPILE_PDB_NAME               "$(ProjectName)")
    set_target_properties(${PDB_OUTPUT_DIRECTORY} PROPERTIES CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
  endif()
  
endfunction()
