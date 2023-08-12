file(GLOB_RECURSE FRAMEWORK_SHADER_VERTEX RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.vert)
file(GLOB_RECURSE FRAMEWORK_SHADER_FRAGME RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.frag)

add_executable(pixel_canvas pixel_canvas.cpp
              ${FRAMEWORK_SHADER_VERTEX}
              ${FRAMEWORK_SHADER_FRAGME})

target_link_libraries(pixel_canvas ${FRAMEWORK_NAME} glfw ${GLFW_LIBRARIES})
add_dependencies(pixel_canvas glfw ${FRAMEWORK_NAME})


if(MSVC)
	# set working directory and start up project for VS project
	set_target_properties(pixel_canvas PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${WORKING_DIR}")
  	set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT pixel_canvas)
endif(MSVC)