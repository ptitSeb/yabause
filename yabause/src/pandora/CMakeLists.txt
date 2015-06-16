project(yabause-pandora)

include_directories(${PORT_INCLUDE_DIRS})
add_definitions(${PORT_CFLAGS})

set(yabause_pandora_SOURCES
    main.c)

set(yabause_main_HEADERS
    main.h)

#link_directories(..)
add_executable(yabause-pandora ${yabause_pandora_SOURCES})
if (YAB_WANT_OPENGL)
	set_target_properties(yabause-pandora PROPERTIES OUTPUT_NAME yabause-gl)
else()
	set_target_properties(yabause-pandora PROPERTIES OUTPUT_NAME yabause)
endif()
target_link_libraries(yabause-pandora ${YABAUSE_LIBRARIES})
target_link_libraries(yabause-pandora ${PORT_LIBRARIES})
target_link_libraries(yabause-pandora yabause)
target_link_libraries(yabause-pandora m)
