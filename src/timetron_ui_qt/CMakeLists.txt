init_directory(timetron_ui_qt_qt)

# Define the Timetron project
init_project(timetron_ui_qt "timetron/qt")

# Sources
setup_project_source(timetron_ui_qt "core"
  src:
    main.cpp
)
setup_project_source(timetron_ui_qt "window"
  src:
    window.cpp
  include:
    window.h
)
setup_project_source_qt_ui(timetron_ui_qt "ui"
  window.ui
  version.ui
)
#setup_project_source(extron_ui_qt "assets"
#  src:
#    ${rel_can_assets}/app_resource/resource.rc
#)

# Target
configure_project_executable(timetron_ui_qt)
configure_project_console(timetron_ui_qt)
configure_cxx_target(timetron_ui_qt)
configure_project_tinyxml2(timetron_ui_qt)
configure_qt_target(timetron_ui_qt)
	
add_dependencies(timetron_ui_qt version)
