init_directory(timetron_core)

# Define the Timetron project
init_project(timetron_core "timetron/core")

# Sources
setup_project_source(timetron_core "data"
  src:
    data_diagnostic.cpp
    data_timeline.cpp
  include:
    data_day.h
    data_diagnostic.h
    data_timeline.h
)
setup_project_source(timetron_core "proc"
  src:
    proc_diagnose.cpp
  include:
    proc_diagnose.h
)

# Target
configure_project_static_lib(timetron_core)
configure_project_tinyxml2(timetron_core)
configure_cxx_target(timetron_core)
