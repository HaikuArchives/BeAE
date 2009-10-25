#
# - Haiku module for CMake
#


#
# Compile a resource definition file(.rdef) to a resource file(.rsrc).
# Add resources from the .rsrc file to the target.
#
function(HAIKU_ADD_RESOURCE_DEF TARGET FILENAME)

	get_filename_component(rdefpath ${FILENAME} ABSOLUTE)
	get_filename_component(rdeffile ${FILENAME} NAME_WE)

	set(rsrcfile "${rdeffile}.rsrc")
	set(rsrcpath "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${rsrcfile}.dir/${rsrcfile}")

	add_custom_command(
		OUTPUT ${rsrcpath}
		COMMAND "rc" "-o" "${rsrcpath}" "${rdefpath}"
		DEPENDS ${rdefpath}
		COMMENT "Compiling resource ${rsrcpath}")

	add_custom_target(${rsrcfile} DEPENDS ${rsrcpath})

	set_source_files_properties(${rsrcfile} PROPERTIES GENERATED TRUE)

	haiku_add_resource_internal(${TARGET} ${rsrcpath})

endfunction(HAIKU_ADD_RESOURCE_DEF)



#
# Add resources from a .rsrc file to the target.
#
function(HAIKU_ADD_RESOURCE TARGET FILENAME)

	haiku_add_resource_internal(${TARGET} "${CMAKE_CURRENT_SOURCE_DIR}/${FILENAME}")

endfunction(HAIKU_ADD_RESOURCE)



#
# Add resources from a .rsrc file to the target.
# This is the internal version meant to be called from this module only.
#
function(HAIKU_ADD_RESOURCE_INTERNAL TARGET RSRC_PATH)

	get_target_property(TARGET_PATH ${TARGET} LOCATION)

	if(NOT TARGET_PATH)
		message(SEND_ERROR "Unable to determine target location for HAIKU_ADD_RESOURCE")
	endif(NOT TARGET_PATH)

	add_custom_command(
		TARGET ${TARGET}
		POST_BUILD
		COMMAND "xres" "-o" "${TARGET_PATH}" "${RSRC_PATH}"
		COMMENT "Merging resource ${RSRC_PATH}")

	# FIXME: Need to fix the dependency so TARGET is rebuilt properly
	# and doesn't need this hack
	add_custom_target(
		${TARGET}_check_resources
		COMMAND "/bin/sh" "-c" \"if [ \"${RSRC_PATH}\" -nt \"${TARGET_PATH}\" ]\\;then rm -f \"${TARGET_PATH}\"\\;fi\")

	get_filename_component(rsrcfile ${RSRC_PATH} NAME)

	# Need this so that rsrcfile target is built before _check_resources
	add_dependencies(${TARGET}_check_resources ${rsrcfile})

	add_dependencies(${TARGET} ${TARGET}_check_resources)

endfunction(HAIKU_ADD_RESOURCE_INTERNAL)


#
# Run "mimeset" command to automatically set mimetype attributes using sniffer rules.
#
function(HAIKU_AUTO_MIMETYPE TARGET)

	get_target_property(TARGET_LOC ${TARGET} LOCATION)

	if(NOT TARGET_LOC)
		message(SEND_ERROR "Unable to determine target location for HAIKU_AUTO_MIMETYPE")
	endif(NOT TARGET_LOC)

	add_custom_command(
		TARGET ${TARGET}
		POST_BUILD
		COMMAND "mimeset" "-f" "${TARGET_LOC}"
		COMMENT "Setting mimetype ${TARGET_LOC}")

endfunction(HAIKU_AUTO_MIMETYPE)


