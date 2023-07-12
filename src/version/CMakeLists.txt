init_directory(version)

# Define the Version project
init_project(version "meta")

# Sources static
setup_project_source(version "version"
  src:
    git_version.cpp
  include:
    git_version.h GENERATED
)
setup_project_source(version "script"
  script:
    ../../touch
    generate_git_version.py
    git_version.txt GENERATED
)

# Target
configure_project_static_lib(version)
configure_cxx_target(version)

# Custom
add_custom_command(
  COMMENT			"Generate version file"
  OUTPUT    		${abs_gen_include}/git_version.h
  COMMAND   		${Python3_EXECUTABLE}
					${abs_can_script}/generate_git_version.py
					${abs_gen_include}/git_version.h
					${abs_gen_script}/git_version.txt
                    "${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}"
  DEPENDS			${project_root_dir}/touch
                    ${abs_can_script}/generate_git_version.py
  WORKING_DIRECTORY ${project_root_dir}
  VERBATIM
)