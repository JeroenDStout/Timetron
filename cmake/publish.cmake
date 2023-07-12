message(STATUS "Config publication")


set_property(GLOBAL PROPERTY publish_exec)
set_property(GLOBAL PROPERTY publish_file)
set_property(GLOBAL PROPERTY publish_dep)


function(configure_publish_exec name)
  message(STATUS "Publication for ${name}")
  
  get_property(_publish_exec GLOBAL PROPERTY publish_exec)
  list(APPEND _publish_exec "${name}" )
  set_property(GLOBAL PROPERTY publish_exec "${_publish_exec}")
endfunction()


function(configure_publish_file in_path out_path)
  message(STATUS "Publication for ${in_path}")
  
  get_property(_publish_file GLOBAL PROPERTY publish_file)
  list(APPEND _publish_file "${in_path} -> ${out_path}" )
  set_property(GLOBAL PROPERTY publish_file "${_publish_file}")
endfunction()


function(configure_publish_qt6)
  message(STATUS "Publication for Qt6")
  
  get_property(_publish_dep GLOBAL PROPERTY publish_dep)
  list(APPEND _publish_dep "qt6" )
  set_property(GLOBAL PROPERTY publish_dep "${_publish_dep}")
endfunction()


function(finalise_publish)
  message(STATUS "")
  message(STATUS "-- Finalising publication")
  
  set(file_out "${CMAKE_BINARY_DIR}/publish_info.txt")
  file(WRITE ${file_out} "")
  
  get_property(_publish_exec GLOBAL PROPERTY publish_exec)
  get_property(_publish_file GLOBAL PROPERTY publish_file)
  get_property(_publish_dep  GLOBAL PROPERTY publish_dep)
  
  message(STATUS "Executables:")
  foreach (var IN ITEMS ${_publish_exec})
    message(STATUS "* " ${var})
    file(APPEND ${file_out} "executable: ${var}\n")
  endforeach()
  
  message(STATUS "Dependencies:")
  foreach (var IN ITEMS ${_publish_dep})
    message(STATUS "* " ${var})
    file(APPEND ${file_out} "dependency: ${var}\n")
  endforeach()
  
  foreach (var IN ITEMS ${_publish_file})
    file(APPEND ${file_out} "file: ${var}\n")
  endforeach()
  
endfunction()