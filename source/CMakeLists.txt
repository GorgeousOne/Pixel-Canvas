file(GLOB_RECURSE FRAMEWORK_SHADER_VERTEX RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.vert)
file(GLOB_RECURSE FRAMEWORK_SHADER_FRAGME RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.frag)

add_executable(ray_casting ray_casting.cpp
              ${FRAMEWORK_SHADER_VERTEX}
              ${FRAMEWORK_SHADER_FRAGME})

target_link_libraries(ray_casting ${FRAMEWORK_NAME} glfw ${GLFW_LIBRARIES})
add_dependencies(ray_casting glfw ${FRAMEWORK_NAME})


if(MSVC)
	# set working directory and start up project for VS project
	set_target_properties(ray_casting PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${WORKING_DIR}")
  	set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ray_casting)
endif(MSVC)