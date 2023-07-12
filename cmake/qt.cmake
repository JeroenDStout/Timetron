message(STATUS "Setup Qt6")

#
# Packages
#

find_package(Qt6 REQUIRED COMPONENTS Widgets Charts)

#
# Source
#

macro(setup_project_source_qt_ui project_ref project_source_group)
  set(input_list "${ARGN}")
  set(in_file_list)
  set(out_file_list)
  
  foreach(var IN LISTS input_list)
    file(RELATIVE_PATH rel ${CMAKE_CURRENT_SOURCE_DIR} ${abs_can_include}/${var})
    qt6_wrap_ui_ex(out_var ${rel})
    list(APPEND in_file_list  ${abs_can_include}/${var})
    list(APPEND out_file_list ${out_var})
  endforeach()
    
  set_project_source_list(${project_ref})
  setup_project_source_external_generated(${project_ref} ${project_source_group} ${out_file_list})
  source_group(${project_source_group} FILES ${in_file_list})
  list(APPEND ${project_source_list} ${in_file_list})
endmacro()

#
# Deploying
#

get_target_property(_qmake_executable Qt6::qmake IMPORTED_LOCATION)
get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)

function(configure_qt_target project_ref)
  message(STATUS "Configure Qt Target ${project_ref}")

  set_target_properties(${project_ref} PROPERTIES AUTOMOC ON)
  set_target_properties(${project_ref} PROPERTIES AUTORRC ON)
  set_target_properties(${project_ref} PROPERTIES AUTOUIC ON)
  target_link_libraries(${project_ref} PRIVATE Qt6::Widgets)
  target_link_libraries(${project_ref} PRIVATE Qt6::Charts)
  
  if(MSVC)
    message(STATUS " - Additional warnings are disabled for ${project_ref} to allow Qt compilation")
    target_compile_options(${project_ref} 
        PRIVATE
            "/wd4365;" # signed/unsigned mismatch
            "/wd4371;" # layout of class may have changed
            "/wd4464;" # relative include path contains '..'
            "/wd4702;" # unreachable code
            "/wd5027;" # move assignment operator was implicitly defined as deleted
            "/wd5204;" # class has virtual functions, but its trivial destructor is not virtual
    )

    message(STATUS " - Add deployment for windows")
    add_custom_command(TARGET ${project_ref} POST_BUILD
        COMMAND "${_qt_bin_dir}/windeployqt.exe"         
                --verbose 0
                \"$<TARGET_FILE:${project_ref}>\"
        COMMENT "Deploying Qt libraries using windeployqt for ${project_ref}"
    )
  endif()

endfunction()


# Override of QT6 wrap ui function to place output files in correcte folder
function(qt6_wrap_ui_ex outfiles )
    set(options)
    set(oneValueArgs)
    set(multiValueArgs OPTIONS)

    cmake_parse_arguments(_WRAP_UI "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set(ui_files ${_WRAP_UI_UNPARSED_ARGUMENTS})
    set(ui_options ${_WRAP_UI_OPTIONS})

    foreach(it ${ui_files})
        get_filename_component(outfile ${it} NAME_WE)
        get_filename_component(infile ${it} ABSOLUTE)
        set(outfile ${abs_gen_include}/ui_${outfile}.h)
        add_custom_command(OUTPUT ${outfile}
          DEPENDS ${QT_CMAKE_EXPORT_NAMESPACE}::uic
          COMMAND ${QT_CMAKE_EXPORT_NAMESPACE}::uic
          ARGS ${ui_options} -o ${outfile} ${infile}
          MAIN_DEPENDENCY ${infile} VERBATIM)
        set_source_files_properties(${infile} PROPERTIES SKIP_AUTOUIC ON)
        set_source_files_properties(${outfile} PROPERTIES SKIP_AUTOMOC ON)
        set_source_files_properties(${outfile} PROPERTIES SKIP_AUTOUIC ON)
        list(APPEND ${outfiles} ${outfile})
    endforeach()
    set(${outfiles} ${${outfiles}} PARENT_SCOPE)
endfunction()